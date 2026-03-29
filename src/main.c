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
#include <usb_app.h>
#include <hal.h>
#include <logger.h>
#include <persistence.h>
#include <common.h>
#include <usb_hid_report_in.h>
#include <usb_hid_report_out.h>
#include <diag.h>
#include <I2Cdev.h>
#include <stdlib.h>

#define TMO_CALIB_BTN_PRESS 1000
#define TMO_CALIB_BTN_RELEASE 4000
int state_tmo = 0;

static void handle_msgq(union msg_u *msg) {
  switch (msg->msg_type) {
    case MSGQ_TYPE_I2C_MOD8: {
      uint8_t data;
      if (msg->i2c_mod8.mask != 0xff) {
        if (I2Cdev_readByte(msg->i2c_mod8.dev, msg->i2c_mod8.addr, &data) == -1)
          break;
        data = (data & ~(msg->i2c_mod8.mask)) | (msg->i2c_mod8.data & msg->i2c_mod8.mask);
      } else {
        data = msg->i2c_mod8.data;
      }
      I2Cdev_writeByte(msg->i2c_mod8.dev, msg->i2c_mod8.addr, data);
    }	  break;
    case MSGQ_TYPE_I2C_MOD16: {
      uint16_t data;
      if (msg->i2c_mod16.mask != 0xffff) {
        if (I2Cdev_readWord(msg->i2c_mod16.dev, msg->i2c_mod16.addr, &data) == -1)
          break;
        data = (data & ~(msg->i2c_mod16.mask)) | (msg->i2c_mod16.data & msg->i2c_mod16.mask);
      } else {
        data = msg->i2c_mod16.data;
      }
      I2Cdev_writeWord(msg->i2c_mod16.dev, msg->i2c_mod16.addr, data);
    }   break;
  }
}

static void handle_irq_flags() {
  if (fetch_n_clear_irq_flags(1))
    mpu_gyro_offs_setup(1);
  if (fetch_n_clear_irq_flags(2))
    store_calibration_data();
  if (fetch_n_clear_irq_flags(4))
    sensor_input_reset_fifo();
  if (fetch_n_clear_irq_flags(8)) {
    print_calibration_data();
    dump_cpu_state();
  }
}

int main(void)
{
  msgq_init(&queue_main);
  for (int i=0;i<DBG_CNT_COUNT;i++)
	  dbg_cnt[i] = 0;

  load_inertial_calibration();

  extern int streaming_requested;
  if (streaming_requested) {
    dev_mode_set_streaming();
  }

  hal_init();
  usb_app_init();
  hal_init_usb(); // USB INIT

  gpio_status_led_set(0);
  gpio_imu_reset_set(1);
  {	  /* Wait 1500ms make sure reset is complete (capacitors need time to discharge).
       * In the meanwhile, see if the button is presset to switch to calib. mode. */
	  uint32_t tick_start = get_common_tick();
	  int32_t btn_ctr = 0;
	  do {
		  if (gpio_button_get()) {
			  btn_ctr++;
			  gpio_status_led_set(1);
		  } else {
			  btn_ctr--;
			  gpio_status_led_set(0);
		  }
		  delay_ms(1);
	  } while(get_common_tick() - tick_start < 1500);
	  if (btn_ctr > 0)
		  dev_mode_set_streaming();
  }


  gpio_imu_reset_set(0);
  
  post_process_init();
  sensor_input_init();

  extern int init_complete;
  init_complete = 1;

  strobe_enable = 1;
  while (1) {
	  union msg_u *msg;
	  while ((msg = msgq_receive(&queue_main)) != 0) {
      handle_msgq(msg);
		  msgq_free(msg);
	  }

    handle_irq_flags();

	  sensor_input_fetch();

	  if (gpio_button_get() || fetch_n_clear_irq_flags(16)) {
		  sensor_fusion_reset();
	  }

    switch (sw_state) {
      case BTN_STATE_IDLE:
        if (gpio_button_get()) {
          if (state_tmo < TMO_CALIB_BTN_PRESS) {
            state_tmo++;
            if (state_tmo == TMO_CALIB_BTN_PRESS) {
              log_printf("will calib...\r\n");
            }
          }
        } else {
          if (state_tmo >= TMO_CALIB_BTN_PRESS) {
            state_tmo = 0;
            sw_state = BTN_STATE_RESTING;
            log_printf("go:resting\r\n");
          }
          state_tmo = 0;
        }
        break;
      case BTN_STATE_RESTING:
        if (state_tmo < TMO_CALIB_BTN_RELEASE) {
          state_tmo++;
        } else {
          state_tmo = 0;
          sensor_calib_init();
          sw_state = BTN_STATE_CALIBRATING;
          log_printf("go:calib\r\n");
        }
        break;
      case BTN_STATE_CALIBRATING:
        break;
    }

    if (sw_state != BTN_STATE_CALIBRATING) {
      report_send();
    }

    {
      char buf[64];
      int len = log_consume(buf, 64);
      usb_app_try_write_com(buf, len); // TODO: not the best solution... some lines may be lost here
    }

    if (strobe_clear) {
      gpio_status_led_set(1);
      strobe_clear = 0;
    }
  }
}


