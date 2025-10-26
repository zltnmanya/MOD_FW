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
 
#include <hal.h>
#include <libopencm3/stm32/gpio.h>

void gpio_pins_init() {
  /* GPIO IO pins */
  /* LED */
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
  /* IMU_RESET */
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);
  /* ZERO */
  gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO1);
  /* MAG_DRDY */
  gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO8);
}

int gpio_drdy_get() {
  return gpio_get(GPIOB, GPIO8) ? 1 : 0;
}
int gpio_button_get() {
  return gpio_get(GPIOA, GPIO1) ? 0 : 1;
}
int gpio_status_led_get() {
  return gpio_get(GPIOA, GPIO13) ? 0 : 1;
}
void gpio_status_led_set(int on) {
  if (on)
    gpio_clear(GPIOC, GPIO13);
  else
    gpio_set(GPIOC, GPIO13);
}
void gpio_imu_reset_set(int value) {
  if (value) 
    gpio_set(GPIOA, GPIO0);
  else
    gpio_clear(GPIOA, GPIO0);
}
