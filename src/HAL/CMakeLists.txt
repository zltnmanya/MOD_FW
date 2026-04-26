# HAL sources built in object library

add_library(hal OBJECT 
  hal_gpio.c
  hal_i2c.c
  hal_misc.c
  hal_flash.c
  hal_init.c
  hal_usb.c
  hal_irq.c
)

target_include_directories(hal
  PUBLIC
  "${CMAKE_SOURCE_DIR}/include/HAL"
)

target_link_libraries(hal PUBLIC libopencm3)

target_compile_definitions(hal PUBLIC ${TARGET_DEFINES})
