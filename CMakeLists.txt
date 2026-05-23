cmake_minimum_required(VERSION 3.22)

include(tools/build_options.cmake)
include(tools/functions.cmake)

project(MOD_FW LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_STANDARD 11)

# common gcc/linker flags for all targets
set(BASE_FLAGS_WARNINGS "-Wall -Werror")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPT_FLAGS "-O0")
  set(BASE_FLAGS_DEBUGGING "-g3 -ggdb")
else()
  set(OPT_FLAGS "-O2")
  set(BASE_FLAGS_DEBUGGING "")
endif()

set(CMAKE_C_FLAGS "${TARGET_BASE_FLAGS} ${BASE_FLAGS_WARNINGS} ${BASE_FLAGS_DEBUGGING} ${OPT_FLAGS}")
set(CMAKE_CXX_FLAGS "${TARGET_BASE_FLAGS} ${BASE_FLAGS_WARNINGS} ${BASE_FLAGS_DEBUGGING} ${OPT_FLAGS} ${TARGET_CXX_FLAGS}")

if (TARGET_SUPPORTS_DFU)
  find_program(DFU_UTIL_EXECUTABLE
    NAMES dfu-util
    DOC "Path to dfu-util tool"
  )
  if(NOT DFU_UTIL_EXECUTABLE)
    message(WARNING "dfu-util not found in PATH.")
  else()
    message(STATUS "Found dfu-util: ${DFU_UTIL_EXECUTABLE}")
  endif()
endif()

include_directories("${CMAKE_SOURCE_DIR}/include")

# subdirectories
include(tools/3pp.cmake)
add_subdirectory(src)
