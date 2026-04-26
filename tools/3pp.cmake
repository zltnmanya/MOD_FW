if (TARGET_LIBOPENCM3_TARGETS)
  set(LIBOPENCM3_ARCHIVE "${CMAKE_SOURCE_DIR}/submodules/libopencm3/lib/libopencm3_stm32f4.a")

  add_custom_command(
    OUTPUT ${LIBOPENCM3_ARCHIVE}
    COMMAND ${CMAKE_MAKE_PROGRAM} TARGETS=${TARGET_LIBOPENCM3_TARGETS} PREFIX=${TOOLCHAIN_PREFIX}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/submodules/libopencm3
    COMMENT "Building libopencm3 stm32f4 archive"
    VERBATIM
  )

  add_library(libopencm3 STATIC IMPORTED)
  set_target_properties(libopencm3 PROPERTIES IMPORTED_LOCATION ${LIBOPENCM3_ARCHIVE})
  target_include_directories(libopencm3 INTERFACE "${CMAKE_SOURCE_DIR}/submodules/libopencm3/include" )
endif()

# I2cdevlib library for STM32
add_library(i2cdevlib_stm32 OBJECT
  "submodules/i2cdevlib/STM32_LibOpenCM3/I2Cdev.c"
  "submodules/i2cdevlib/STM32/MPU6050/MPU6050.c"
  "submodules/i2cdevlib/STM32/QMC5883L/QMC5883L.c"
)
target_compile_definitions(i2cdevlib_stm32 PUBLIC ${TARGET_DEFINES})

target_include_directories(i2cdevlib_stm32 PUBLIC
  "submodules/i2cdevlib/STM32/MPU6050"
  "submodules/i2cdevlib/STM32/QMC5883L"
  "submodules/i2cdevlib/STM32_LibOpenCM3"
)

target_link_libraries(i2cdevlib_stm32 PUBLIC libopencm3)

# Quaternions header-only library
add_library(lib_quaternions INTERFACE)
target_include_directories(lib_quaternions INTERFACE
  "submodules/quaternions/include"
)