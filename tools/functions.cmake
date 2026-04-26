function(create_dfu_image TARGET_NAME ADDRESS)
  if(NOT PYTHON3)
    find_program(PYTHON3 python3)
    if(NOT PYTHON3)
      message(FATAL_ERROR "Python 3 is required to build the DFU image")
    endif()
  endif()

  set(EXEC_PATH "$<TARGET_FILE:${TARGET_NAME}>")
  set(BIN_PATH "${CMAKE_BINARY_DIR}/output/${TARGET_NAME}.bin")
  set(DFU_PATH "${CMAKE_BINARY_DIR}/output/${TARGET_NAME}.dfu")

  add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXEC_PATH} ${BIN_PATH}
    COMMAND ${PYTHON3} ${CMAKE_SOURCE_DIR}/submodules/dfu-util/dfuse-pack.py -b ${ADDRESS}:${BIN_PATH} ${DFU_PATH}
    COMMENT "Creating DFU image for ${TARGET_NAME} (addr:${ADDRESS})"
    BYPRODUCTS ${BIN_PATH} ${DFU_PATH}
  )
endfunction()
