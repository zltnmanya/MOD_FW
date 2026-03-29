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
 
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#ifdef __cplusplus
#include <util_math.hpp>

struct CalibrationSettings {
	Vec3<int16_t> gyro_offset_hw; /* 1 diff in gyro_offset_hw -> 4 diff in measurement */
	Vec3d         gyro_offset_fine;
	Vec3d         accel_offset;
	Vec3d         mag_gain, mag_offs;
	double        accel_multiplier;
};

struct settings_post_proc {
  struct {
    double rotation_slack;
  } filtering;
};

extern CalibrationSettings calib;

extern "C" {
#endif

void load_inertial_calibration();
void load_post_process_settings();
void store_calibration_data();
void print_calibration_data();
void store_restart_streaming(int streaming);

#ifdef __cplusplus
}
#endif
#endif
