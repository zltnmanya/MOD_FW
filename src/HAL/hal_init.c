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
 
#include <stdlib.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/usb/cdc.h>

#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/usb/dwc/otg_fs.h>

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/dwt.h>

#include <hal.h>

static void init_debug() {
  SCS_DEMCR |= SCS_DEMCR_TRCENA;
	DWT_CYCCNT = 0;
	DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}

void hal_init() {
	rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_OTGFS);

  init_debug(); /* CYCCNT */

  gpio_pins_init();

  i2c_bus_init();

	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(10499); /* 84MHz/8/10500 -> 1000Hz -> 1ms */
	systick_interrupt_enable();
	systick_counter_enable();
}

void hal_init_usb() {
  /* USB */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
	gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);
  OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS;
  nvic_enable_irq(NVIC_OTG_FS_IRQ);
}
