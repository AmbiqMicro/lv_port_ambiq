
TARGET := lvgl_test

DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESX=464
DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESY=464
DEFINES+= -DCPU_RUN_IN_HP_MODE
DEFINES+= -DGPU_RUN_IN_HP_MODE
DEFINES+= -DLV_AMBIQ_USE_DIRECT_MODE=1
DEFINES+= -DLV_AMBIQ_DRAW_BUFFER_RATIO=1
# lvgl_test

INCLUDES+= -I../src
VPATH+=:../src

SRC += rtos.c
SRC += am_resources.c
SRC += gui_task.c
SRC += lvgl_test.c

# LVGL sources
LVGL_CSRCS += \
    $(wildcard $(LVGL_PATH)/demos/scroll/*.c) \
    $(wildcard $(LVGL_PATH)/demos/render/*.c) \
    $(wildcard $(LVGL_PATH)/demos/vector_graphic/*.c) \
    $(wildcard $(LVGL_PATH)/demos/benchmark/*.c) \
    $(wildcard $(LVGL_PATH)/demos/music/*.c) \
    $(wildcard $(LVGL_PATH)/demos/music/assets/*.c) \
    $(wildcard $(LVGL_PATH)/examples/libs/gif/*.c) \
    $(wildcard $(LVGL_PATH)/examples/libs/lodepng/*.c)