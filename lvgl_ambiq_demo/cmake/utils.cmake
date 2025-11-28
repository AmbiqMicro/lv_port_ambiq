function(generate_firmware target_name)
    # Use demo-specific output directory
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/${target_name}")

    # Generate .bin file
    add_custom_command(
        OUTPUT "${OUTPUT_DIR}/${target_name}.bin"
        COMMAND ${CMAKE_OBJCOPY} -Obinary "$<TARGET_FILE:${target_name}>" "${OUTPUT_DIR}/${target_name}.bin"
        DEPENDS ${target_name}
        COMMENT "Generating ${target_name}.bin"
        VERBATIM
    )
    add_custom_target(${target_name}_bin ALL DEPENDS "${OUTPUT_DIR}/${target_name}.bin")

    # Generate .hex file
    add_custom_command(
        OUTPUT "${OUTPUT_DIR}/${target_name}.hex"
        COMMAND ${CMAKE_OBJCOPY} -O ihex "$<TARGET_FILE:${target_name}>" "${OUTPUT_DIR}/${target_name}.hex"
        DEPENDS ${target_name}
        COMMENT "Generating ${target_name}.hex"
        VERBATIM
    )
    add_custom_target(${target_name}_hex ALL DEPENDS "${OUTPUT_DIR}/${target_name}.hex")

    # Generate .lst file
    add_custom_command(
        OUTPUT "${OUTPUT_DIR}/${target_name}.lst"
        COMMAND ${CMAKE_OBJDUMP} -S "$<TARGET_FILE:${target_name}>" > "${OUTPUT_DIR}/${target_name}.lst"
        DEPENDS ${target_name}
        COMMENT "Generating ${target_name}.lst"
        VERBATIM
    )
    add_custom_target(${target_name}_lst ALL DEPENDS "${OUTPUT_DIR}/${target_name}.lst")

    # Generate .size file
    add_custom_command(
        OUTPUT "${OUTPUT_DIR}/${target_name}.size"
        COMMAND ${CMAKE_SIZE} "$<TARGET_FILE:${target_name}>" > "${OUTPUT_DIR}/${target_name}.size"
        DEPENDS ${target_name}
        COMMENT "Generating ${target_name}.size"
        VERBATIM
    )
    add_custom_target(${target_name}_size ALL DEPENDS "${OUTPUT_DIR}/${target_name}.size")
endfunction()

function(add_flash_target target_name board_name)
    set(JLINK "JLinkExe")
    set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/${target_name}")

    # Use template from boards directory
    set(FLASH_TEMPLATE "${CMAKE_SOURCE_DIR}/boards/${board_name}/flash.jlink.template")

    if(EXISTS "${FLASH_TEMPLATE}")
        # Read template content
        file(READ "${FLASH_TEMPLATE}" FLASH_CONTENT)

        # Replace demo name in loadbin line
        string(REGEX REPLACE
            "loadbin bin/[^ ]+\\.bin"
            "loadbin ${target_name}/${target_name}.bin"
            FLASH_CONTENT
            "${FLASH_CONTENT}"
        )

        # Write generated script to build directory
        set(GENERATED_FLASH_SCRIPT "${OUTPUT_DIR}/flash_${target_name}.jlink")
        file(WRITE "${GENERATED_FLASH_SCRIPT}" "${FLASH_CONTENT}")

        add_custom_target(flash
            COMMAND ${JLINK} -commanderscript "${GENERATED_FLASH_SCRIPT}"
            DEPENDS ${target_name}_bin
            COMMENT "Flashing ${target_name} to board ${board_name}"
            WORKING_DIRECTORY ${OUTPUT_DIR}
        )
        message(STATUS "Flash target added: use 'cmake --build . --target flash' to flash firmware")
    else()
        message(WARNING "Flash template not found: ${FLASH_TEMPLATE}")
    endif()
endfunction()
