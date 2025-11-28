#******************************************************************************
# CMakeLists.txt - Top-level CMake configuration for all demos
# Build directory: lvgl_ambiq_demo/build/
# cmake .. -G "Unix Makefiles"
#******************************************************************************
cmake_minimum_required(VERSION 3.20)

# Set TOP_DIR to lv_port_ambiq root directory (before project() for toolchain setup)
get_filename_component(TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(TOP_DIR ${TOP_DIR} CACHE INTERNAL "lv_port_ambiq root directory" FORCE)
message(STATUS "TOP_DIR set to: ${TOP_DIR}")

# Set paths based on TOP_DIR
get_filename_component(AMBIQSUITE_PATH "${TOP_DIR}/../AmbiqSuite" ABSOLUTE)
get_filename_component(LVGL_PATH "${TOP_DIR}/LVGL" ABSOLUTE)

# Cache paths for reuse
set(AMBIQSUITE_PATH ${AMBIQSUITE_PATH} CACHE PATH "AmbiqSuite path" FORCE)
set(LVGL_PATH ${LVGL_PATH} CACHE PATH "LVGL path" FORCE)

# Check required paths
if(NOT EXISTS ${AMBIQSUITE_PATH})
    message(FATAL_ERROR "AmbiqSuite directory not found: ${AMBIQSUITE_PATH}")
endif()
if(NOT EXISTS ${LVGL_PATH})
    message(FATAL_ERROR "LVGL directory not found: ${LVGL_PATH}")
endif()

# Include toolchain BEFORE project() for cross-compilation
include(${CMAKE_CURRENT_LIST_DIR}/cmake/tool_chain.cmake)

# Now call project() after toolchain is configured
project(lvgl_ambiq_demo LANGUAGES C CXX ASM)

# Include remaining CMake utilities
include(${CMAKE_CURRENT_LIST_DIR}/cmake/bsp_config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/lvgl_version.cmake)
# Note: lvgl_library.cmake will be included after DEMO_NAME is determined
# This allows it to find lv_conf.h in the demo's src directory

# Set build output directory structure
# All demos will build to: build/<demo_name>/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


# --- ThinkSi NemaGFX GPU Stack ----------------------------------------------
set(AMBIQ_GPU_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/NemaDC
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/NemaGFX
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/NemaGFX/Nema
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/common/mem
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaDC
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaGFX
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaVG
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/NemaGFX_SDK/include/tsi/common
)

# --- FatFs File System -------------------------------------------------------
set(FATFS_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/third_party/FatFs/apollo_driver/mmc_apollo5
    ${AMBIQSUITE_PATH}/third_party/FatFs/source
)
set(FATFS_SOURCES
    ${AMBIQSUITE_PATH}/third_party/FatFs/apollo_driver/mmc_apollo5/mmc_apollo5.c
    ${AMBIQSUITE_PATH}/third_party/FatFs/source/ff.c
    ${AMBIQSUITE_PATH}/third_party/FatFs/source/ffsystem.c
    ${AMBIQSUITE_PATH}/third_party/FatFs/source/ffunicode.c
    ${AMBIQSUITE_PATH}/third_party/FatFs/source/diskio.c
)

# --- NemaGFX HAL -------------------------------------------------------------
set(NEMAGFX_HAL_INCLUDE_DIRS
    ${TOP_DIR}/lvgl_ambiq_porting/NemaGFX_hal/apollo510_nemagfx
)
set(NEMAGFX_HAL_SOURCES
    ${TOP_DIR}/lvgl_ambiq_porting/NemaGFX_hal/apollo510_nemagfx/nema_dc_hal.c
    ${TOP_DIR}/lvgl_ambiq_porting/NemaGFX_hal/apollo510_nemagfx/nema_hal.c
)

# --- FreeRTOS ----------------------------------------------------------------
set(FREERTOS_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/include
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5
)
set(FREERTOS_SOURCES
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/event_groups.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/queue.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/tasks.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/timers.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/list.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/portable/MemMang/heap_4.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5/port.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5/port_stimer.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5/port_systick_stimer.c
    ${AMBIQSUITE_PATH}/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5/portasm.c
)

# --- TLSF Memory Allocator ---------------------------------------------------
set(TLSF_INCLUDE_DIRS
    ${TOP_DIR}/modules/tlsf
)
set(TLSF_SOURCES
    ${TOP_DIR}/modules/tlsf/tlsf.c
)

# --- LVGL Ambiq Porting Layer ------------------------------------------------
set(LVGL_AMBIQ_PORTING_INCLUDE_DIRS
    ${TOP_DIR}/lvgl_ambiq_porting
)
set(LVGL_AMBIQ_PORTING_SOURCES
    ${TOP_DIR}/lvgl_ambiq_porting/am_common.c
    ${TOP_DIR}/lvgl_ambiq_porting/am_mem.c
    ${TOP_DIR}/lvgl_ambiq_porting/lv_ambiq_touch.c
    ${TOP_DIR}/lvgl_ambiq_porting/lv_ambiq_fs.c
    ${TOP_DIR}/lvgl_ambiq_porting/lv_ambiq_display.c
)
set(LVGL_AMBIQ_PORTING_LIBS
    ${TOP_DIR}/lvgl_ambiq_porting/gpu_patch/bin/lvgl_ambiq_porting.a
)

# --- CMSIS -------------------------------------------------------------------
set(CMSIS_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/CMSIS/ARM/Include
    ${AMBIQSUITE_PATH}/CMSIS/AmbiqMicro/Include
)
set(CMSIS_LIBS
    ${AMBIQSUITE_PATH}/CMSIS/ARM/Lib/ARM/DSP_LIB_CM55/libarm_cortexM55f_math.a
)

# --- AmbiqSuite HAL & Utilities ----------------------------------------------
set(AMBIQ_DEVICE_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/devices
)
set(AMBIQ_UTILITY_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/utils
)
set(AMBIQ_UTILITY_SOURCES
    ${AMBIQSUITE_PATH}/utils/am_util_delay.c
    ${AMBIQSUITE_PATH}/utils/am_util_stdio.c
)
set(AMBIQ_HAL_INCLUDE_DIRS
    ${AMBIQSUITE_PATH}/mcu/apollo510
    ${AMBIQSUITE_PATH}/mcu/apollo510/hal
)
set(AMBIQ_HAL_LIBS
    ${AMBIQSUITE_PATH}/mcu/apollo510/hal/mcu/gcc/bin/libam_hal.a
    ${AMBIQSUITE_PATH}/third_party/ThinkSi/config/apollo510_nemagfx/gcc/bin/lib_nema_apollo510_nemagfx.a
)

# --- Common Sources (excluding LVGL, which is now a library) ---
set(COMMON_SOURCES
    ${FATFS_SOURCES}
    ${NEMAGFX_HAL_SOURCES}
    # LVGL is now linked as library, not included here
    ${FREERTOS_SOURCES}
    ${TLSF_SOURCES}
    ${LVGL_AMBIQ_PORTING_SOURCES}
    ${AMBIQ_UTILITY_SOURCES}
)

set(COMMON_INCLUDES
    ${AMBIQ_GPU_INCLUDE_DIRS}
    ${FATFS_INCLUDE_DIRS}
    ${NEMAGFX_HAL_INCLUDE_DIRS}
    ${FREERTOS_INCLUDE_DIRS}
    ${TLSF_INCLUDE_DIRS}
    ${LVGL_AMBIQ_PORTING_INCLUDE_DIRS}
    ${AMBIQ_DEVICE_INCLUDE_DIRS}
    ${AMBIQ_UTILITY_INCLUDE_DIRS}
    ${AMBIQ_HAL_INCLUDE_DIRS}
    ${CMSIS_INCLUDE_DIRS}
)

set(COMMON_LIBS
    ${LVGL_AMBIQ_PORTING_LIBS}
    ${CMSIS_LIBS}
    ${AMBIQ_HAL_LIBS}
)

# --- Common Compile Definitions (corresponds to DEFINES in common.mk) ---------
set(COMMON_DEFINES
    LV_CONF_INCLUDE_SIMPLE
    LV_LVGL_H_INCLUDE_SIMPLE
    NEMA_PLATFORM=apollo510_nemagfx
    VMEM_SIZE=0x3FFFF
    WAIT_IRQ_BINARY_SEMAPHORE=1
    gcc
)

# --- Common Compile / Link Options -------------------------------------------
set(COMMON_C_FLAGS
    -ffunction-sections
    -fdata-sections
    -fomit-frame-pointer
    -fno-common
    -fno-exceptions
    -fstack-protector
    -MMD
    -MP
    -std=c99
    -Wall
    -g
    -Wimplicit-fallthrough
    -Wundef
    -Wpointer-arith
    -Wshadow
    -Wredundant-decls
    -Wstrict-prototypes
    -Wno-sign-compare
    -Wno-unknown-pragmas
    -Wno-psabi
    -O3
)

set(COMMON_CXX_FLAGS
    -ffunction-sections
    -fdata-sections
    -fomit-frame-pointer
    -fno-common
    -fno-exceptions
    -fstack-protector
    -fno-use-cxa-atexit
    -MMD
    -MP
    -std=c++11
    -Wall
    -g
    -Wimplicit-fallthrough
    -Wundef
    -Wpointer-arith
    -Wshadow
    -Wredundant-decls
    -Wno-sign-compare
    -Wno-unknown-pragmas
    -Wno-psabi
    -O3
)

# Common linker flags
set(COMMON_LINKER_FLAGS
    -nostartfiles
    -static
    -Wl,--gc-sections
)

# System libraries
set(COMMON_SYSTEM_LIBS
    m           # math library
    c           # C library
    gcc         # GCC library
    nosys       # No operating system library
    stdc++      # C++ library
)

# --- Add demo subdirectories ---
# Select which demo to build via -DDEMO_NAME=<name>
if(NOT DEFINED DEMO_NAME OR DEMO_NAME STREQUAL "")
    set(DEMO_NAME "lvgl_test" CACHE STRING "Demo name to build")
endif()

message(STATUS "Building demo: ${DEMO_NAME}")

# Configure BSP before including lvgl_library.cmake
# This allows lvgl_library.cmake to use BSP_CPU_FLAGS directly
# BOARD can be set via -DBOARD=<name> or will use default value
if(NOT DEFINED BOARD OR BOARD STREQUAL "")
    set(BOARD "apollo510_evb" CACHE STRING "Target board")
endif()

configure_bsp(${BOARD})

# Set demo src directory (fixed path: {DEMO_NAME}/src/)
# This is where lv_conf.h should be located
set(DEMO_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/${DEMO_NAME}/src")

# Include lvgl_library.cmake after DEMO_SRC_DIR and BSP are configured
# lvgl_library.cmake will use DEMO_SRC_DIR to find lv_conf.h and BSP_CPU_FLAGS for compile options
include(${CMAKE_CURRENT_LIST_DIR}/cmake/lvgl_library.cmake)

# Add the selected demo as subdirectory
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${DEMO_NAME}/CMakeLists.txt")
    add_subdirectory(${DEMO_NAME})
else()
    message(FATAL_ERROR "Demo '${DEMO_NAME}' not found or has no CMakeLists.txt")
endif()
