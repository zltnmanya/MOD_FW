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
 
#ifndef _DIAG_H_
#define _DIAG_H_

#include <stdint.h>
#include <libopencm3/usb/usbd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BM_TSTAMP_COUNT 32
#define DBG_CNT_COUNT 10

extern uint32_t dbg_cnt[DBG_CNT_COUNT];
extern uint32_t tstamp_last[BM_TSTAMP_COUNT];
extern uint64_t tstamp_sum[BM_TSTAMP_COUNT];

void timestamp(uint8_t index);
void timestamp_start(uint8_t index);
void timestamp_stamp(uint8_t index, uint8_t next_start);
void timestamp_calc_avg();

int dbg_flag_toggle(uint8_t flag_no);
int dbg_flag_get(uint8_t flag_no);
int dbg_flag_get_n_clear(uint8_t flag_no);
uint32_t dbg_flags_get();

void dump_cpu_state();

void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep);

#ifdef __cplusplus
}
#endif

#endif
