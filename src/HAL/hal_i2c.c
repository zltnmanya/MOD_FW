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
 
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

void i2c_bus_init(){
  /* GPIO/I2C */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6 | GPIO7);
  gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_100MHZ, GPIO6 | GPIO7); // TODO: is this speed ok?
	gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO7);

	rcc_periph_clock_enable(RCC_I2C1);

  /* I2C1 */
  rcc_periph_reset_pulse(RST_I2C1);
  i2c_peripheral_disable(I2C1);
  i2c_set_speed(I2C1, i2c_speed_fm_400k, 42);
  i2c_set_dutycycle(I2C1, I2C_CCR_DUTY_DIV2);
  i2c_peripheral_enable(I2C1);
}


