set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX "arm-none-eabi-" CACHE STRING "Cross compiler prefix")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc" CACHE STRING "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++" CACHE STRING "C++ compiler" FORCE)
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}objcopy" CACHE STRING "objcopy" FORCE)

# Skip compiler checks for cross-compilation
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Target-specific defines
set(TARGET_DEFINES "-DSTM32F4")

# Target-specific compile flags
set(TARGET_MFLAGS "-mtune=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(TARGET_BASE_FLAGS "${TARGET_MFLAGS} --static -nostartfiles -fno-common -ffunction-sections -fdata-sections -specs=nano.specs -specs=nosys.specs")
set(TARGET_CXX_FLAGS "-fno-use-cxa-atexit")

# Linking related settings
option(OPTS_CUSTOM_DFU "Use custom DFU boot image" ON)

if (OPTS_CUSTOM_DFU)
  set(TARGET_CUSTOM_BOOTLOADER_ADDR 0x8000000)
  set(TARGET_APPLICATION_ADDR 0x8004000)
else()
  set(TARGET_APPLICATION_ADDR 0x8000000)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${TARGET_MFLAGS} -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -T ${CMAKE_SOURCE_DIR}/tools/target_stm32f401.ld -T ${CMAKE_SOURCE_DIR}/submodules/libopencm3/lib/cortex-m-generic.ld")
set(TARGET_APP_LINKER_FLAGS -lm -Wl,-Ttext=${TARGET_APPLICATION_ADDR} -u _printf_float)

# Other target-specific settings
set(TARGET_SUPPORTS_DFU TRUE)
set(TARGET_LIBOPENCM3_TARGETS "stm32/f4")
set(TARGET_NAME STM32F401)
