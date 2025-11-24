# boards.mk - Board configuration definitions
# Each board configuration is defined as a variable with board name

# apollo510_evb configuration
define board_apollo510_evb
PART := apollo510
CPU := cortex-m55
FPU := auto
FABI := hard
BSP := apollo510_evb
DEFINES += -DAM_PART_APOLLO510
DEFINES += -Dapollo510_evb
INCLUDES += -I$$(AMBIQSUITE_PATH)/boards/apollo510_evb/bsp
VPATH += :$$(AMBIQSUITE_PATH)/boards/apollo510_evb/bsp
SRC += am_bsp.c
SRC += am_bsp_pins.c
SRC += system_apollo510.c
SRC += am_devices_mspi_psram_aps25616ba_1p2v.c
SRC += am_devices_display_generic.c
SRC += am_devices_chsc5816_ap5.c
endef

# apollo510b_evb configuration
define board_apollo510b_evb
PART := apollo510
CPU := cortex-m55
FPU := auto
FABI := hard
BSP := apollo510b_evb
DEFINES += -DAM_PART_APOLLO510
DEFINES += -Dapollo510b_evb
INCLUDES += -I$$(AMBIQSUITE_PATH)/boards/apollo510b_evb/bsp
INCLUDES += -I$$(AMBIQSUITE_PATH)/third_party/cordio/wsf/sources/port/freertos
INCLUDES += -I$$(AMBIQSUITE_PATH)/third_party/cordio/wsf/sources/util
VPATH += :$$(AMBIQSUITE_PATH)/boards/apollo510b_evb/bsp
SRC += am_bsp.c
SRC += am_bsp_pins.c
SRC += system_apollo510.c
SRC += am_devices_mspi_psram_aps25616ba_1p2v.c
SRC += am_devices_display_generic.c
SRC += am_devices_chsc5816_ap5.c
SRC += am_devices_em9305.c
endef

# apollo5b_eb_revb configuration
define board_apollo5b_eb_revb
PART := apollo510
CPU := cortex-m55
FPU := auto
FABI := hard
BSP := apollo5b_eb_revb
DEFINES += -DAM_PART_APOLLO510
DEFINES += -Dapollo5b_eb_revb
INCLUDES += -I$$(AMBIQSUITE_PATH)/boards/apollo5b_eb_revb/bsp
VPATH += :$$(AMBIQSUITE_PATH)/boards/apollo5b_eb_revb/bsp
SRC += am_bsp.c
SRC += am_bsp_pins.c
SRC += system_apollo510.c
SRC += am_devices_mspi_psram_aps25616n.c
SRC += am_devices_display_generic.c
SRC += am_devices_chsc5816_ap5.c
endef
