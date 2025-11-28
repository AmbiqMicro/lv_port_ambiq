#******************************************************************************
# lvgl_library.cmake - LVGL static library configuration
# Note: LVGL_VERSION is defined in parent CMakeLists.txt via lvgl_version.cmake
#******************************************************************************

# Collect all LVGL source files recursively
# This automatically includes all modules (core, widgets, themes, draw backends, etc.)
file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS
    ${LVGL_PATH}/src/*.c
)

# Create LVGL static library
add_library(lvgl STATIC ${LVGL_SOURCES})

# Set include directories
# COMMON_INCLUDES (FreeRTOS, CMSIS, etc.) is defined in parent CMakeLists.txt
target_include_directories(lvgl PUBLIC
    ${LVGL_PATH}
    ${LVGL_PATH}/src/draw/ambiq
    ${COMMON_INCLUDES}
)

# Setup lv_conf.h configuration
# DEMO_SRC_DIR is set in parent CMakeLists.txt: {DEMO_NAME}/src/
if(NOT LV_BUILD_USE_KCONFIG)
    target_compile_definitions(lvgl PUBLIC LV_KCONFIG_IGNORE)

    if(NOT DEFINED DEMO_SRC_DIR)
        message(FATAL_ERROR "DEMO_SRC_DIR is not defined. Please set it before including lvgl_library.cmake")
    endif()

    set(CONF_PATH ${DEMO_SRC_DIR}/lv_conf.h)
    if(NOT EXISTS ${CONF_PATH})
        message(FATAL_ERROR "lv_conf.h not found at: ${CONF_PATH}")
    endif()

    message(STATUS "Using lv_conf.h from: ${CONF_PATH}")

    target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)
    target_include_directories(lvgl PUBLIC ${DEMO_SRC_DIR})
endif()

# Set compile options for lvgl library
# COMMON_C_FLAGS and BSP_CPU_FLAGS are defined in parent CMakeLists.txt
target_compile_options(lvgl PRIVATE
    ${COMMON_C_FLAGS}
    ${BSP_CPU_FLAGS}
)

# Set library properties
# Note: VERSION property is not set for static libraries in embedded systems
# as it causes linker errors with ARM GCC (--major-image-version not supported)
set_target_properties(lvgl PROPERTIES
    OUTPUT_NAME "lvgl"
)

message(STATUS "LVGL library configured (version ${LVGL_VERSION})")
