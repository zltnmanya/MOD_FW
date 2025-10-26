#!/bin/bash

find_sources() {
  find "$1" -iname '*.c' \
        -or -iname '*.cc' \
        -or -iname '*.cpp' \
        -or -iname '*.h' \
        -or -iname '*.hh' \
        -or -iname '*.hpp'
}

rm cscope.files  cscope.in.out  cscope.out  cscope.po.out

{
  find_sources 'include'
  find_sources 'src'

  ls submodules/libopencm3/include/libopencm3/stm32/*.h
  find_sources 'submodules/libopencm3/include/libopencm3/stm32/common'
  find_sources 'submodules/libopencm3/include/libopencm3/stm32/f4'
  find_sources 'submodules/libopencm3/include/libopencm3/usb'

  ls submodules/libopencm3/lib/stm32/*.*
  find_sources 'submodules/libopencm3/lib/stm32/common'
  find_sources 'submodules/libopencm3/lib/stm32/f4'

  find_sources 'submodules/libopencm3/lib/usb'

  ls submodules/libopencm3/include/libopencmsis/*.h
  find_sources 'submodules/libopencm3/include/libopencmsis/stm32/f4'
} > cscope.files

cscope -bq
