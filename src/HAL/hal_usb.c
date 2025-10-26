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
 
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/desig.h>
#include <stdio.h>
#include <common.h>
#include <usb_app.h>

extern usbd_device *usbd_dev;
extern struct usb_device_descriptor usb_descriptor_device;

static char dev_serial_no[25];

const char *usb_strings[] = {
  /* 1 */	"MOD",
  /* 2 */	"MOD (streaming mode)",
  /* 3 */	dev_serial_no,
};

void otg_fs_isr(void) {
  if (usbd_dev)
    usbd_poll(usbd_dev);
}

void usb_app_init() {
  uint32_t uid[3];
  desig_get_unique_id(uid);
  sprintf(dev_serial_no, "%08lx", uid[0]);
  sprintf(dev_serial_no + 8, "%08lx", uid[1]);
  sprintf(dev_serial_no + 16, "%08lx", uid[2]);

  usb_dev_init(usb_strings);
}

int usb_send_report(HIDReport_t *report) {
	uint16_t len;
	if (dev_mode_is_streaming()) {
		len = sizeof(HIDReport_streaming_t);
	} else {
#ifdef HID_REPORT_SEND_QUAT
		len = 17;
#else
		len = sizeof(HIDReport_tracking_t);
#endif
	}

  int rc = usbd_ep_write_packet(usbd_dev, 0x81, (uint8_t*)report, len);
  return rc == 0 ? -1 : 0;
}

int usb_app_try_write_com(char *buf, int len) {
  if (len > 64) len = 64;
  return usbd_ep_write_packet(usbd_dev, 0x83, buf, len) == 0 ? -1 : 0;
}
