#******************************************************************************
# apollo510b_evb BSP configuration
#******************************************************************************

# CPU-specific compile options
set(BSP_CPU_FLAGS "-mthumb" "-mcpu=cortex-m55" "-mfpu=auto" "-mfloat-abi=hard")
set(BSP_LINKER_ENTRY "-Wl,--entry=Reset_Handler")

# BSP defines
set(BSP_DEFINES
    "AM_PART_APOLLO510"
    "apollo510b_evb"
)

# BSP includes
set(BSP_INCLUDES
    "${AMBIQSUITE_PATH}/boards/apollo510b_evb/bsp"
    "${AMBIQSUITE_PATH}/third_party/cordio/wsf/sources/port/freertos"
    "${AMBIQSUITE_PATH}/third_party/cordio/wsf/sources/util"
)

# BSP sources
set(BSP_SOURCES
    "${AMBIQSUITE_PATH}/boards/apollo510b_evb/bsp/am_bsp.c"
    "${AMBIQSUITE_PATH}/boards/apollo510b_evb/bsp/am_bsp_pins.c"
    "${AMBIQSUITE_PATH}/CMSIS/AmbiqMicro/Source/system_apollo510.c"
)

# Device sources
set(DEVICE_SOURCES
    "${AMBIQSUITE_PATH}/devices/am_devices_mspi_psram_aps25616ba_1p2v.c"
    "${AMBIQSUITE_PATH}/devices/am_devices_display_generic.c"
    "${AMBIQSUITE_PATH}/devices/am_devices_chsc5816_ap5.c"
    "${AMBIQSUITE_PATH}/devices/am_devices_em9305.c"
)

# Linker script and startup file (relative to boards directory)
set(BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(LINKER_SCRIPT "${BOARD_DIR}/linker_script.ld")
set(STARTUP_FILE "${BOARD_DIR}/startup_gcc.c")

