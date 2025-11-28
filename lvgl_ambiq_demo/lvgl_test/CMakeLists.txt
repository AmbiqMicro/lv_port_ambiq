#******************************************************************************
# CMakeLists.txt - Demo subproject configuration
# This file is included via add_subdirectory() from top-level CMakeLists.txt
#******************************************************************************

# Set demo name
set(DEMO_NAME lvgl_test)

# Demo sources
set(DEMO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rtos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/am_resources.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui_task.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lvgl_test.c
)

# LVGL demo sources
file(GLOB LVGL_DEMO_SOURCES
    ${LVGL_PATH}/demos/scroll/*.c
    ${LVGL_PATH}/demos/render/*.c
    ${LVGL_PATH}/demos/widgets/*.c
    ${LVGL_PATH}/demos/widgets/assets/*.c
    ${LVGL_PATH}/demos/vector_graphic/*.c
    ${LVGL_PATH}/demos/benchmark/*.c
    ${LVGL_PATH}/demos/benchmark/assets/*.c
    ${LVGL_PATH}/demos/music/*.c
    ${LVGL_PATH}/demos/music/assets/*.c
    ${LVGL_PATH}/examples/libs/gif/*.c
    ${LVGL_PATH}/examples/libs/lodepng/*.c
)

# Demo configuration
set(DEMO_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(DEMO_DEFINES
    LV_AMBIQ_DISPLAY_BUFFER_RESX=464
    LV_AMBIQ_DISPLAY_BUFFER_RESY=464
    CPU_RUN_IN_HP_MODE
    GPU_RUN_IN_HP_MODE
    LV_AMBIQ_USE_DIRECT_MODE=1
    LV_AMBIQ_DRAW_BUFFER_RATIO=1
)

# Create executable target
add_executable(${DEMO_NAME}
    ${COMMON_SOURCES}
    ${DEMO_SOURCES}
    ${LVGL_DEMO_SOURCES}
    ${BSP_SOURCES}
    ${DEVICE_SOURCES}
    ${STARTUP_FILE}
)

set_target_properties(${DEMO_NAME} PROPERTIES SUFFIX ".axf")

# Set output directory to build/<demo_name>/
set_target_properties(${DEMO_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DEMO_NAME}"
)

# Configure target
target_include_directories(${DEMO_NAME} PRIVATE
    ${COMMON_INCLUDES}
    ${DEMO_INCLUDES}
    ${BSP_INCLUDES}
)

target_compile_definitions(${DEMO_NAME} PRIVATE
    ${COMMON_DEFINES}
    ${DEMO_DEFINES}
    ${BSP_DEFINES}
)

target_compile_options(${DEMO_NAME} PRIVATE
    ${BSP_CPU_FLAGS}
    ${COMMON_C_FLAGS}
)

if(DEFINED EXTRA_CFLAGS)
    target_compile_options(${DEMO_NAME} PRIVATE ${EXTRA_CFLAGS})
endif()

target_link_options(${DEMO_NAME} PRIVATE
    -T "${LINKER_SCRIPT}"
    ${BSP_CPU_FLAGS}
    ${BSP_LINKER_ENTRY}
    ${COMMON_LINKER_FLAGS}
    -Wl,-Map,${CMAKE_BINARY_DIR}/${DEMO_NAME}/${DEMO_NAME}.map
)

target_link_libraries(${DEMO_NAME} PRIVATE
    -Wl,--start-group
    lvgl  # Link LVGL static library
    ${COMMON_SYSTEM_LIBS}
    ${COMMON_LIBS}
    -Wl,--end-group
)

# Generate firmware files (output to build/<demo_name>/)
generate_firmware(${DEMO_NAME})

# Add flash target
add_flash_target(${DEMO_NAME} ${BOARD})
