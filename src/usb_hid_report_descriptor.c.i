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
 
// vim: set syntax=c:

/* input hid report is 64 bytes at max when using full speed usb */

const uint8_t hid_report_descriptor_tracking[] = {
#ifdef HID_REPORT_SEND_QUAT
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)

    /* HID report IN */
		0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
		0x15, 0x80,  //   Logical Minimum (-128)
		0x25, 0x7F,  //   Logical Maximum (127)

		0x75, 0x08,        //   Report Size (8)
		0x95, 0x01,        //   Report Count (1)
		0x09, 0x32,        //   Usage ()
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

		0x75, 0x20,        //   Report Size (32)
		0x95, 0x01,        //   Report Count (1)
		0x09, 0x33,        //   Usage (Rx)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    /* int16_t pitch[1] */
		0x09, 0x34,        //   Usage (Ry)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    /* int16_t roll[1] */
		0x09, 0x35,        //   Usage (Rz)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

		0x09, 0x31,        //   Usage ()
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
#else
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)

    /* HID report IN */
    /* int16_t yaw[1] */
		0x16, 0x00, 0x80,  //   Logical Minimum (-32768)
		0x36, 0x4C, 0xFF,  //   Physical Minimum (-180)
		0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
		0x65, 0xB4,        //   Unit (System: English Rotation)
		0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
		0x46, 0xB4, 0x00,  //   Physical Maximum (180)
		0x09, 0x33,        //   Usage (Rx)
		0x75, 0x10,        //   Report Size (16)
		0x95, 0x01,        //   Report Count (1)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    /* int16_t pitch[1] */
		0x35, 0xA6,        //   Physical Minimum (-90)
		0x45, 0x5A,        //   Physical Maximum (90)
		0x09, 0x34,        //   Usage (Ry)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    /* int16_t roll[1] */
		0x36, 0x4C, 0xFF,  //   Physical Minimum (-180)
		0x46, 0xB4, 0x00,  //   Physical Maximum (180)
		0x09, 0x35,        //   Usage (Rz)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

#ifdef HID_REPORT_SEND_INDICATORS
		0x15, 0x00,        //   Logical Minimum (1)
		0x35, 0x00,        //   Physical Minimum (1)
		0x05, 0x09,        //   Usage Page (buttons)
		0x25, 0x01,        //   Logical Maximum (1)
		0x45, 0x01,        //   Physical Maximum (1)
    0x19, 1,           //   Usage min 1
    0x29, 16,          //   Usage max 16
		0x75, 1,           //   Report Size (1)
		0x95, 16,          //   Report Count (16)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
#endif

#ifdef HID_REPORT_SEND_DBG_CTR
		0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
		0x19, 1,           //   Usage Minimum (1)
		0x29, 32,          //   Usage Maximum (32)
		0x15, 0x80,        //   Logical Minimum (-128)
		0x25, 0x7F,        //   Logical Maximum (127)
		0x75, 8,           //   Report Size (8)
		0x95, 32,          //   Report Count (32)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
#endif
#endif

  0xC0    /*     END_COLLECTION	             */
};

const uint8_t hid_report_descriptor_streaming[] = {
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)

    /* HID report IN : 64 bytes raw (64*8 bits) */
		0x15, 0x80,        //   Logical Minimum (-128)
		0x25, 0x7F,        //   Logical Maximum (127)
		0x05, 0x0A,        //   Usage Page (Ordinal)
    0x75, 8,           //   Report Size (8)
    0x95, 64,          //   Report Count (64)
    0x19, 1,           //   Usage Minimum (1)
    0x29, 64,          //   Usage Maximum (64)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		// TODO: check possibility: buffered bytes input???

    /* HID report OUT */
		0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
		0x09, 0x01,        //   Usage (0x01)
		0x75, 32,          //   Report Size (32)
		0x95, 1,           //   Report Count (1)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0x19, 1,           //   Usage Minimum (1)
		0x29, 32,          //   Usage Maximum (32)
		0x75, 8,           //   Report Size (8)
		0x95, 32,          //   Report Count (32)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

  0xC0    /*     END_COLLECTION	             */
};
