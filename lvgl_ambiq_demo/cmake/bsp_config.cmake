#******************************************************************************
# BspConfig.cmake - BSP configuration loader
# Each board's configuration is in boards/{board_name}/bsp_config.cmake
#******************************************************************************

# Supported board list
set(SUPPORTED_BOARDS apollo510_evb apollo510b_evb apollo5b_eb_revb CACHE STRING "Supported board list")

# Main configuration function: load board-specific configuration
function(configure_bsp BOARD)
    # Check if board is supported
    if(NOT BOARD IN_LIST SUPPORTED_BOARDS)
        message(FATAL_ERROR "Unsupported board: ${BOARD}. Supported: ${SUPPORTED_BOARDS}")
    endif()

    # Board-specific configuration file path
    # TOP_DIR points to lv_port_ambiq root directory, boards are in lvgl_ambiq_demo/boards/
    set(BOARD_CONFIG_FILE "${TOP_DIR}/lvgl_ambiq_demo/boards/${BOARD}/bsp_config.cmake")

    if(NOT EXISTS "${BOARD_CONFIG_FILE}")
        message(FATAL_ERROR "Board configuration file not found: ${BOARD_CONFIG_FILE}")
    endif()

    # Include board-specific configuration
    # This will set BSP_CPU_FLAGS, BSP_DEFINES, BSP_INCLUDES, etc. in the current scope
    include("${BOARD_CONFIG_FILE}")

    # Propagate variables to caller scope
    set(BSP_CPU_FLAGS ${BSP_CPU_FLAGS} PARENT_SCOPE)
    set(BSP_LINKER_ENTRY ${BSP_LINKER_ENTRY} PARENT_SCOPE)
    set(BSP_DEFINES ${BSP_DEFINES} PARENT_SCOPE)
    set(BSP_INCLUDES ${BSP_INCLUDES} PARENT_SCOPE)
    set(BSP_SOURCES ${BSP_SOURCES} PARENT_SCOPE)
    set(DEVICE_SOURCES ${DEVICE_SOURCES} PARENT_SCOPE)
    set(LINKER_SCRIPT ${LINKER_SCRIPT} PARENT_SCOPE)
    set(STARTUP_FILE ${STARTUP_FILE} PARENT_SCOPE)

    message(STATUS "Configured BSP: ${BOARD}")
endfunction()

# Confirm file is loaded
message(STATUS "bsp_config.cmake loaded, configure_bsp function defined")
