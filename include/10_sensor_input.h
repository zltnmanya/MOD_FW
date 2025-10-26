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
 
#ifndef _10_SENSOR_INPUT_H_ 
#define _10_SENSOR_INPUT_H_ 

#ifdef __cplusplus
extern "C" {
#endif

void sensor_input_init();
void sensor_input_fetch();
void sensor_input_reset_fifo();

void mpu_gyro_offs_setup(int calibrated);

#ifdef __cplusplus
}
#endif

#endif /* 10_SENSOR_INPUT_H_ */
