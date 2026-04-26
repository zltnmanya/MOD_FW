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
 
#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <msg_queue.h>

#if defined(__GNUC__)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define be16toh(x) __builtin_bswap16(x)
#define be32toh(x) __builtin_bswap32(x)
#define be64toh(x) __builtin_bswap64(x)
#define le16toh(x) (x)
#define le32toh(x) (x)
#define le64toh(x) (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define be16toh(x) (x)
#define be32toh(x) (x)
#define be64toh(x) (x)
#define le16toh(x) __builtin_bswap16(x)
#define le32toh(x) __builtin_bswap32(x)
#define le64toh(x) __builtin_bswap64(x)
#else
# error "ERR: can't determine endianness"
#endif
#else
# error "ERR: unsupported compiler"
#endif

enum btn_press_state {
  BTN_STATE_IDLE = 0,
  BTN_STATE_RESTING,
  BTN_STATE_CALIBRATING
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct msg_queue queue_main;

extern enum btn_press_state sw_state;

void handle_com_transfer(int keep_sending);
int dev_mode_is_tracking();
int dev_mode_is_streaming();
void dev_mode_set_streaming();
void error_inf_loop(uint32_t err_code) __attribute__((__noreturn__));

#define REP_TYPE_IMU         (1u << 0)
#define REP_TYPE_MAG         (1u << 1)
#define REP_TYPE_ORI_FUSED   (1u << 2)
#define REP_TYPE_ORI_DEDREC  (1u << 3)
extern uint32_t reports_selected;

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H_ */
