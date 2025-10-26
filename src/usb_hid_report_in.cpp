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
 
#include <usb_hid_report_in.h>
#include <math.h>
#include <common.h>
#include <usb_app.h>
#include <hal.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>

static HIDReport_t report_1;
static HIDReport_t report_2;

static HIDReport_t *report_to_send = &report_1;
static HIDReport_t *report_to_modify = &report_2;

static int report_write_pos = 0;

void report_send() {
	usb_irq_disable();
  if (usb_send_report((HIDReport_t*)&(report_to_modify->tracking)) == 0) { /* NON-BLOCKING !!!! */ // ?? still?
		HIDReport_t *tmp = report_to_send;
		report_to_send = report_to_modify;
		report_to_modify = tmp;

		if (dev_mode_is_tracking()) {
#ifdef HID_REPORT_SEND_DBG_CTR
			memset(&report_to_modify->tracking.dbg, 0, sizeof(report_to_modify->tracking.dbg));
#endif
		}
		if (dev_mode_is_streaming()) {
			report_write_pos = 0;
			memset(&report_to_modify->streaming, 0, sizeof(HIDReport_t));
		}
	}
	usb_irq_enable(); // TODO: really need to disable IRQ here?
}

/* ================================================================================ */
/* ================================================================================ */
/* tracking */


#ifdef HID_REPORT_SEND_QUAT
void report_t_quat_set(float q1,float q2,float q3,float q4) {
  struct {
    float quat[4];
  } tmp;
	if (!dev_mode_is_tracking()) return;
  tmp.quat[0] = q1;
  tmp.quat[1] = q2;
  tmp.quat[2] = q3;
  tmp.quat[3] = q4;
  memcpy(report_to_modify->tracking.data + 1, &tmp, 17);
}

void report_t_set_indicator(int index, int value) { }
void report_t_set_dbg_u32(uint8_t start, uint32_t value) { }
void report_t_set_dbg_float(uint8_t start, float value) { }

#else
void report_t_orientation_set(float yaw, float pitch, float roll){
  const float rad2s16 = 32767.0f / PI;
	if (!dev_mode_is_tracking()) return;
	report_to_modify->tracking.yaw = round(yaw * rad2s16);
	report_to_modify->tracking.pitch = round(pitch * rad2s16);
	report_to_modify->tracking.roll = round(roll * rad2s16);
}

void report_t_set_indicator(int index, int value) {
#ifdef HID_REPORT_SEND_INDICATORS
  if (index < 0 || index >= 16) return;
	if (!dev_mode_is_tracking()) return;

  uint16_t mask = (1u<<index);

  if (value)
    report_to_modify->tracking.indicators |= mask;
  else
    report_to_modify->tracking.indicators &= ~mask;
#endif
}

void report_t_set_dbg_u32(uint8_t start, uint32_t value) {
#ifdef HID_REPORT_SEND_DBG_CTR
	if (start + sizeof(value) >= 32) return;
	if (!dev_mode_is_tracking()) return;
	*(uint32_t*)&(report_to_modify->tracking.dbg[start]) = value;
#endif
}

void report_t_set_dbg_float(uint8_t start, float value) {
#ifdef HID_REPORT_SEND_DBG_CTR
	if (start + sizeof(value) >= 32) return;
	if (!dev_mode_is_tracking()) return;
	*(float*)&(report_to_modify->tracking.dbg[start]) = value;
#endif
}
#endif

/* ================================================================================ */
/* ================================================================================ */
/* streaming */

#define REPORT_INDEX_MAX_VEC3F 16

#define REPORT_CHUNK_VAL_END       0
#define REPORT_CHUNK_VAL_IMU       1
#define REPORT_CHUNK_VAL_MAG       2
#define REPORT_CHUNK_VAL_TMP       3
#define REPORT_CHUNK_VAL_ORI_FUSED 4
#define REPORT_CHUNK_VAL_ORI_DEDREC 5
#define REPORT_CHUNK_VAL_VEC3F(_index_)     (255-(_index_))

static inline int has_room_for(int bytes) {
  return report_write_pos + bytes + 2 < 64;
}

static inline void append_u8(uint8_t value) {
	report_to_modify->streaming.data[report_write_pos++] = value;
}
static inline void append_float(float value) {
  append_u8(((uint8_t*)&value)[0]);
  append_u8(((uint8_t*)&value)[1]);
  append_u8(((uint8_t*)&value)[2]);
  append_u8(((uint8_t*)&value)[3]);
}
static inline void append_i16(int16_t value) {
	append_u8(((uint16_t)value) & 0xff);
	append_u8(((uint16_t)value) >> 8);
}

static inline void append_chunk_marker(uint8_t type, uint8_t length) {
  append_u8(type);
  append_u8(length);
}

int report_s_add_imu(int16_t *gyro, int16_t *accel) {
	if (!has_room_for(6*2)) return -1;
	if (!dev_mode_is_streaming()) return 0;
	if (reports_selected & REP_TYPE_IMU) {
		append_chunk_marker(REPORT_CHUNK_VAL_IMU, 12);

		append_i16(gyro[0]);
		append_i16(gyro[1]);
		append_i16(gyro[2]);
		append_i16(accel[0]);
		append_i16(accel[1]);
		append_i16(accel[2]);
	}

	return 0;
}

int report_s_add_mag(int16_t *mag, int16_t temp) {
	if (!has_room_for(4*2)) return -1;
	if (!dev_mode_is_streaming()) return 0;
	if (reports_selected & REP_TYPE_MAG) {
		append_chunk_marker(REPORT_CHUNK_VAL_MAG, 8);

		append_i16(mag[0]);
		append_i16(mag[1]);
		append_i16(mag[2]);
		append_i16(temp);
	}

	return 0;
}

int report_s_add_temp(int16_t temp_imu) {
	if (!has_room_for(1*2)) return -1;
	if (!dev_mode_is_streaming()) return 0;
	append_chunk_marker(REPORT_CHUNK_VAL_TMP, 2);

	append_i16(temp_imu);

	return 0;
}
int report_s_add_ori_fused(float yaw, float pitch, float roll) {
	if (!has_room_for(6*2)) return -1;
	if (!dev_mode_is_streaming()) return 0;
	if (reports_selected & REP_TYPE_ORI_FUSED) {
		append_chunk_marker(REPORT_CHUNK_VAL_ORI_FUSED, 12);
		append_i16((int16_t)round(yaw*(32767/180)));
		append_i16((int16_t)round(pitch*(32767/90)));
		append_i16((int16_t)round(roll*(32767/180)));
	}
	return 0;
}
int report_s_add_ori_dedrec(float yaw, float pitch, float roll) {
	if (!has_room_for(6*2)) return -1;
	if (!dev_mode_is_streaming()) return 0;
	if (reports_selected & REP_TYPE_ORI_DEDREC) {
		append_chunk_marker(REPORT_CHUNK_VAL_ORI_DEDREC, 12);
		append_i16((int16_t)round(yaw*(32767/180)));
		append_i16((int16_t)round(pitch*(32767/90)));
		append_i16((int16_t)round(roll*(32767/180)));
	}
	return 0;
}

int report_s_add_vec3f(const Vec3f& v, uint8_t index) {
  if (index >= REPORT_INDEX_MAX_VEC3F) error_inf_loop(0);
	if (!has_room_for(3*4)) return -1;
	if (!dev_mode_is_streaming()) return 0;
  
  append_chunk_marker(REPORT_CHUNK_VAL_VEC3F(index), 12);
  append_float(v.x);
  append_float(v.y);
  append_float(v.z);

  return 0;
}

