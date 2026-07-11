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
#include <20_sensor_fusion.h>
#include <20b_sensor_calib.h>
#include <30_post_process.h>
#include <util_math.hpp>
#include <persistence.h>
#include <logger.h>
#include <diag.h>
#include <hal.h>
#include <math.h>
#include <usb_hid_report_in.h>
#include <common.h>
#include <params.h>

#include <I2Cdev.h>
extern "C" {
#include <MPU6050.h>
#include <QMC5883L.h>
}
#include <libopencm3/stm32/i2c.h>
#include <quaternion.h>

#define VECSWAP_IMU2(_back_, _right_, _up_) (_back_), (_right_), (_up_)

#define IMU_FIFO_PACKET_SIZE (6*2)

using namespace quaternion;

struct sensor_correction_coeffs {
    float   b_x,    b_y,    b_z;
    float   s_x, /* 0,      0 */
            a_xy,   s_y, /* 0 */
            a_xz,   a_yz,   s_z;
};

struct sensor_correction_coeffs accel_calib = {
   DEFAULT_CALIB__ACCEL_BIAS,
   DEFAULT_CALIB__ACCEL_ALIGN
};
struct sensor_correction_coeffs magnetometer_calib = {
   DEFAULT_CALIB__ACCEL_BIAS,
   DEFAULT_CALIB__ACCEL_ALIGN
};

static inline Vec3f correct(int16_t *raw, const struct sensor_correction_coeffs& coeffs) {
  float x = raw[0] / 16384.0f - coeffs.b_x;
  float y = raw[1] / 16384.0f - coeffs.b_y;
  float z = raw[2] / 16384.0f - coeffs.b_z;

  float fx = x * coeffs.s_x;
  float fy = x * coeffs.a_xy + y * coeffs.s_y;
  float fz = x * coeffs.a_xz + y * coeffs.a_yz + z * coeffs.s_z;

  return Vec3f(fx, fy, fz);
}

void sensor_input_reset_fifo() {
	MPU6050_resetFIFO();
}
static int chk_i2c_bus_error() {
  if (I2Cdev_get_error() != 0)
    return -1;
  return 0;
}
static void wait_for_connection() {
	uint32_t tick_start = get_common_tick();
	uint32_t tick;
  uint8_t tx_buf[1];
  uint8_t rx_buf[1];
  tx_buf[0] = MPU6050_RA_WHO_AM_I;
	do {
    delay_ms(100);
    i2c_transfer7_tmo(I2C1, MPU6050_DEFAULT_ADDRESS, tx_buf, 1, rx_buf, 1, 100);
    if (chk_i2c_bus_error() == 0) return;
    I2Cdev_clear_error();
		tick = get_common_tick();
	} while (tick - tick_start < 5000);
	error_inf_loop(0);
}

void mpu_gyro_offs_setup(int calibrated) {
  if (calibrated) {
    MPU6050_setXGyroOffset(calib.gyro_offset_hw.x);
    MPU6050_setYGyroOffset(calib.gyro_offset_hw.y);
    MPU6050_setZGyroOffset(calib.gyro_offset_hw.z);
  } else {
	  MPU6050_setXGyroOffset(0);
	  MPU6050_setYGyroOffset(0);
	  MPU6050_setZGyroOffset(0);
  }
  MPU6050_resetGyroscopePath();
  MPU6050_resetFIFO();
}

void sensor_input_init() {
  MPU6050(MPU6050_ADDRESS_AD0_LOW);

  wait_for_connection();

  MPU6050_initialize();

  MPU6050_reset();
  delay_ms(100);
  I2Cdev_writeByte(MPU6050_ADDRESS_AD0_LOW, MPU6050_RA_USER_CTRL, 0b111);
  delay_ms(100);

  MPU6050_setSleepEnabled(0);
  MPU6050_setIntEnabled(0);
  MPU6050_setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  MPU6050_setDLPFMode(MPU6050_DLPF_BW_188); // 188Hz -- lower rate; not too much delay

  MPU6050_setDMPEnabled(0);
  MPU6050_setStandbyXAccelEnabled(0);
  MPU6050_setStandbyYAccelEnabled(0);
  MPU6050_setStandbyZAccelEnabled(0);
  MPU6050_setStandbyXGyroEnabled(0);
  MPU6050_setStandbyYGyroEnabled(0);
  MPU6050_setStandbyZGyroEnabled(0);
  MPU6050_setTempSensorEnabled(1);
  MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
#if (IMU_SAMPLERATE_DIVIDER < 1)
#error IMU_SAMPLERATE_DIVIDER must be >= 1
#endif
  MPU6050_setRate(IMU_SAMPLERATE_DIVIDER - 1);
  /* raw data: 16 bit signed values (-32768..32767) 
   * measurement range: -250..250 deg/sec
   * measurement rate: 1000 Hz
   * divide by 2 for use as quaternion
   */

  MPU6050_setI2CBypassEnabled(1);
  if (!QMC5883L_soft_reset()) error_inf_loop(0);
  delay_ms(10);
  if (!QMC5883L_fbr_set(0x4)) error_inf_loop(0);
  delay_ms(10);
  static const uint8_t ctrl1_value = QMC5883L_CTRL1_VALUE(QMC5883L_MODE_CONT, QMC5883L_OUTPUT_RATE_200HZ, QMC5883L_SCALE_2G, QMC5883L_OVERSAMPLE_512);
  if (!QMC5883L_control_1_set(ctrl1_value)) error_inf_loop(0);

  if (dev_mode_is_streaming())
    mpu_gyro_offs_setup(0);
  else
    mpu_gyro_offs_setup(1);

  MPU6050_setXGyroFIFOEnabled(1);
  MPU6050_setYGyroFIFOEnabled(1);
  MPU6050_setZGyroFIFOEnabled(1);
  MPU6050_setAccelFIFOEnabled(1);
  MPU6050_setFIFOEnabled(0);
  MPU6050_resetFIFO();
  MPU6050_setFIFOEnabled(1);
  log_printf("rate: %d\r\n", MPU6050_getRate());
  log_printf("dlpf: %d\r\n", MPU6050_getDLPFMode()); 
}

