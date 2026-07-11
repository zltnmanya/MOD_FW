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
 
#include <logger.h>
#include <diag.h>
#include <libopencm3/cm3/dwt.h>
#include <stdint.h>
#include <common.h>
#include <hal.h>
#include <persistence.h>

uint32_t dbg_cnt[DBG_CNT_COUNT];

static uint32_t dbg_flags = 0x0;

uint32_t tstamp_last[BM_TSTAMP_COUNT];
uint64_t tstamp_sum[BM_TSTAMP_COUNT];
uint32_t tstamp_count[BM_TSTAMP_COUNT];

void timestamp(uint8_t index) {
	if (index >= BM_TSTAMP_COUNT) return;
	uint32_t cyc = DWT_CYCCNT;
	uint32_t diff = cyc - tstamp_last[index];
	tstamp_last[index] = cyc;
	tstamp_sum[index] += (uint64_t)diff;
	tstamp_count[index]++;
}

void timestamp_start(uint8_t index) {
	if (index >= BM_TSTAMP_COUNT) return;
	tstamp_last[index] = DWT_CYCCNT;
}
void timestamp_stamp(uint8_t index, uint8_t next_start) {
	if (index >= BM_TSTAMP_COUNT) return;
	uint32_t cyc = DWT_CYCCNT;
	uint32_t diff = cyc - tstamp_last[index];
	tstamp_sum[index] += (uint64_t)diff;
	tstamp_count[index]++;

	if (next_start >= BM_TSTAMP_COUNT) return;
	tstamp_last[next_start] = cyc;
}
void timestamp_calc_avg() {
	uint32_t avgs[BM_TSTAMP_COUNT];
	uint64_t sum = 0;
	float percentage[BM_TSTAMP_COUNT];
	for (int i=0;i<BM_TSTAMP_COUNT;i++) {
		avgs[i] = tstamp_count[i] ? (tstamp_sum[i] / tstamp_count[i]) : 0;
		sum += tstamp_sum[i];
	}
	for (int i=0;i<BM_TSTAMP_COUNT;i++) {
		percentage[i] = tstamp_sum[i] / (float)sum * 100;
	}
	for (int i=0;i<BM_TSTAMP_COUNT;i++) {
    if (tstamp_count[i] == 0) continue;
    log_printf("TS:%d 0x%08lx%08lx",i,(uint32_t)(tstamp_sum[i] >> 32),(uint32_t)(tstamp_sum[i]));
    log_printf("/%lu~%lu (%.2f%%)\r\n",tstamp_count[i],  avgs[i], percentage[i]);
  }
}

int dbg_flag_toggle(uint8_t flag_no) {
  if (flag_no >= 32) return 0;
  uint32_t mask = (1u<<flag_no);
  dbg_flags ^= mask;
  return dbg_flags & mask ? 1 : 0;
}
int dbg_flag_get(uint8_t flag_no) {
  if (flag_no >= 32) return false;
  return (dbg_flags & (1u<<flag_no)) ? 1 : 0;
}
uint32_t dbg_flags_get() {
  return dbg_flags;
}
int dbg_flag_get_n_clear(uint8_t flag_no) {
  if (flag_no >= 32) return false;
  uint32_t mask = 1u<<flag_no;
  int rv = (dbg_flags & mask) ? 1 : 0;
  dbg_flags &= ~mask;
  return rv;
}


void dump_cpu_state() {
    uint32_t regs[15];
    __asm__(
       "str %%r0, %0\n\t"
       "str %%r1, %1\n\t"
       "str %%r2, %2\n\t"
       "str %%r3, %3\n\t"
       "str %%r4, %4\n\t"
       "str %%r5, %5\n\t"
       "str %%r6, %6\n\t"
       "str %%r7, %7\n\t"
       "str %%r8, %8\n\t"
       "str %%r9, %9\n\t"
       "str %%r10, %10\n\t"
       "str %%r11, %11\n\t"
       "str %%r12, %12\n\t"
       "str %%r13, %13\n\t"
       "str %%r14, %14\n\t"
        : 
        "=m" (regs[0]), "=m" (regs[1]), "=m" (regs[2]), "=m" (regs[3]),
        "=m" (regs[4]), "=m" (regs[5]), "=m" (regs[6]), "=m" (regs[7]),
        "=m" (regs[8]), "=m" (regs[9]), "=m" (regs[10]), "=m" (regs[11]),
        "=m" (regs[12]), "=m" (regs[13]), "=m" (regs[14])
        );
    log_printf("\r\nCPU:\r\n");
    log_printf("%08lx %08lx %08lx ", regs[0], regs[1], regs[2]);
    log_printf("%08lx\r\n%08lx %08lx ", regs[3], regs[4], regs[5]);
    log_printf("%08lx %08lx\r\n%08lx ", regs[6], regs[7], regs[8]);
    log_printf("%08lx %08lx %08lx\r\n", regs[9], regs[10], regs[11]);
    log_printf("%08lx %08lx %08lx\r\n", regs[12], regs[13], regs[14]);

    log_printf("stack:\r\n");
    uint32_t *stack = (uint32_t*)regs[13];
    for (int i=0;i<50;i++) {
      if ((uint32_t)(stack+2) > 0x2000fffc) break;
      log_printf("%d:%08lx %08lx %08lx\r\n",i, stack[0], stack[1], stack[2]);
      stack+=3;
    }
}

void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep) {
	(void)ep;

	char buf[64];
	int len = usbd_ep_read_packet(usbd_dev, 0x02, buf, 64);
  extern void post_irq_flag(uint32_t flag);
  for (int i=0;i<len;i++) {
    if (buf[i]=='C') {
      if (!cpu_hang_indication)
        post_irq_flag(8);
      else
        dump_cpu_state();
    } else if (buf[i] == 'r') {
      post_irq_flag(16);
    } else if (buf[i] == 'd') {
      for (int i=0;i<DBG_CNT_COUNT;i++)
        log_printf("dbg_cnt[%d]:%lu\r\n", i, dbg_cnt[i]);
      log_printf("tick:%lu rep:%08lx\r\n", get_common_tick(), reports_selected);
      log_printf("mode:%c\r\n", dev_mode_is_streaming() ? 'S' : 'T');
      log_printf("dbg_flags:%08lx\r\n", dbg_flags_get());
    } else if (buf[i] == 't') {
      timestamp_calc_avg();
    } else if (buf[i] >= '0' && buf[i] <= '9') {
      int flag = buf[i]-'0';
      int new_val = dbg_flag_toggle(flag);
      log_printf("dbg[%d]=%d\r\n", flag, new_val);
    } else if (buf[i] == 'S') {
      store_restart_streaming(1);
      log_printf("req.streaming\r\n");
    } else if (buf[i] == 'T') {
      store_restart_streaming(0);
      log_printf("req.tracking\r\n");
    }
  }
}
