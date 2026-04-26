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
 
#ifndef _PARAMS_H_
#define _PARAMS_H_

#include <util_math.hpp>

#define IMU_SAMPLERATE_DIVIDER 1

/* IMU orientation on board */
// #define VECSWAP_IMU(x, y, z) VECSWAP_IMU2((x),(y),(z))
// #define VECSWAP_IMU(x, y, z) VECSWAP_IMU2((z),-(y),(x))

// #define VECSWAP_YPR(y, p, r) (r),(p),(y) // for headset (front)
#define VECSWAP_YPR(y, p, r) (y),(p),(r)

// #define VECSWAP_GET_V3(v) -(v.z), -(v.y), -(v.x) // for headset (front)
#define VECSWAP_GET_V3(v) (v.z), -(v.y), (v.x) // for modular setup (top)

#define DEFAULT_PP_SLACK (0.3 * PI / 180.0);

/* sensor fusion parameters */
static const double gyro_rest_value_threshold = cos(20.0*PI/180/1000/180*250/2); /* gyro resting criteria: max movement */
static const int gyro_rest_cycle_threshold = 30; /* gyro resting criteria: duration of max movement checking */
static const float c_threshold_accel_rest_valid = 0.2f; /* accel. resting chreshold -- max diff from 1G  */
static const float c_threshold_mag_valid = 0.1f; /* mag. valid threshold -- max diff from calibrated mag. field strength */
static const float rotation_max =  5.0f; /* max rotation of abs. orientation from stored zero orientation */
static const float a_avg_factor = 0.01f; /* rate at which G vector is updated */
static const float m_avg_factor = 0.02f; /* rate at which M vector is updated */
static const double r_avg_factor = 0.01; /* rate at which ded.rec. orientation is updated using abs. orientation */

#endif
