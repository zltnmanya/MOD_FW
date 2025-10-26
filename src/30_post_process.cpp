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
 
#include <30_post_process.h>
#include <persistence.h>
#include <usb_hid_report_in.h>
#include <params.h>
#include <logger.h>

#include <quaternion.h>
#include <libopencm3/cm3/systick.h>

using namespace quaternion;

class Backlash {
  double slack;
  double last;
  public:
  Backlash(double slack = 0.0) {
    setBacklash(slack);
  }
  void setBacklash(double slack) {
    this->slack = slack;
  }
  double eval(double v) {
    if (v < last - slack) {
      last = v + slack;
    } else if (v > last + slack) {
      last = v - slack;
    }
    return last;
  }
};

static Backlash sl_yaw;
static Backlash sl_pitch;

struct settings_post_proc pp_settings;

void post_process_init() {
  load_post_process_settings();

  sl_yaw.setBacklash(pp_settings.filtering.rotation_slack);
  sl_pitch.setBacklash(pp_settings.filtering.rotation_slack);
}

//static uint32_t last_log_ts = 0;

void post_process_feed(const Quaternion<float>& rotation) {
#if 0
int do_log_cycle = 0;
if (TICK_TIMEOUT(last_log_ts, 100)) {
  last_log_ts = tick_count;
  do_log_cycle = 1;
}
#endif

#ifdef HID_REPORT_SEND_QUAT
  report_t_quat_set(rotation.a(), rotation.c(), rotation.d(), rotation.b());
#else
  float yaw, pitch, roll;
  quat2ypr(VECSWAP_YPR(&yaw, &pitch, &roll),
      /* up */     rotation.b(),
      /* left */   rotation.c(),
      /* back */   rotation.d(),
      rotation.a());

  if (OUTPUT_INVERT_YAW) yaw = -yaw;
  if (OUTPUT_INVERT_PITCH) pitch = -pitch;
  if (OUTPUT_INVERT_ROLL) roll = -roll;

  // ypr now in degrees

  report_s_add_ori_fused(yaw,pitch,roll);

  yaw = sl_yaw.eval(yaw);
  pitch = sl_pitch.eval(pitch);

  report_t_orientation_set(yaw, pitch, roll);
#endif
}
