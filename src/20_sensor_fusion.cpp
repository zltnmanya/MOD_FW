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
#include <30_post_process.h>
#include <common.h>
#include <usb_hid_report_in.h>
#include <logger.h>
#include <params.h>
#include <diag.h>
#include <util_math.hpp>

#include <libopencm3/cm3/systick.h>
#include <quaternion.h>

using namespace quaternion;

////////////////////////////////////////////////////////////////////////////////
// constants
//
#define IMU_MEAS_PERIOD (IMU_SAMPLERATE_DIVIDER / 1000.0 /* Hz */)

////////////////////////////////////////////////////////////////////////////////
// types
//
typedef double dedrec_t;

////////////////////////////////////////////////////////////////////////////////
// variables
//

static Quaternion<double> rotation_dedrec(1.0, 0.0, 0.0, 0.0);
//static Quaternion<double> rotation_dedrec_only; // for debugging purposes only

//static uint32_t last_log_ts = 0;

static int gyro_in_rest_ctr = 0;

static Vec3f v_mag_last; /* magnetic field vector */
static Vec3f v_accel_last; /* acceleration vector */
static Matrix3x3<float> mtr_zero; /* stored matrix that rotates to the "zero" orientation set by the user */

////////////////////////////////////////////////////////////////////////////////
// helper functions:
//

/* checks if all sensors indicate resting (acceleration and rotation within limits) */
static inline int is_resting() {
  /* check gyro resting */
  if (gyro_in_rest_ctr < gyro_rest_cycle_threshold) {
    report_t_set_indicator(2, 0);
    return 0;
  }
  report_t_set_indicator(2, 1);

  /* check accel inside 1.0 +/- threshold (1.0 is normalized value for 1g) */
  /* as usual: try to spare the CPU from needlessly calculating square roots */
  static const RangeSqareCheck<float> v_accel_threshold(1.0f, c_threshold_accel_rest_valid);
  float vacc_sq = v_accel_last.square();
  if (!v_accel_threshold.inside(vacc_sq)) {
    report_t_set_indicator(3, 0);
    return 0;
  }
  report_t_set_indicator(3, 1);

  /* check magnetometer inside 1.0 +/- threshold (1.0 is normalized value for calibrated magnetic field) */
  /* as usual: try to spare the CPU from needlessly calculating square roots */
  static const RangeSqareCheck<float> v_mag_threshold(1.0f, c_threshold_mag_valid);
  float vmag_sq = v_mag_last.square();
  if (!v_mag_threshold.inside(vmag_sq)) {
    report_t_set_indicator(4, 0);
    return 0;
  }
  report_t_set_indicator(4, 1);


#if 0
  if (do_log_cycle) {
    if (dbg_flag_get(1))
      log_printf("va:%.3f vm:%.3f gr:%d\r\n", (double)vacc_sq, (double)vmag_sq, gyro_in_rest_ctr);
  }
#endif
  return 1;
}

/* check if absolute rotation is facing roughly forward and horizontal */
static inline int is_near_center(const Quaternion<float>& rotation_absolute) {
  static const float rotation_max_halcos = cos(rotation_max*PI/180/2);
  return rotation_absolute.a() >= rotation_max_halcos;
}

template<typename T>
static Matrix3x3<T> g_mag_to_base(const Vec3<T>& g, const Vec3<T>& mag) {
  Vec3<T> a;
  Vec3<T> b;
  Vec3<T> c;

  a = g;
  a.normalize();
  b = mag;
  c = a ^ b;
  c.normalize(); // east-west direction

  b = a ^ c;

  return Matrix3x3<float>(c,b,a);
}

static const Quaternion<float> quat_zero(1, 0,0,0);
void sensor_fusion_reset() {
  rotation_dedrec = quat_zero;
  //rotation_dedrec_only.setZero();

  /* store "zero" orientation */
  mtr_zero = g_mag_to_base(v_accel_last, v_mag_last);
  mtr_zero.invert();
}

