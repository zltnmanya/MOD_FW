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
 
#include <10_sensor_input.h>
#include <20b_sensor_calib.h>
#include <common.h>
#include <logger.h>
#include <persistence.h>

extern "C" {
#include <MPU6050.h>
}

#include <libopencm3/cm3/systick.h>

#define SAMPLES_COARSE 2000
#define SAMPLES_FINE 10000
#define SAMPLES_TRASH 5000

int32_t gyro_sum[3];
uint32_t sample_count_coarse;
uint32_t sample_count_fine;
uint32_t sample_count_trash;

static void calibmode(int a) {
  mpu_gyro_offs_setup(a);
  MPU6050_setFIFOEnabled(0);
  MPU6050_resetFIFO();
  MPU6050_setFIFOEnabled(1);
}

static void zero() {
  gyro_sum[0] = 0;
  gyro_sum[1] = 0;
  gyro_sum[2] = 0;
}
void sensor_calib_init() {
  zero();
  sample_count_coarse = 0;
  sample_count_fine = 0;
  sample_count_trash = 0;
  calibmode(0);
}
void sensor_calib_feed_inertial(int16_t gyro[3]) {
  if (sample_count_trash < SAMPLES_TRASH) {
    sample_count_trash++;
    if (sample_count_trash == SAMPLES_TRASH) {
      log_printf("skipped trash\r\n");
    }
    return;
  }

  for (int i=0;i<3;i++) {
    gyro_sum[i] += gyro[i];
  }

  if (sample_count_coarse < SAMPLES_COARSE) {
#if 0
    if ((sample_count_coarse & 128) == 0) {
      log_printf("sample:%d %d %d\r\n", gyro[0], gyro[1], gyro[2]);
    }
#endif
    sample_count_coarse++;
    if (sample_count_coarse == SAMPLES_COARSE) {
      log_printf("sums:%ld %ld %ld\r\n", gyro_sum[0], gyro_sum[1], gyro_sum[2]);
      calib.gyro_offset_hw.x = (int16_t)roundf(-gyro_sum[0] / (float)(SAMPLES_COARSE * 4));
      calib.gyro_offset_hw.y = (int16_t)roundf(-gyro_sum[1] / (float)(SAMPLES_COARSE * 4));
      calib.gyro_offset_hw.z = (int16_t)roundf(-gyro_sum[2] / (float)(SAMPLES_COARSE * 4));
      log_printf("set:%d %d %d\r\n", calib.gyro_offset_hw.x, calib.gyro_offset_hw.y, calib.gyro_offset_hw.z);
      calibmode(1);
      sample_count_trash = 0;
      zero();
      log_printf("go:fine\r\n");
    }
  } else if (sample_count_fine < SAMPLES_FINE) {
    sample_count_fine++;
    if (sample_count_fine == SAMPLES_FINE) {
      calib.gyro_offset_fine.x = -((double)gyro_sum[0]) / SAMPLES_FINE;
      calib.gyro_offset_fine.y = -((double)gyro_sum[1]) / SAMPLES_FINE;
      calib.gyro_offset_fine.z = -((double)gyro_sum[2]) / SAMPLES_FINE;
      sw_state = BTN_STATE_IDLE;
      log_printf("go:idle\r\n");
      zero();
      store_calibration_data();
    }
  }
}
