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
 
#include <common.h>
#include <util_math.hpp>
#include <hal.h>
#include <msg_queue.h>
#include <diag.h>
#include <usb_hid_report_out.h>
#include <logger.h>
#include <usb_app.h>

msg_queue queue_main;

int init_complete = 0;
enum btn_press_state sw_state = BTN_STATE_IDLE;

typedef enum {
 dev_mode_tracking = 0,
 dev_mode_streaming = 1
} device_mode_t;

static device_mode_t dev_mode = dev_mode_tracking;
uint32_t reports_selected = REP_TYPE_IMU | REP_TYPE_MAG;

int dev_mode_is_tracking() {
  return dev_mode == dev_mode_tracking;
}
int dev_mode_is_streaming() {
  return dev_mode == dev_mode_streaming;
}
void dev_mode_set_streaming() {
  dev_mode = dev_mode_streaming;
  usb_set_hid_report_desc_streaming();
}

static char buf[64];
static int len = 0;
void handle_com_transfer(int keep_sending) {
  if (len <= 0) {
    len = log_consume(buf, 64);
  }
  if (len > 0) {
    do {
      if (usb_app_try_write_com(buf, len) == 0) {
        len = 0;
      }
    } while (keep_sending);
  }
}

void error_inf_loop(uint32_t err_code) {
  int led = 0;
	while (1) {
		gpio_status_led_set(led);
    led = !led;
		for (int i=0;i<1000000;i++) {
      if (fetch_n_clear_irq_flags(8)) {
        dump_cpu_state();
      }
      handle_com_transfer(1);
    }
	}
}

