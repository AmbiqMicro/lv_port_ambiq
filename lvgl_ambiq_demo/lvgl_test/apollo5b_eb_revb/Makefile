TARGET := lvgl_test

PART = apollo510
CPU = cortex-m55
FPU = auto
FABI = hard

BSP = apollo5b_eb_revb
DEFINES+= -Dapollo5b_eb_revb
# AmbiqSuite/boards
INCLUDES+= -I$(AMBIQSUITE_PATH)/boards/$(BSP)/bsp
VPATH+=:$(AMBIQSUITE_PATH)/boards/$(BSP)/bsp

SRC += am_bsp.c
SRC += am_bsp_pins.c
SRC += system_apollo510.c

# AmbiqSuite/devices
# SRC += am_devices_mspi_psram_aps25616ba_1p2v.c
SRC += am_devices_mspi_psram_aps25616n.c
SRC += am_devices_display_generic.c
# SRC += am_devices_tma525.c
SRC += am_devices_chsc5816_ap5.c

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
    $(wildcard $(LVGL_PATH)/examples/libs/gif/*.c) \
    $(wildcard $(LVGL_PATH)/examples/libs/lodepng/*.c)
# Common makefile
include ../../common.mk