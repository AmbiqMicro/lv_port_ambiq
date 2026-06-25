#******************************************************************************
#
# demo_dL.mk - Smartwatch settings for Apollo510dL (2MB MRAM, reduced SSRAM)
#
# Match apollo510_evb display path (Direct mode). Wallpaper frames are preloaded
# into PSRAM at startup so runtime eMMC I/O does not block lv_timer_handler.
#
#******************************************************************************

TARGET := lvgl_smartwatch

DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESX=392
DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESY=392
DEFINES+= -DCPU_RUN_IN_HP_MODE
DEFINES+= -DGPU_RUN_IN_HP_MODE
DEFINES+= -DLV_AMBIQ_USE_DIRECT_MODE=1
DEFINES+= -DLV_AMBIQ_DRAW_BUFFER_RATIO=1

# Preload all 63 wallpaper frames from eMMC into PSRAM at startup (same as apollo510_evb).
DEFINES+= -DWALLPAPER_PRELOAD_TO_PSRAM=1
DEFINES+= -DWALLPAPER_NUMBER=63

# lvgl_smartwatch
INCLUDES+= -I../src
INCLUDES+= -I../src/gui

VPATH+=:../src

SRC += rtos.c
SRC += am_resources.c
SRC += gui_task.c
SRC += lvgl_smartwatch.c

LVGL_CSRCS += \
    $(wildcard ../src/gui/*.c) \
    $(wildcard ../src/gui/components/*.c) \
    $(wildcard ../src/gui/fonts/*.c) \
    $(wildcard ../src/gui/images/*.c) \
    $(wildcard ../src/gui/screens/*.c)
