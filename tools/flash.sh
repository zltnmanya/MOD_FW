#!/bin/bash

DFU_UTIL_PATH='submodules/dfu-util/src/dfu-util'

method='dfu-util'
file='main'

usage() {
  echo "usage: $(basename "$0") <method> <build_directory> [main|dfu_boot]"
  echo " methods: dfu-util openocd_stlink openocd_raspi"
  exit 0
}
dload_dfu_util() { # 1: .dfu file
  "$DFU_UTIL_PATH" -d '0483:df11' -a 0 -D "${DIR_BUILD}/$1.dfu"
}
dload_openocd_stlink() {
  # 1: .elf file
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg -c "program ${DIR_BUILD}/$1.elf verify reset exit"
}
dload_openocd_raspi() {
  # 1: .elf file
	openocd -f interface/sysfsgpio-raspberrypi.cfg -c transport select swd -f target/stm32f4x.cfg -c "program ${DIR_BUILD}/$1.elf verify reset exit"
}

if [[ $# -lt 2 || "$1" == '-h' || "$1" == '--help' ]]; then
  usage
fi

method="$1"

DIR_BUILD="$2"

if [ $# -ge 3 ]; then
  file="$3"
fi

case "$method" in
  'dfu-util') dload_dfu_util "$file";;
  'openocd_stlink') dload_openocd_stlink "$file";;
  'openocd_raspi') dload_openocd_raspi "$file";;
  *) echo "unimplemented method: $1"; usage; exit 1;;
esac





