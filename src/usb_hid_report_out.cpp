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
 
#include <usb_hid_report_out.h>
#include <persistence.h>
#include <common.h>
#include <msg_queue.h>
#include <hal.h>

#define PKT_TYPE_FLASH	  		  1
#define PKT_TYPE_RESET_FIFO       2
#define PKT_TYPE_REPORTS_SELECT   3
#define PKT_TYPE_I2C_MODIFY_8     4
#define PKT_TYPE_I2C_MODIFY_16    5

#define PKT_TYPE_GYRO_HW_OFFS_SET 10
#define PKT_TYPE_GYRO_SW_OFFS_SET 11
#define PKT_TYPE_MAG_GAIN_SET	  12
#define PKT_TYPE_MAG_OFFS_SET	  13
#define PKT_TYPE_ACCEL_GAIN_SET   14
#define PKT_TYPE_ACCEL_OFFS_SET   15

static uint8_t* unpack_i16(uint8_t *buffer, int16_t *value) {
	*value = buffer[0] | (buffer[1] << 8); // TODO: ?etohs() instead
	// *value = be16toh(*(uint16_t*)(buffer)) // TODO: test this
	return buffer + 2;
}
static uint8_t* unpack_u32(uint8_t *buffer, uint32_t *value) {
	*value = buffer[0] | (((uint32_t)buffer[1]) << 8)  | (((uint32_t)buffer[2]) << 16) | (((uint32_t)buffer[3]) << 24); // TODO: ?etohs() instead
	// *value = be16toh(*(uint16_t*)(buffer)) // TODO: test this
	return buffer + 2;
}
static uint8_t* unpack_double(uint8_t *buffer, double *value) {
	*value = *(double*)(buffer); // TODO: is this safe? maybe unaligned access???
	return buffer + sizeof(double);
}

/////////////////// IRQ - non-IRQ transfer ///////////////////////

static uint32_t irq_flags = 0;

uint32_t fetch_n_clear_irq_flags(uint32_t flags) {
	usb_irq_disable();
	uint32_t rv = irq_flags & flags;
	irq_flags &= ~flags;
	usb_irq_enable();
	return rv;
}

/////////////////////////////////////////////////////////////////////////////
/* !!! WARNING !!! IRQ CONTEXT BELOW !!! WARNING !!! IRQ CONTEXT BELOW !!! */
/* ======================================================================= */
extern "C" void post_irq_flag(uint32_t flag);
void post_irq_flag(uint32_t flag) {
	irq_flags |= flag;
}

static void process_gyro_calib_hw_offs(uint8_t *buffer) {
	buffer = unpack_i16(buffer, &calib.gyro_offset_hw.x);
	buffer = unpack_i16(buffer, &calib.gyro_offset_hw.y);
	buffer = unpack_i16(buffer, &calib.gyro_offset_hw.z);
	post_irq_flag(1);
}

static void process_gyro_calib_sw_offs(uint8_t *buffer) {
	buffer = unpack_double(buffer, &calib.gyro_offset_fine.x);
	buffer = unpack_double(buffer, &calib.gyro_offset_fine.y);
	buffer = unpack_double(buffer, &calib.gyro_offset_fine.z);
}

static void process_mag_calib_gain(uint8_t *buffer) {
	buffer = unpack_double(buffer, &calib.mag_gain.x);
	buffer = unpack_double(buffer, &calib.mag_gain.y);
	buffer = unpack_double(buffer, &calib.mag_gain.z);
}
static void process_mag_calib_offset(uint8_t *buffer) {
	buffer = unpack_double(buffer, &calib.mag_offs.x);
	buffer = unpack_double(buffer, &calib.mag_offs.y);
	buffer = unpack_double(buffer, &calib.mag_offs.z);
}

static void process_accel_calib_gain(uint8_t *buffer) {
	buffer = unpack_double(buffer, &calib.accel_multiplier);
}
static void process_accel_calib_offs(uint8_t *buffer) {
	buffer = unpack_double(buffer, &calib.accel_offset.x);
	buffer = unpack_double(buffer, &calib.accel_offset.y);
	buffer = unpack_double(buffer, &calib.accel_offset.z);
}

static void process_reports_select(uint8_t *buffer) {
	buffer = unpack_u32(buffer, &reports_selected);
}

static void process_report_i2c_w8(uint8_t *buffer) {
	union msg_u *msg = msgq_request_buffer(&queue_main);
	msg->i2c_mod8.dev = buffer[0];
	msg->i2c_mod8.addr= buffer[1];
	msg->i2c_mod8.data= buffer[2];
	msg->i2c_mod8.mask= buffer[3];
	msgq_post(msg, MSGQ_TYPE_I2C_MOD8);
}
static void process_report_i2c_w16(uint8_t *buffer) {
	union msg_u *msg = msgq_request_buffer(&queue_main);
	msg->i2c_mod16.dev = buffer[0];
	msg->i2c_mod16.addr= buffer[1];
	buffer += 2;
	buffer = unpack_i16(buffer, (int16_t*)&msg->i2c_mod16.data);
	buffer = unpack_i16(buffer, (int16_t*)&msg->i2c_mod16.mask);
	msgq_post(msg, MSGQ_TYPE_I2C_MOD16);
}

void hid_out_process(uint8_t *buffer, uint8_t len) {
	uint32_t packet_type = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
	// uint32_t packet_type = be32toh(*(uint32_t*)buffer); // TODO: test this
	buffer += 4;
	len -= 4;

	switch (packet_type) {
		case PKT_TYPE_GYRO_HW_OFFS_SET:
			process_gyro_calib_hw_offs(buffer);
			break;
		case PKT_TYPE_GYRO_SW_OFFS_SET:
			process_gyro_calib_sw_offs(buffer);
			break;
		case PKT_TYPE_MAG_GAIN_SET:
			process_mag_calib_gain(buffer);
			break;
		case PKT_TYPE_MAG_OFFS_SET:
			process_mag_calib_offset(buffer);
			break;
		case PKT_TYPE_ACCEL_GAIN_SET:
			process_accel_calib_gain(buffer);
			break;
		case PKT_TYPE_ACCEL_OFFS_SET:
			process_accel_calib_offs(buffer);
			break;
		case PKT_TYPE_FLASH:
			post_irq_flag(2);
			break;
		case PKT_TYPE_RESET_FIFO:
			post_irq_flag(4);
			break;
		case PKT_TYPE_REPORTS_SELECT:
			process_reports_select(buffer);
			break;
		case PKT_TYPE_I2C_MODIFY_8:
			process_report_i2c_w8(buffer);
			break;
		case PKT_TYPE_I2C_MODIFY_16:
			process_report_i2c_w16(buffer);
			break;
		default:
			break;
	}

}

/* ======================================================================= */
/* !!! WARNING !!! IRQ CONTEXT ABOVE !!! WARNING !!! IRQ CONTEXT ABOVE !!! */
/////////////////////////////////////////////////////////////////////////////
