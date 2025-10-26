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
 
#ifndef USB_APP_H
#define USB_APP_H

#include <stdint.h>
#include <stdlib.h>

#ifdef HID_REPORT_SEND_QUAT
 typedef struct __attribute__((packed)) HIDReport_tracking_s {
  uint8_t data[17];
 } HIDReport_tracking_t;
#else
 typedef struct HIDReport_tracking_s {
 	int16_t yaw;
 	int16_t pitch;
 	int16_t roll;
#ifdef HID_REPORT_SEND_INDICATORS
 	uint16_t indicators;
#endif
#ifdef HID_REPORT_SEND_DBG_CTR
 	uint8_t dbg[32];
#endif
 } HIDReport_tracking_t;
#endif

 typedef struct HIDReport_streaming_s {
   uint8_t data[64];
 } HIDReport_streaming_t;


 typedef union {
   HIDReport_tracking_t tracking;
   HIDReport_streaming_t streaming;
 } HIDReport_t;

#define HID_OUT_REPORT_BUF_SIZE     36U

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t hid_report_descriptor_tracking[];
extern const uint8_t hid_report_descriptor_streaming[];

void usb_dev_init(const char **usb_strings);

void usb_app_init();
int usb_send_report(HIDReport_t *report);
void usb_set_hid_report_desc_streaming();

int usb_app_try_write_com(char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