#ifndef USE_I2C_IRQS

static void reset_fifo() {
  dbg_cnt[1]++;
  MPU6050_setFIFOEnabled(0);
  uint8_t trash[1024];
  MPU6050_getFIFOBytes(trash, 1024);
  MPU6050_resetFIFO(); // checked: it does return to 0 after reset
  MPU6050_setFIFOEnabled(1);
}

static uint16_t get_mpu_packet_counter() {
  extern int init_complete;
  init_complete = 0;
  do {
    uint16_t count;
    do {
      count = MPU6050_getFIFOCount();
      if (count == 1024) {
        reset_fifo();
        continue;
      }
      if ((count % IMU_FIFO_PACKET_SIZE) != 0) {
        continue;
      }
      break;
    } while (1);


    if (count > 400) { // > threshold -> reset fifo
      reset_fifo();
      continue;
    }

    return count;
  } while (1);
}

static void packet2vectors(int16_t *accel, int16_t *gyro, const void *pdata) {
  uint16_t *pdata16 = (uint16_t*)pdata;

  accel[0] = be16toh(pdata16[0]);
  accel[1] = be16toh(pdata16[1]);
  accel[2] = be16toh(pdata16[2]);
  gyro[0] = be16toh(pdata16[3]);
  gyro[1] = be16toh(pdata16[4]);
  gyro[2] = be16toh(pdata16[5]);
}

static Quaternion<double> gyro_to_quat(const Vec3d& gyro) {
    /* board orientation */
    Vec3d gyro_vec(VECSWAP_GET_V3(gyro));
    double vlen = gyro_vec.abs();
    double angle_half = vlen * ((250.0 / 180.0) * PI / 1000 / 2);
    gyro_vec /= vlen;
    gyro_vec *= sin(angle_half);
    return Quaternion<double>(cos(angle_half), gyro_vec.x, gyro_vec.y, gyro_vec.z);
}

void sensor_input_fetch() {
  uint8_t packet[1024];

  usb_irq_disable();
  uint16_t count = get_mpu_packet_counter();
  usb_irq_enable();

  if (count >= IMU_FIFO_PACKET_SIZE) {
    int16_t c = count;
    usb_irq_disable();
    timestamp_start(0);
    MPU6050_getFIFOBytes(packet, c); //Get 1 packet
    timestamp_stamp(0,1);
    usb_irq_enable();
    uint8_t *pstart = packet;
    do {
      dbg_cnt[0]++;

      int16_t arr_a[3], arr_g[3];
      packet2vectors(arr_a, arr_g, pstart);

      report_s_add_imu(arr_g, arr_a);


      if (sw_state != BTN_STATE_CALIBRATING) {
        /* apply calibration values and scale */
        Vec3d gyro_vec_tmp = (Vec3d(arr_g[0], arr_g[1], arr_g[2]) + calib.gyro_offset_fine) / 32768; /* [-1.0 .. +1.0] per axis */
        Quaternion<double> gyro_quat = gyro_to_quat(gyro_vec_tmp);

        /* apply calibration values and scale */
        Vec3f v_accel_tmp = correct(arr_a, accel_calib);
        /* board orientation */
        Vec3f v_accel(VECSWAP_GET_V3(v_accel_tmp));

        /* feed to sensor fusion */
        sensor_fusion_feed_inertial(gyro_quat, v_accel);
      } else {
        sensor_calib_feed_inertial(arr_g);
      }

      pstart += IMU_FIFO_PACKET_SIZE;
      c -= IMU_FIFO_PACKET_SIZE;
    } while (c > 0);
    timestamp(1);
  }

  if (gpio_drdy_get()) {
    int16_t arr[3];
    int16_t mag_temp;

    usb_irq_disable();
    QMC5883L_magGet(arr);
    if (dev_mode_is_streaming()) {
      QMC5883L_tempGet(&mag_temp);
    }
    usb_irq_enable();

    if (dev_mode_is_streaming()) {
      report_s_add_mag(arr, mag_temp);
    }
    //log_printf("mag: %d %d %d\r\n", VECSWAP_IMU(arr[0], arr[1], arr[2]));
    Vec3d v_mag_tmp = correct(arr, magnetometer_calib);
    /* physical range: -2..+2 Gauss; logical range: -32768..+32767 */
    Vec3d v_mag(VECSWAP_GET_V3(v_mag_tmp));
    sensor_fusion_feed_mag(v_mag);
  }

  if (dev_mode_is_streaming()) {
    static int imu_temp_send_counter = 0;
    imu_temp_send_counter++;
    if (imu_temp_send_counter >= 5 && count >= IMU_FIFO_PACKET_SIZE) {
      imu_temp_send_counter = 0;
      usb_irq_disable();
      report_s_add_temp(MPU6050_getTemperature());
      usb_irq_enable();
    }
  }
}
#endif
