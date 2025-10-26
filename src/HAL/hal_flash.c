/*
 * This file is part of the MOD project.
 *
 * Copyright (C) 2025 Zoltán Mánya <zltnmanya@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 

#include <common.h>
#include <stdint.h>
#include <hal_flash.h>
#include <libopencm3/stm32/flash.h>

#define FLASH_ADDR_END 0x08040000
#define FLASH_ADDR_START (FLASH_ADDR_END - 128*1024)

// #define FLASH_DRYRUN

static uint32_t flash_get_error() {
	return FLASH_SR & (FLASH_SR_PGPERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_OPERR);
}

static uint32_t find_lowest_used_addr() {
	uint32_t addr = FLASH_ADDR_END;
	do {
		uint32_t v = *(uint32_t*)(addr - 4);
		if (v == 0xffffffff) {
			return addr;
		}
		v &= 0xffff;

		addr -= (v + 4);
		addr &= 0xfffffffc;
	} while (addr > FLASH_ADDR_START);
	return 0xffffffff;
}

static int chk_flash_error() {
  if (FLASH_SR & (FLASH_SR_PGPERR | FLASH_SR_PGAERR | FLASH_SR_WRPERR | FLASH_SR_OPERR))
    return -1;
  return 0;
}

static void store_data(void *data, int len, uint32_t dst_addr) {
	uintptr_t src_addr = (uintptr_t)data;
	while (len > 0) {
		/* double word programming does not seem to be supported on STM32F401CCU6 */
		if (len >= 4) {
#ifndef FLASH_DRYRUN
			flash_program_word(dst_addr, *(uint32_t*)src_addr);
      if (chk_flash_error() != 0) error_inf_loop(flash_get_error());
#endif
			dst_addr += 4;
			src_addr += 4;
			len -= 4;
		} else if (len >= 2) {
#ifndef FLASH_DRYRUN
			flash_program_half_word(dst_addr, *(uint16_t*)src_addr);
      if (chk_flash_error() != 0) error_inf_loop(flash_get_error());
#endif
			dst_addr += 2;
			src_addr += 2;
			len -= 2;
		} else { /* 1 */
#ifndef FLASH_DRYRUN
			flash_program_byte(dst_addr, *(uint8_t*)src_addr);
      if (chk_flash_error() != 0) error_inf_loop(flash_get_error());
#endif
			dst_addr++;
			src_addr++;
			len--;
			break;
		}
	}
}

void store_data_tlv(void *data, uint16_t length, uint16_t type) {
	uint32_t addr = find_lowest_used_addr();
	if (addr < FLASH_ADDR_START || addr > FLASH_ADDR_END) return; // TODO: clear flash
	addr -= 4; // addr of type+len

	flash_unlock();
	{
#ifndef FLASH_DRYRUN
		uint32_t type_len = (((uint32_t)type) << 16) | length;
		flash_program_word(addr, type_len);
    if (chk_flash_error() != 0) error_inf_loop(flash_get_error());
#endif
	}
	addr = (addr - length) & 0xfffffffc; // base address

	store_data(data, length, addr);

	flash_lock();
}

void clear_flash_data() { // tested: works
	flash_unlock();

  // TODO: set voltage range
  flash_erase_sector(5, 2); // sector 5, 32bit words

  if (chk_flash_error() != 0) error_inf_loop(1);

	flash_lock();
}

void traverse_flash(traverse_cb_fn *cb) {
	uint32_t addr = FLASH_ADDR_END;
	do {
		uint32_t v = *(uint32_t*)(addr - 4);
		if (v == 0xffffffff) {
			return;
		}

		uint16_t length = v & 0xffff;
		uint16_t type = (v >> 16);

		addr -= (length + 4);
		addr &= 0xfffffffc;

		cb((void*)addr, length, type);
	} while (addr > FLASH_ADDR_START);
}
