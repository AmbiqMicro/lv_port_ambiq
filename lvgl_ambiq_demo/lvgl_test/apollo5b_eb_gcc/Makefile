#******************************************************************************
#
# Makefile - Rules for building the libraries, examples and docs.
#
# Copyright (c) 2024, Ambiq Micro, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# Third party software included in this distribution is subject to the
# additional license terms as defined in the /docs/licenses directory.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# This is part of revision release_sdk_4_5_0-a1ef3b89f9 of the AmbiqSuite Development Package.
#
#******************************************************************************TARGET := lvgl_test
TARGET := lvgl_v9_test
COMPILERNAME := gcc
PROJECT := lvgl_v9_test_gcc
CONFIG := bin
SHELL:=/bin/bash
TOP_DIR := ../../..


# Enable printing explicit commands with 'make VERBOSE=1'
ifneq ($(VERBOSE),1)
Q:=@
endif

#### Setup ####

TOOLCHAIN ?= arm-none-eabi
PART = apollo5b
CPU = cortex-m55
FPU = auto
# Default to FPU hardware calling convention.  However, some customers and/or
# applications may need the software calling convention.
#FABI = softfp
FABI = hard

LINKER_FILE := ./linker_script.ld
STARTUP_FILE := ./startup_$(COMPILERNAME).c

#### Required Executables ####
CC = $(TOOLCHAIN)-gcc
GCC = $(TOOLCHAIN)-gcc
CPP = $(TOOLCHAIN)-cpp
LD = $(TOOLCHAIN)-ld
CP = $(TOOLCHAIN)-objcopy
OD = $(TOOLCHAIN)-objdump
RD = $(TOOLCHAIN)-readelf
AR = $(TOOLCHAIN)-ar
SIZE = $(TOOLCHAIN)-size
RM = $(shell which rm 2>/dev/null)

EXECUTABLES = CC LD CP OD AR RD SIZE GCC
K := $(foreach exec,$(EXECUTABLES),\
		$(if $(shell which $($(exec)) 2>/dev/null),,\
		$(info $(exec) not found on PATH ($($(exec))).)$(exec)))
$(if $(strip $(value K)),$(info Required Program(s) $(strip $(value K)) not found))

ifneq ($(strip $(value K)),)
all clean:
	$(info Tools $(TOOLCHAIN)-$(COMPILERNAME) not installed.)
	$(RM) -rf bin
else

DEFINES+= -DAM_PART_APOLLO5B

DEFINES+= -DLV_AMBIQ_USE_DIRECT_MODE=1
DEFINES+= -DLV_AMBIQ_DRAW_BUFFER_RATIO=1
DEFINES+= -DLV_CONF_INCLUDE_SIMPLE
DEFINES+= -DLV_LVGL_H_INCLUDE_SIMPLE

DEFINES+= -DNEMA_PLATFORM=apollo5b_nemagfx
DEFINES+= -DVMEM_SIZE=0x7FFFF
DEFINES+= -DWAIT_IRQ_BINARY_SEMAPHORE=1

DEFINES+= -Dgcc

# bsp/boards
INCLUDES+= -I../../../bsp/boards/apollo5b_eb

VPATH+=:../../../bsp/boards/apollo5b_eb

SRC += am_bsp.c
SRC += am_bsp_pins.c

# bsp/cmsis
INCLUDES+= -I../../../bsp/CMSIS/ARM/Include
INCLUDES+= -I../../../bsp/CMSIS/AmbiqMicro/Include

LIBS += ../../../bsp/CMSIS/ARM/Lib/ARM/libarm_cortexM4lf_math.a

VPATH += ../../../bsp/CMSIS/AmbiqMicro/Source
SRC += system_apollo5b.c

# bsp/devices
INCLUDES+= -I../../../bsp/devices
VPATH += ../../../bsp/devices
SRC += am_devices_mspi_psram_aps25616n.c
SRC += am_devices_display_generic.c
SRC += am_devices_tma525.c

# bsp/gpu
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/NemaDC
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/NemaGFX
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/common/mem
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/include/tsi/NemaDC
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/include/tsi/NemaGFX
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/include/tsi/NemaVG
INCLUDES+= -I../../../bsp/gpu/NemaGFX_SDK/include/tsi/common
INCLUDES+= -I../../../bsp/gpu/config/apollo5b_nemagfx

VPATH+=:../../../bsp/gpu/config/apollo5b_nemagfx

SRC += nema_dc_hal.c
SRC += nema_hal.c

LIBS += ../../../bsp/gpu/config/apollo5b_nemagfx/gcc/bin/lib_nema_apollo5b_nemagfx.a

# bsp/mcu
INCLUDES+= -I../../../bsp/mcu/apollo5b
INCLUDES+= -I../../../bsp/mcu/apollo5b/hal

LIBS += ../../../bsp/mcu/apollo5b/hal/mcu/gcc/bin/libam_hal.a

# bsp/utils
INCLUDES+= -I../../../bsp/utils

VPATH+=:../../../bsp/utils

SRC += am_util_delay.c
SRC += am_util_stdio.c

# lvgl

LVGL_PATH = ../../../LVGL_V9

