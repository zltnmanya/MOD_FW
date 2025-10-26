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
 
#ifndef _MSG_QUEUE_
#define _MSG_QUEUE_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSGQ_SIZE 10
#define MSGQ_TYPE_FREE 0
typedef uint8_t msg_type_t;

#define MSGQ_TYPE_I2C_MOD8   1
struct msg_i2c_mod8 {
  msg_type_t msg_type;
  uint8_t dev;
  uint8_t addr;
  uint8_t data;
  uint8_t mask;
};

#define MSGQ_TYPE_I2C_MOD16  2
struct msg_i2c_mod16 {
  msg_type_t msg_type;
  uint8_t dev;
  uint8_t addr;
  uint16_t data;
  uint16_t mask;
};

union msg_u {
  msg_type_t    msg_type;
  struct msg_i2c_mod8  i2c_mod8;
  struct msg_i2c_mod16 i2c_mod16;
};
struct msg_queue {
  uint8_t next_write_pos;
  uint8_t current_read_pos;
  union msg_u msgs[MSGQ_SIZE];
};

/* initialization */
void msgq_init(struct msg_queue *q);

/* sending side (multiple senders allowed) */
union msg_u* msgq_request_buffer(struct msg_queue* q);
static inline void msgq_post(union msg_u* msg, msg_type_t msg_type) {
  msg->msg_type = msg_type;
}

/* receiving side (only one receiver allowed) */
union msg_u* msgq_receive(struct msg_queue* q);
void msgq_free(union msg_u* m);

#ifdef __cplusplus
}
#endif

#endif
