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
 
#ifndef USB_HID_REPORT_IN_H_
#define USB_HID_REPORT_IN_H_

#include <stdint.h>

#ifdef __cplusplus
#include <util_math.hpp>
extern "C" {
#endif

void report_send();

/* HID report IN / tracking */
#ifdef HID_REPORT_SEND_QUAT
void report_t_quat_set(float q1, float q2, float q3, float q4);
#else
void report_t_orientation_set(float yaw, float pitch, float roll);
#endif
void report_t_set_indicator(int index, int value);
void report_t_set_dbg_u32(uint8_t start, uint32_t value);
void report_t_set_dbg_float(uint8_t start, float value);

/* HID report IN / streaming */
int report_s_add_imu(int16_t *gyro, int16_t *accel);
int report_s_add_mag(int16_t *mag, int16_t temp);
int report_s_add_temp(int16_t temp_imu);
int report_s_add_ori_fused(float yaw, float pitch, float roll);
int report_s_add_ori_dedrec(float yaw, float pitch, float roll);

#ifdef __cplusplus
}
int report_s_add_vec3f(const Vec3f& v, uint8_t index);
#endif

#endif /* USB_HID_REPORT_IN_H_ */
