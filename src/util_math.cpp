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
#include <math.h>

void quat2ypr(float *yaw, float *pitch, float *roll, float x, float y, float z, float w) {
  float sq_x = x * x;
  float sq_y = y * y;
  float sq_z = z * z;

  *yaw   = atan2f(2.0f * (w * x + y * z), 1.0f - 2.0f * (sq_x + sq_y));  /* -180..+180 */
  *pitch =  asinf(2.0f * (w * y - z * x));                               /* -90..+90 */
  *roll  = atan2f(2.0f * (w * z + x * y), 1.0f - 2.0f * (sq_y + sq_z));  /* -180..+180 */
}

