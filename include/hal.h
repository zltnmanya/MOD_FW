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
 
#ifndef HAL_H_
#define HAL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int strobe_clear;
extern int strobe_enable;
extern int cpu_hang_indication;

void hal_init();
void hal_init_usb();

void usb_irq_disable();
void usb_irq_enable();

void gpio_pins_init();
int gpio_drdy_get();
int gpio_button_get(); // OK
int gpio_status_led_get();
void gpio_status_led_set(int on); // OK
void gpio_imu_reset_set(int value);

void i2c_bus_init(); // OK

void delay_ms(uint32_t d); // OK
uint32_t get_common_tick(); // OK probably

#ifdef __cplusplus
}
#endif

#endif
