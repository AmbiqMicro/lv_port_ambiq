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

include ../common.mk