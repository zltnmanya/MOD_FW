# MOD firmware
MOD (Modular Orientation Device) is an open-source headtracking device, that uses a combination of sensors (gyroscope, accelerometer, magnetometer) to track the head movements of the user. The device communicates over USB as a HID input device (like a joystick), and sends its current orientation to the PC.
The project is built around an STM32 microcontroller.

<a href="https://www.youtube.com/watch?v=DoeuLWn88_c" target="_blank"><img alt="demo video thumbnail" src="http://github.com/zltnmanya/MOD_FW/blob/main/demo_thumbnail.jpg?raw=true" /></a>

## Modes of operation

### Tracking mode (default)
In this mode, the device continously tracks its orientation using the sensors, and sends the calculated orientation over USB. The mode is enabled by sending `T` through the virtual serial port.

Since errors from the gyro tend to accumulate over time, the orientation is automatically recentered if the device is facing roughly in the same direction as it was last centered manually.

#### As a standalone headtracking Device:
The orientation data is sent as yaw, pitch, roll triplets. These can be used as inputs to [Opentrack](https://github.com/opentrack/opentrack)s joystick input.

#### As part of DIY VR headset:
In this mode, the device sends the orientation as quaternions. This can be used to interwork with the [Relativity VR driver](https://github.com/relativty/Relativty/tree/master/Relativty_Driver).
This mode can be enabled by compiling the firmware with  OPT_HID_SEND_QUAT set to ON.

### Streaming mode
This mode can be used for obtaining raw sensor readings from the device to be used for calibration. The mode is enabled by sending `S` through the virtual serial port.

## Building
For building the firmware binary you will need:
 - WSL (on Windows) OR your favorite Linux distro
 - gcc-arm-none-eabi
 - cmake
 - make
 - python (for converting to DFU image)

run:  
  cmake --preset default
  cmake --build build/stm32f401-release/ --target all

if building libopcm3 fails with:  
> GENHDR  stm32/f4
> - Cannot openmake: *** [include/libopencm3/stm32/f4/nvic.h] Error 1  

run the following manually: `cd submodules/libopencm3/; ./scripts/irq2nvic_h ./include/libopencm3/stm32/f4/irq.json; cd ../..`

The two DFU images will be:  
  - custom DFU -- build/dfu_boot.dfu
  - main image -- build/main.dfu  

The images can be flashed using the script: `./tools/flash.sh`.

## Communication
The device acts as a composite USB device of:
 - a HID input device (used for sending the orientation data) and
 - a virtual serial port (which can be used for debugging the firmware)

## Debugging over the virtual serial port
Printouts can be sent using `log_printf(format, ...)`. As the current log handling is not well optimized, it is advised to keep printouts to a minimum.

The following commands can be sent over the virtual serial port as single character keystrokes:
 - `C` -- dump registers and stack
 - `r` -- reset orientation to default
 - `d` -- print device state
 - `t` -- dump performance info
 - `0`..`9` -- toggle debug flags 0..9
 - `S` -- enter streaming mode after reconnecting the device
 - `T` -- enter tracking mode after reconnecting the device

The following functions can be used to measure the number of cycles it takes on average for certain parts of the firmware to run.
 - `timestamp_start(uint8_t index)` -- store current cycle count for timer specified by `index`
 - `timestamp_stamp(uint8_t index, uint8_t next_start)` -- store elapsed cycles for `index` and start measurement for `next_start`
 - `timestamp(uint8_t index)` -- store elapsed cycles for `index`

The following functions can be used to get the current values of debug flags (that can be used to toggle features on and off on-the-fly by sending characters `0`..`9`):
 - `dbg_flag_get(uint8_t flag_no)` -- returns 1 if nth flag is enabled, otherwise 0
 - `dbg_flags_get` -- returns all flags as a bitmap
 - `dbg_flag_get_n_clear(uint8_t flag_no)` -- returns 1 if nth flag is enabled, otherwise 0 while clearing the flag

## License
The MOD code is released under the terms of the GNU Lesser General Public
License (LGPL), version 3 or later.

See COPYING.LESSER for details.
