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

static void traverse_cb(void* data, uint16_t length, uint16_t type) {
	if (type == TLV_TYPE_CALIB_DATA) {
		if (length != 30) return;
		flash_storage_struct_v1 *fs = (flash_storage_struct_v1*)data;
		calib.gyro_offset_hw.x = fs->gyro_offs_hw.x;
		calib.gyro_offset_hw.y = fs->gyro_offs_hw.y;
		calib.gyro_offset_hw.z = fs->gyro_offs_hw.z;
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

  calib.accelerometer = {
     DEFAULT_CALIB__ACCEL_BIAS,
     DEFAULT_CALIB__ACCEL_ALIGN
  };
  calib.magnetometer = {
     DEFAULT_CALIB__ACCEL_BIAS,
     DEFAULT_CALIB__ACCEL_ALIGN
  };
	traverse_flash(traverse_cb);
}

void store_calibration_data() {
	flash_storage_struct_v1 flash_data;
	flash_data.gyro_offs_hw.x = calib.gyro_offset_hw.x;
	flash_data.gyro_offs_hw.y = calib.gyro_offset_hw.y;
	flash_data.gyro_offs_hw.z = calib.gyro_offset_hw.z;
	store_data_tlv(&flash_data, sizeof(flash_data), TLV_TYPE_CALIB_DATA);
}
void store_restart_streaming(int streaming) {
	uint32_t data = streaming ? BOOT_TYPE_STREAMING : BOOT_TYPE_TRACKING;
	store_data_tlv(&data, 4, TLV_TYPE_NEXT_BOOT_TYPE);
}

static void print_calib_coeffs(const char* name, const struct sensor_correction_coeffs& coeffs) {
  log_printf("%s   bias: %.8f %.8f %.8f\r\n", name, coeffs.b_x, coeffs.b_y, coeffs.b_z);
  log_printf("%s coeffs:\r\n", name);
  log_printf("  %.8f\r\n", coeffs.s_x);
  log_printf("  %.8f %.8f\r\n", coeffs.a_xy, coeffs.s_y);
  log_printf("  %.8f %.8f %.8f\r\n", coeffs.a_xz, coeffs.a_yz, coeffs.s_z);
}
void print_calibration_data() {
  log_printf("calib data:\r\n");
  log_printf("gyro_offs_hw: %d %d %d\r\n", calib.gyro_offset_hw.x, calib.gyro_offset_hw.y, calib.gyro_offset_hw.z);
  log_printf("gyro_offs_sw: %.8f %.8f %.8f\r\n", calib.gyro_offset_fine.x, calib.gyro_offset_fine.y, calib.gyro_offset_fine.z);
  print_calib_coeffs("accelerometer", calib.accelerometer);
  print_calib_coeffs("magnetometer", calib.magnetometer);
}

void load_post_process_settings() {
  extern struct settings_post_proc pp_settings;
  pp_settings.filtering.rotation_slack = DEFAULT_PP_SLACK;
}