void sensor_fusion_feed_inertial(const Quaternion<double>& gyro_quat, const Vec3f& v_accel) {
  Quaternion<float> gyro_quat_f(gyro_quat);
#if 0
  int do_log_cycle = 0;
if (TICK_TIMEOUT(last_log_ts, 100)) {
  last_log_ts = tick_count;
  do_log_cycle = 1;
}
#endif

  /* check if gyro is at rest */
  if (gyro_quat.a() < gyro_rest_value_threshold) {
    gyro_in_rest_ctr = 0;
    report_t_set_indicator(0, 0);
  } else {
    if (gyro_in_rest_ctr < gyro_rest_cycle_threshold) {
      gyro_in_rest_ctr++;
      report_t_set_indicator(0, 0);
    } else
      report_t_set_indicator(0, 1);
  }

  /* update accel and mag vectors using gyro */
  {
    Quaternion<float> inverse_rotation(gyro_quat_f.a(), -gyro_quat_f.b(), -gyro_quat_f.c(), -gyro_quat_f.d());
    v_accel_last = rotate(v_accel_last, inverse_rotation); // direction seems ok: should be negative; rate is a bit high for some reason
    v_mag_last = rotate(v_mag_last, inverse_rotation);
  }

  /* update accel vector */
  v_accel_last += (v_accel - v_accel_last) * a_avg_factor;

  if (is_resting()) {
    Quaternion<float> rotation_absolute;
#if 0
  if (do_log_cycle) {
    if (dbg_flag_get(2)) {
      log_printf("%.2f %.2f %.2f | ", v_accel_last.x, v_accel_last.y, v_accel_last.z);
      log_printf("%.2f %.2f %.2f ==> ", v_mag_last.x, v_mag_last.y, v_mag_last.z);
      log_printf("%.2f %.2f %.2f | ", i.x, i.y, i.z);
      log_printf("%.2f %.2f %.2f | ", j.x, j.y, j.z); 
      log_printf("%.2f %.2f %.2f\r\n", k.x, k.y, k.z);
    }
  }
#endif

    /* calculate transformation matrix (base vectors) from accel and mag vectors */

    /* get rotation quaternion from matrix (rotation needed to transform union matrix to stored "zero" orientation) */
    {
      Matrix3x3<float> mtr = g_mag_to_base(v_accel_last, v_mag_last);
#if 0
      { /* mtr seems ok */
        static int refresh_count = 0;
        refresh_count++;
        if (refresh_count == 30) {
          for (int i=0;i<3;i++) {
            static const int colors[3] = {2,3,5};
            Vec3f v_mat(mtr.values[i][0],mtr.values[i][1],mtr.values[i][2]);
            v_mat *= 0.3f;
            report_s_add_vec3f(v_mat, colors[i]);
          }
        }
        if (refresh_count >= 50) {
          refresh_count = 0;
        }
      }
#endif
      mtr = mtr_zero * mtr;
      std::array<std::array<float, 3>, 3> mtr2;
      for (int i=0;i<3;i++)
        for (int j=0;j<3;j++)
          mtr2[i][j]=mtr.values[i][j];
#if 0
      {
        static int refresh_count = 0;
        refresh_count++;
        if (refresh_count == 40) {
          for (int i=0;i<3;i++) {
            static const int colors[3] = {6,7,8};
            Vec3f v_mat(mtr2[i][0],mtr2[i][1],mtr2[i][2]);
            report_s_add_vec3f(v_mat, colors[i]);
          }
        }
        if (refresh_count >= 50) {
          refresh_count = 0;
        }
      }
#endif
      rotation_absolute = from_rotation_matrix(mtr2);
#if 0
      {
        static int refresh_count = 0;
        refresh_count++;
        if (refresh_count == 40) {
          log_printf("abs_rot:%.2f\r\n", acosf(rotation_absolute.a())*2*180/PI);
        }
        if (refresh_count >= 50) {
          refresh_count = 0;
        }
      }
#endif
    }
    /* rotation_absolute ok, tested */

    if (is_near_center(rotation_absolute)) {
      report_t_set_indicator(1, 1);
      /* update dedrec rotation using measured absolute rotation  */
      Quaternion<double> diff = Quaternion<double>(rotation_absolute) * conj(rotation_dedrec);

      /* rotate by factor of r_avg_factor (bit sloppy, but it'll do here) */
      Quaternion<double> diff_2(0.0, diff.b() * r_avg_factor, diff.c() * r_avg_factor, diff.d() * r_avg_factor);
      diff_2.normalize_real();
      rotation_dedrec = rotation_dedrec * diff_2;
    } else {
      report_t_set_indicator(1, 0);
    }
  } else {
    report_t_set_indicator(1, 0);
  }

  /* update dedrec rotation (integrate gyro quaternion) */
  rotation_dedrec = normalize(rotation_dedrec * gyro_quat);
#if 0
  if (do_log_cycle) {
    log_printf("%.2f %.2f %.2f %.2f\r\n", rotation_dedrec.a(), rotation_dedrec.b(), rotation_dedrec.c(), rotation_dedrec.d());
  }
#endif

#if 0
  if (reports_selected & REP_TYPE_ORI_DEDREC) {
    rotation_dedrec_only = normalize(rotation_dedrec_only  * gyro_quat);
	  double yaw, pitch, roll;
	    quat2ypr(&yaw, &pitch, &roll,
	        /* up */    -rotation_dedrec_only.z,
	        /* left */  -rotation_dedrec_only.y,
	        /* back */   rotation_dedrec_only.x,
			rotation_dedrec_only.w);
	  report_s_add_ori_dedrec(yaw,pitch,roll);
  }
#endif
  {
    static int refresh_count = 0;
    refresh_count++;
    if (refresh_count >= 50) {
      report_s_add_vec3f(v_accel, 1);
      report_s_add_vec3f(v_accel_last, 9);
      refresh_count  = 0;
    }
  }
  post_process_feed(rotation_dedrec);
}
void sensor_fusion_feed_mag(const Vec3f& v_mag) {
  v_mag_last += (v_mag - v_mag_last) * m_avg_factor;
  {
    static int refresh_count = 0;
    refresh_count++;
    if (refresh_count >= 15) {
      report_s_add_vec3f(v_mag, 4);
      report_s_add_vec3f(v_mag_last, 12);
      refresh_count  = 0;
    }
  }
}
