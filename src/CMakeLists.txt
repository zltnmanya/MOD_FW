# Main source directory
add_subdirectory(HAL)

add_executable(main 
  main.c
  10_sensor_input.cpp
  20_sensor_fusion.cpp
  20b_sensor_calib.cpp
  30_post_process.cpp
  common.cpp
  msg_queue.c
  persistence.cpp
  usb_hid_report_in.cpp
  usb_hid_report_out.cpp
  util_math.cpp
  logger.c
  diag.c
  3pp/usb_desc.c
)

# Link object libraries and opencm3
target_link_libraries(main PRIVATE
  hal
  libopencm3
  i2cdevlib_stm32
  lib_quaternions
)

set_target_properties(main PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output"
)

add_opt_define(main OPT_HID_SEND_QUAT FALSE)
add_opt_define(main OPT_HID_SEND_INDICATORS FALSE)
add_opt_define(main OPT_HID_SEND_DEBUG_COUNTERS FALSE)
add_opt_define(main OUTPUT_INVERT_YAW TRUE)
add_opt_define(main OUTPUT_INVERT_PITCH TRUE)
add_opt_define(main OUTPUT_INVERT_ROLL TRUE)

# Pass calibration defaults to compiler
target_compile_definitions(main
  PRIVATE
  DEFAULT_CALIB__GYRO_HW=${DEFAULT_CALIB__GYRO_HW}
  DEFAULT_CALIB__GYRO_FINE=${DEFAULT_CALIB__GYRO_FINE}
  DEFAULT_CALIB__ACCEL_OFFSET=${DEFAULT_CALIB__ACCEL_OFFSET}
  DEFAULT_CALIB__ACCEL_MULTIPLIER=${DEFAULT_CALIB__ACCEL_MULTIPLIER}
  DEFAULT_CALIB__MAG_GAIN=${DEFAULT_CALIB__MAG_GAIN}
  DEFAULT_CALIB__MAG_OFFS=${DEFAULT_CALIB__MAG_OFFS}
  DEFAULT_SLACK=${DEFAULT_SLACK}
)

target_link_options(main PUBLIC ${TARGET_APP_LINKER_FLAGS})

if (TARGET_SUPPORTS_DFU AND OPTS_CUSTOM_DFU)
    # DFU Bootloader target
    add_executable(dfu_boot "${CMAKE_SOURCE_DIR}/src/3pp/usbdfu.c")

    target_link_libraries(dfu_boot PRIVATE
      libopencm3
    )

    set_target_properties(dfu_boot PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output"
    )
    target_compile_definitions(dfu_boot PUBLIC ${TARGET_DEFINES})

    create_dfu_image(dfu_boot ${TARGET_CUSTOM_BOOTLOADER_ADDR})
endif()

create_dfu_image(main ${TARGET_APPLICATION_ADDR})

target_compile_definitions(main PUBLIC ${TARGET_DEFINES})