INCLUDES+= -I../../../LVGL_V9
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/core -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/display -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/draw -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/font -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/indev -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/layouts -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/misc -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/osal -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/others -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/stdlib -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/themes -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/tick -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/widgets -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/src/libs/bin_decoder -type f -name '*.c')
LVGL_CSRCS += $(LVGL_PATH)/src/lv_init.c
LVGL_CSRCS += $(shell find $(LVGL_PATH)/demos/scroll -type f -name '*.c')

CSRC += $(notdir $(LVGL_CSRCS))

VPATH += $(sort $(dir $(LVGL_CSRCS)))

# freertos

INCLUDES+= -I../../../modules/FreeRTOSv10.5.1/Source/include
INCLUDES+= -I../../../modules/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5

VPATH+=:../../../modules/FreeRTOSv10.5.1/Source
VPATH+=:../../../modules/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5
VPATH+=:../../../modules/FreeRTOSv10.5.1/Source/portable/MemMang

SRC += event_groups.c
SRC += queue.c
SRC += tasks.c
SRC += timers.c
SRC += startup_gcc.c
SRC += heap_4.c
SRC += list.c

SRC += port.c
SRC += port_stimer.c
SRC += port_systick_stimer.c
SRC += portasm.c

# lvgl_test
INCLUDES+= -I../src

VPATH+=:../src

SRC += rtos.c
SRC += am_resources.c
SRC += gui_task.c
SRC += lvgl_test.c
SRC += display_task.c






CSRC += $(filter %.c,$(SRC))
ASRC += $(filter %.s,$(SRC))
CPPSRC += $(filter %.cpp,$(SRC))

OBJS = $(CSRC:%.c=$(CONFIG)/%.o)
OBJS+= $(ASRC:%.s=$(CONFIG)/%.o)
OBJS+= $(CPPSRC:%.cpp=$(CONFIG)/%.o)

DEPS = $(CSRC:%.c=$(CONFIG)/%.d)
DEPS+= $(ASRC:%.s=$(CONFIG)/%.d)
DEPS+= $(CPPSRC:%.cpp=$(CONFIG)/%.d)



# Compiler cflags
CFLAGS+= -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CFLAGS+= -ffunction-sections -fdata-sections -fomit-frame-pointer
CFLAGS+= -MMD -MP -std=c99 -Wall -g
CFLAGS+= -O0
CFLAGS+= $(DEFINES)
CFLAGS+= $(INCLUDES)
CFLAGS+= 

# Compiler cxxflags
CXXFLAGS+= -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CXXFLAGS+= -ffunction-sections -fdata-sections -fomit-frame-pointer
CXXFLAGS+= -MMD -MP -std=c++11 -fno-use-cxa-atexit -Wall -g
CXXFLAGS+= -O0
CXXFLAGS+= $(DEFINES)
CXXFLAGS+= $(INCLUDES)
CXXFLAGS+= 

LFLAGS = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
LFLAGS+= -nostartfiles -static
LFLAGS+= -Wl,--gc-sections,--entry,Reset_Handler,-Map,$(CONFIG)/$(TARGET).map
LFLAGS+= -Wl,--start-group -lm -lc -lgcc -lnosys -lstdc++ $(LIBS) -Wl,--end-group
LFLAGS+= 

# Additional user specified CFLAGS
CFLAGS+=$(EXTRA_CFLAGS)

CPFLAGS = -Obinary

ODFLAGS = -S

#### Rules ####
all: directories $(CONFIG)/$(TARGET).bin


ifeq ($(OS),Windows_NT)
    JLINK := JLink.exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        JLINK := JLinkExe
    else ifeq ($(UNAME_S),Darwin)
        JLINK := JLinkExe
	else
        $(error OS not support)
    endif
endif

.PHONY: flash
flash: all
	$(JLINK) -commanderscript flash.jlink

.PHONY: ozone
ozone: all
	ozone oz.jdebug &

directories: $(CONFIG)

$(CONFIG):
	@mkdir -p $@

$(CONFIG)/%.o: %.c $(CONFIG)/%.d
	@echo " Compiling $(COMPILERNAME) $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

$(CONFIG)/%.o: %.cpp $(CONFIG)/%.d
	@echo " Compiling $(COMPILERNAME) $<"
	$(Q) $(CC) -c $(CXXFLAGS) $< -o $@

$(CONFIG)/%.o: %.s $(CONFIG)/%.d
	@echo " Assembling $(COMPILERNAME) $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

$(CONFIG)/$(TARGET).axf: $(OBJS) $(LIBS)
	@echo " Linking $(COMPILERNAME) $@"
	$(Q) $(CC) -Wl,-T,$(LINKER_FILE) -o $@ $(OBJS) $(LFLAGS)

$(CONFIG)/$(TARGET).bin: $(CONFIG)/$(TARGET).axf
	@echo " Copying $(COMPILERNAME) $@..."
	$(Q) $(CP) $(CPFLAGS) $< $@
	$(Q) $(OD) $(ODFLAGS) $< > $(CONFIG)/$(TARGET).lst
	$(Q) $(SIZE) $(OBJS) $(LIBS) $(CONFIG)/$(TARGET).axf >$(CONFIG)/$(TARGET).size

clean:
	@echo "Cleaning..."
	$(Q) $(RM) -rf $(CONFIG)
$(CONFIG)/%.d: ;

# Automatically include any generated dependencies
-include $(DEPS)
endif
.PHONY: all clean directories

