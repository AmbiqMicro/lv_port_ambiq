#******************************************************************************
# lvgl_version.cmake - LVGL version information
#******************************************************************************

# Include LVGL official version file
if(EXISTS "${LVGL_PATH}/env_support/cmake/version.cmake")
    include("${LVGL_PATH}/env_support/cmake/version.cmake")
    message(STATUS "LVGL Version: ${LVGL_VERSION}")
else()
    # Fallback if version file not found
    set(LVGL_VERSION_MAJOR "9")
    set(LVGL_VERSION_MINOR "3")
    set(LVGL_VERSION_PATCH "0")
    set(LVGL_VERSION "${LVGL_VERSION_MAJOR}.${LVGL_VERSION_MINOR}.${LVGL_VERSION_PATCH}")
    message(WARNING "LVGL version file not found, using default: ${LVGL_VERSION}")
endif()
