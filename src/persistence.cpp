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
 
#include <util_math.hpp>
#include <persistence.h>
#include <hal_flash.h>
#include <logger.h>
#include <params.h>
#include <common.h>

#define TLV_TYPE_CALIB_DATA 1
#define TLV_TYPE_NEXT_BOOT_TYPE 2
#define BOOT_TYPE_TRACKING 0x00000000
#define BOOT_TYPE_STREAMING 0x5354524d

CalibrationSettings calib;
int streaming_requested = 0;

static void unpack_vec3d(Vec3d *v, const packed_v3_double& p) {
	v->x = p.x;
	v->y = p.y;
	v->z = p.z;
}

static void traverse_cb(void* data, uint16_t length, uint16_t type) {
	if (type == TLV_TYPE_CALIB_DATA) {
		if (length != 110) return;
		flash_storage_struct_v1 *fs = (flash_storage_struct_v1*)data;
		calib.gyro_offset_hw.x = fs->gyro_offs_hw.x;
		calib.gyro_offset_hw.y = fs->gyro_offs_hw.y;
		calib.gyro_offset_hw.z = fs->gyro_offs_hw.z;
		unpack_vec3d(&calib.gyro_offset_fine, fs->gyro_offs_fine);
		unpack_vec3d(&calib.mag_gain, fs->mag_gain);
		unpack_vec3d(&calib.mag_offs, fs->mag_offs);
		calib.accel_multiplier = fs->accel_gain;
		unpack_vec3d(&calib.accel_offset, fs->accel_offs);
    streaming_requested = 0;
	} else if (type == TLV_TYPE_NEXT_BOOT_TYPE) {
		if (length != 4) return;
    streaming_requested = ((*(uint32_t*)data) == BOOT_TYPE_STREAMING);
	}
}

void load_inertial_calibration() {
#ifdef DEFAULT_CALIB__GYRO_HW
	calib.gyro_offset_hw = Vec3<int16_t>(DEFAULT_CALIB__GYRO_HW);
#else
	calib.gyro_offset_hw = Vec3<int16_t>(0,0,0);
#endif

#ifdef DEFAULT_CALIB__GYRO_FINE 
	calib.gyro_offset_fine = Vec3d(DEFAULT_CALIB__GYRO_FINE);
#else
	calib.gyro_offset_fine = Vec3d(0,0,0);
#endif

#ifdef DEFAULT_CALIB__MAG_GAIN  
	calib.mag_gain = Vec3d(DEFAULT_CALIB__MAG_GAIN);
#else
	calib.mag_gain = Vec3d(1,1,1);
#endif

#ifdef DEFAULT_CALIB__MAG_OFFS  
	calib.mag_offs = Vec3d(DEFAULT_CALIB__MAG_OFFS);
#else
	calib.mag_offs = Vec3d(0,0,0);
#endif

	traverse_flash(traverse_cb);
}

static void pack_vec3d(packed_v3_double *packed, const Vec3d& v) {
	packed->x = v.x;
	packed->y = v.y;
	packed->z = v.z;
}
void store_calibration_data() {
	flash_storage_struct_v1 flash_data;
	flash_data.gyro_offs_hw.x = calib.gyro_offset_hw.x;
	flash_data.gyro_offs_hw.y = calib.gyro_offset_hw.y;
	flash_data.gyro_offs_hw.z = calib.gyro_offset_hw.z;
	pack_vec3d(&flash_data.gyro_offs_fine, calib.gyro_offset_fine);
	pack_vec3d(&flash_data.mag_gain, calib.mag_gain);
	pack_vec3d(&flash_data.mag_offs, calib.mag_offs);
	flash_data.accel_gain = calib.accel_multiplier;
	pack_vec3d(&flash_data.accel_offs, calib.accel_offset);
	store_data_tlv(&flash_data, sizeof(flash_data), 1);
}
void store_restart_streaming(int streaming) {
	uint32_t data = streaming ? BOOT_TYPE_STREAMING : BOOT_TYPE_TRACKING;
	store_data_tlv(&data, 4, TLV_TYPE_NEXT_BOOT_TYPE);
}

void print_calibration_data() {
  log_printf("calib data:\n\r");
  log_printf("gyro_offs_hw: %d %d %d\n\r", calib.gyro_offset_hw.x, calib.gyro_offset_hw.y, calib.gyro_offset_hw.z);
  log_printf("gyro_offs_sw: %.8f %.8f %.8f\n\r", calib.gyro_offset_fine.x, calib.gyro_offset_fine.y, calib.gyro_offset_fine.z);
  log_printf("accel_offs: %.8f %.8f %.8f\n\r", calib.accel_offset.x, calib.accel_offset.y, calib.accel_offset.z);
  log_printf("acc_mult: %.8f\n\r", calib.accel_multiplier);
  log_printf("mag_gain: %.8f %.8f %.8f\n\r", calib.mag_gain.x, calib.mag_gain.y, calib.mag_gain.z);
  log_printf("mag_offs: %.8f %.8f %.8f\n\r", calib.mag_offs.x, calib.mag_offs.y, calib.mag_offs.z);
}

void load_post_process_settings() {
  extern struct settings_post_proc pp_settings;
  pp_settings.filtering.rotation_slack = DEFAULT_PP_SLACK;
}

