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
 
#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/systick.h>

#include <common.h>
#include <hal.h>
#include <usb_hid_report_in.h>

static uint32_t tick_strobe_start = 0;
int strobe_clear = 0;
int strobe_enable = 0;
int cpu_hang_indication = 0;

uint32_t get_common_tick() {
  return tick_count;
}

void delay_ms(uint32_t d) {
  uint32_t started = tick_count;
  while (tick_count - started < d) ;
}

void sys_tick_handler(void) {
  tick_count++;

  if (tick_count - tick_strobe_start > 1000) {
    if (!strobe_clear) {
      strobe_clear = 1;
      tick_strobe_start = tick_count;
    } else if (tick_count - tick_strobe_start > 2000) {
      cpu_hang_indication = 1;
    }
  } else if (tick_count - tick_strobe_start > 900 && !gpio_status_led_get() && !strobe_clear) {
    gpio_status_led_set(0);
  }
  if (cpu_hang_indication) {
    handle_com_transfer(0);
  }
}

