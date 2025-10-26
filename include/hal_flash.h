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
 
#ifndef PERSISTENCE_FLASH_H_
#define PERSISTENCE_FLASH_H_

struct packed_v3_s16 { // 6 bytes
	int16_t x;
	int16_t y;
	int16_t z;
} __attribute__((packed));
struct packed_v3_s32 { // 12 bytes
	int32_t x;
	int32_t y;
	int32_t z;
} __attribute__((packed));
struct packed_v3_double { // 24 bytes
	double x;
	double y;
	double z;
} __attribute__((packed));


struct flash_storage_struct_v1 { //
	struct packed_v3_s16 gyro_offs_hw;	// 6 bytes
	struct packed_v3_double gyro_offs_fine; // 24 bytes
	struct packed_v3_double mag_offs; // 24 bytes
	struct packed_v3_double mag_gain; // 24 bytes
	double accel_gain; // 8 bytes
	struct packed_v3_double accel_offs; // 24 bytes
	// ... padding
	// uint32_t type_length
} __attribute__((packed));

typedef void(traverse_cb_fn)(void* /* data */, uint16_t /* length */, uint16_t /* type */);

#ifdef __cplusplus
extern "C" {
#endif
void store_data_tlv(void *data, uint16_t length, uint16_t type);
void traverse_flash(traverse_cb_fn *cb);
void clear_flash_data();
#ifdef __cplusplus
}
#endif


#endif /* PERSISTENCE_FLASH_H_ */
