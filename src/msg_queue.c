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
 
#include <msg_queue.h>
#include <common.h>

void msgq_init(struct msg_queue *q) {
  q->next_write_pos = 0;
  q->current_read_pos = 0;
  for (int i=0;i<MSGQ_SIZE; i++) {
    q->msgs->msg_type = MSGQ_TYPE_FREE;
  }
}

union msg_u* msgq_request_buffer(struct msg_queue* q) {
  uint8_t nwp_old, nwp_new;
  do {
	nwp_old = q->next_write_pos;
	nwp_new = nwp_old + 1;
	if (nwp_new >= MSGQ_SIZE) nwp_new = 0;
  } while (__sync_val_compare_and_swap(&q->next_write_pos, nwp_old, nwp_new) != nwp_old);
  if (q->msgs[nwp_old].msg_type != MSGQ_TYPE_FREE) error_inf_loop(0);
  return q->msgs + nwp_old;
}

union msg_u* msgq_receive(struct msg_queue* q) {
  if (q->msgs[q->current_read_pos].msg_type == MSGQ_TYPE_FREE) return 0;
  union msg_u *rv = q->msgs + q->current_read_pos;
  q->current_read_pos++;
  if (q->current_read_pos >= MSGQ_SIZE) q->current_read_pos = 0;
  return rv;
}

void msgq_free(union msg_u* m) {
  if (m->msg_type == MSGQ_TYPE_FREE) error_inf_loop(0);
  m->msg_type = MSGQ_TYPE_FREE;
}
