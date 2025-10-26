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
 
#ifndef _20_SENSOR_FUSION_H
#define _20_SENSOR_FUSION_H

#ifdef __cplusplus
#include <util_math.hpp>
#include <quaternion.h>

void sensor_fusion_feed_inertial(const quaternion::Quaternion<double>& gyro_quat, const Vec3f& v_accel);
void sensor_fusion_feed_mag(const Vec3f& v_mag);

extern "C" {
#endif

void sensor_fusion_reset();

#ifdef __cplusplus
}
#endif

#endif
