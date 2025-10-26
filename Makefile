##
## This file is part of the MOD project.
##
## Copyright (C) 2025 Zoltán Mánya <zltnmanya@gmail.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##

OPTS_CUSTOM_DFU=1
OPTS_OZERO=1
REBUILD_LIBS=0
VERBOSE=0

PREFIX?=arm-none-eabi-
CC=$(PREFIX)gcc
CPP=$(PREFIX)g++
OBJCOPY=$(PREFIX)objcopy
DIR_BUILD=build
DFUSE_PATH=submodules/dfu-util/dfuse-pack.py

ifeq ($(VERBOSE),1)
V=
else
V=@
endif

all:

# Flags 
# ==============================================================================
BUILD_TARGETS=$(DIR_BUILD)/main.dfu
APP_START_ADDRESS=0x8000000

ifeq ($(OPTS_CUSTOM_DFU),1)
DFU_START_ADDRESS=0x8000000
APP_START_ADDRESS=0x8004000
BUILD_TARGETS+= $(DIR_BUILD)/dfu_boot.dfu
endif

DEFINES=
DEFINES+= -DSTM32F4
# DEFINES+= -DHID_REPORT_SEND_INDICATORS
# DEFINES+= -DHID_REPORT_SEND_DBG_CTR
# DEFINES+= -DHID_REPORT_SEND_QUAT

INCLUDES=
INCLUDES+= -I./submodules/libopencm3/include
INCLUDES+= -I./include
INCLUDES+= -I./include/3pp
INCLUDES+= -I./include/HAL
INCLUDES+= -I./submodules/i2cdevlib/STM32_LibOpenCM3
INCLUDES+= -I./submodules/i2cdevlib/STM32/MPU6050
INCLUDES+= -I./submodules/i2cdevlib/STM32/QMC5883L
INCLUDES+= -I./submodules/quaternions/include

LIBS+= -L./submodules/libopencm3/lib

MFLAGS= -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

CFLAGS=$(DEFINES) $(INCLUDES) $(MFLAGS)
# CFLAGS+= -Wall -Werror
CFLAGS+= --static -nostartfiles -g3
CFLAGS+= -ggdb
ifeq ($(OPTS_OZERO),1)
CFLAGS+= -O0
else
CFLAGS+= -O2
endif
CFLAGS+= -fno-common -ffunction-sections -fdata-sections -specs=nano.specs -specs=nosys.specs
CFLAGS+= -Wall -Werror
# CFLAGS+= -funwind-tables

CPPFLAGS:=$(CFLAGS)

CPPFLAGS+= -fno-use-cxa-atexit
CFLAGS+= -std=c11

LDFLAGS_COMMON=$(LIBS) -Wl,--start-group -lopencm3_stm32f4 -lc -lgcc -lnosys -Wl,--end-group -T stm32f401.ld $(MFLAGS)
LDFLAGS_DFU=$(LDFLAGS_COMMON)
LDFLAGS_APP=$(LDFLAGS_COMMON) -lm -Wl,-Ttext=$(APP_START_ADDRESS) -u _printf_float

# build
# ==============================================================================
DFUSE_PACK=python3 $(DFUSE_PATH)

OBJS=
OBJS+= $(DIR_BUILD)/main.o

OBJS+= $(DIR_BUILD)/10_sensor_input.o
OBJS+= $(DIR_BUILD)/20_sensor_fusion.o
OBJS+= $(DIR_BUILD)/20b_sensor_calib.o
OBJS+= $(DIR_BUILD)/30_post_process.o
OBJS+= $(DIR_BUILD)/common.o
OBJS+= $(DIR_BUILD)/msg_queue.o
OBJS+= $(DIR_BUILD)/persistence.o
OBJS+= $(DIR_BUILD)/usb_hid_report_in.o
OBJS+= $(DIR_BUILD)/usb_hid_report_out.o
OBJS+= $(DIR_BUILD)/util_math.o
OBJS+= $(DIR_BUILD)/logger.o
OBJS+= $(DIR_BUILD)/diag.o

OBJS+= $(DIR_BUILD)/3pp/I2Cdev.o
OBJS+= $(DIR_BUILD)/3pp/MPU6050.o
OBJS+= $(DIR_BUILD)/3pp/QMC5883L.o
OBJS+= $(DIR_BUILD)/3pp/usb_desc.o

OBJS+= $(DIR_BUILD)/HAL/hal_gpio.o
OBJS+= $(DIR_BUILD)/HAL/hal_i2c.o
OBJS+= $(DIR_BUILD)/HAL/hal_misc.o
OBJS+= $(DIR_BUILD)/HAL/hal_flash.o
OBJS+= $(DIR_BUILD)/HAL/hal_init.o
OBJS+= $(DIR_BUILD)/HAL/hal_usb.o
OBJS+= $(DIR_BUILD)/HAL/hal_irq.o

# ==============================================================================
# object files

$(DIR_BUILD)/%.o: src/%.c include/params.h | submodules/libopencm3/lib/libopencm3_stm32f4.a outdir
	@echo "CC $<"
	$(V)$(CC) $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/%.o: src/%.cpp include/params.h | submodules/libopencm3/lib/libopencm3_stm32f4.a outdir
	@echo "CC $<"
	$(V)$(CPP) $(CPPFLAGS) -c $< -o $@

$(DIR_BUILD)/3pp/%.o: submodules/i2cdevlib/STM32/MPU6050/%.c | outdir
	@echo "CC $<"
	$(V)$(CC) $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/3pp/%.o: submodules/i2cdevlib/STM32/QMC5883L/%.c | outdir
	@echo "CC $<"
	$(V)$(CC) $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/3pp/%.o: submodules/i2cdevlib/STM32_LibOpenCM3/%.c | outdir
	@echo "CC $<"
	$(V)$(CC) $(CFLAGS) -c $< -o $@

# ==============================================================================
# linking, image generation, etc.

$(DIR_BUILD)/dfu_boot.elf: $(DIR_BUILD)/3pp/usbdfu.o
	@echo "linking: $@"
	$(V)$(CC) $(CFLAGS) $^ $(LDFLAGS_DFU) -o $@

$(DIR_BUILD)/main.elf: $(OBJS) submodules/libopencm3/lib/libopencm3_stm32f4.a
	@echo "linking: $@"
	$(V)$(CPP) $^ $(CPPFLAGS) $(LDFLAGS_APP) -o $@

$(DIR_BUILD)/%.bin: $(DIR_BUILD)/%.elf
	@echo "converting to raw binary: $@"
	$(V)$(OBJCOPY) -O binary $^ $@

$(DIR_BUILD)/main.dfu: $(DIR_BUILD)/main.bin
	@echo "creating DFU image: $@"
	$(V)$(DFUSE_PACK) -b $(APP_START_ADDRESS):$^ $@

$(DIR_BUILD)/dfu_boot.dfu: $(DIR_BUILD)/dfu_boot.bin
	@echo "creating DFU image: $@"
	$(V)$(DFUSE_PACK) -b $(DFU_START_ADDRESS):$^ $@

$(DIR_BUILD)/test_1: _off/test_1.cpp | outdir
	g++ $(INCLUDES) -Wall -Werror $^ -o $@

# ==============================================================================
# directories, flashing, etc.

submodules/libopencm3/lib/libopencm3_stm32f4.a:
	$(V)$(MAKE) -C submodules/libopencm3 TARGETS=stm32/f4 PREFIX=$(PREFIX)

outdir:
	$(V)mkdir -p $(DIR_BUILD)
	$(V)mkdir -p $(DIR_BUILD)/3pp
	$(V)mkdir -p $(DIR_BUILD)/HAL

all: $(BUILD_TARGETS)

clean:
	$(RM) -r $(DIR_BUILD)

clean_swp:
	$(RM) `find -iname '.*.swp'`

.PHONY: outdir clean all

ifeq ($(REBUILD_LIBS),1)
.PHONY: submodules/libopencm3/lib/libopencm3_stm32f4.a
endif
