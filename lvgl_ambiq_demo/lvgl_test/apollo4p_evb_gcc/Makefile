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
#******************************************************************************
TARGET := lvgl_test
COMPILERNAME := gcc
PROJECT := lvgl_test_gcc
CONFIG := bin
TOP_DIR := ../../..
LVGL_DIR := ../../..
LVGL_DIR_NAME := LVGL_V8

SHELL:=/bin/bash

# Enable printing explicit commands with 'make VERBOSE=1'
ifneq ($(VERBOSE),1)
Q:=@
endif

#### Setup ####

TOOLCHAIN ?= arm-none-eabi
PART = apollo4p
CPU = cortex-m4
FPU = fpv4-sp-d16
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

DEFINES+= -DAM_PART_APOLLO4P

DEFINES+= -DLV_AMBIQ_FB_REFRESH_TWO
DEFINES+= -DLV_AMBIQ_FB_RESX=390
DEFINES+= -DLV_AMBIQ_FB_RESY=390
DEFINES+= -DLV_AMBIQ_FB_USE_RGB565=1
DEFINES+= -DLV_AMBIQ_FB_USE_RGB888=0

DEFINES+= -DLV_CONF_INCLUDE_SIMPLE
DEFINES+= -DLV_LVGL_H_INCLUDE_SIMPLE


DEFINES+= -D'NEMA_CUSTOM_MALLOC_INCLUDE="lv_ambiq_nema_hal.h"'
DEFINES+= -DNEMA_PLATFORM=apollo4p_nemagfx
DEFINES+= -DNEMA_USE_CUSTOM_MALLOC
DEFINES+= -DWAIT_IRQ_BINARY_SEMAPHORE=1


DEFINES+= -Dgcc


# bsp/boards
INCLUDES+= -I../../../bsp/boards/apollo4p_evb

VPATH+=:../../../bsp/boards/apollo4p_evb

SRC += am_bsp.c
SRC += am_bsp_pins.c

# bsp/cmsis
INCLUDES+= -I../../../bsp/CMSIS/ARM/Include
INCLUDES+= -I../../../bsp/CMSIS/AmbiqMicro/Include
LIBS = ../../../bsp/CMSIS/ARM/Lib/ARM/libarm_cortexM4lf_math.a

# bsp/devices
INCLUDES+= -I../../../bsp/devices
VPATH += ../../../bsp/devices
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
INCLUDES+= -I../../../bsp/gpu/config/apollo4p_nemagfx

VPATH+=:../../../bsp/gpu/config/apollo4p_nemagfx

SRC += nema_dc_hal.c
SRC += nema_hal.c

LIBS += ../../../bsp/gpu/config/apollo4p_nemagfx/gcc/bin/lib_nema_apollo4p_nemagfx.a

# bsp/mcu
INCLUDES+= -I../../../bsp/mcu/apollo4p
INCLUDES+= -I../../../bsp/mcu/apollo4p/hal
VPATH +=:../../../bsp/mcu/apollo4p/hal

SRC += am_hal_mram.c
SRC += am_hal_cmdq.c
SRC += am_hal_cachectrl.c
SRC += am_hal_dsi.c
SRC += am_hal_fault.c
SRC += am_hal_itm.c
SRC += am_hal_sysctrl.c
SRC += am_hal_iom.c
SRC += am_hal_ios.c
SRC += am_hal_mcuctrl.c
SRC += am_hal_mspi.c
SRC += am_hal_reset.c
SRC += am_hal_systick.c
SRC += am_hal_tpiu.c
SRC += am_hal_uart.c
SRC += am_hal_clkgen.c
SRC += am_hal_rtc.c
SRC += am_hal_secure_ota.c
SRC += am_hal_card_host.c
SRC += am_hal_card.c
SRC += am_hal_sdhc.c
SRC += am_hal_mpu.c

INCLUDES+= -I../../../bsp/mcu/apollo4p/hal/mcu
VPATH +=:../../../bsp/mcu/apollo4p/hal/mcu

SRC += am_hal_access.c
SRC += am_hal_adc.c
SRC += am_hal_i2s.c
SRC += am_hal_pin.c
SRC += am_hal_gpio.c
SRC += am_hal_audadc.c
SRC += am_hal_pdm.c
SRC += am_hal_pwrctrl.c
SRC += am_hal_queue.c
SRC += am_hal_stimer.c
SRC += am_hal_timer.c
SRC += am_hal_utils.c
SRC += am_hal_security.c
SRC += am_hal_wdt.c
SRC += am_hal_otp.c
SRC += am_hal_dcu.c
SRC += am_hal_bootrom_helper.c

INCLUDES+= -I../../../bsp/mcu/apollo4p/hal/dsp
VPATH +=:../../../bsp/mcu/apollo4p/hal/dsp
SRC += am_hal_interrupt.c

# LIBS += ../../../bsp/mcu/apollo4p/hal/mcu/gcc/bin/libam_hal.a

# bsp/utils
INCLUDES+= -I../../../bsp/utils

VPATH+=:../../../bsp/utils

SRC += am_util_delay.c
SRC += am_util_stdio.c

# lvgl
INCLUDES+= -I../../../LVGL_V8
INCLUDES+= -I../../../LVGL_V8/src/gpu

include ../../../LVGL_V8/lvgl.mk

# lvgl_ambiq_porting
INCLUDES+= -I../../../lvgl_ambiq_porting

VPATH+=:../../../lvgl_ambiq_porting

SRC += display_task_fake.c
SRC += display_task_one_and_partial_fb.c
SRC += display_task_one_fb.c
SRC += display_task_two_fb.c
SRC += lv_ambiq_font_align.c
SRC += lv_ambiq_misc.c
SRC += lv_ambiq_nema_hal.c

LIBS += ../../../lvgl_ambiq_porting/gpu_lib_apollo4/gcc/bin/lvgl_ambiq_porting.a

# freertos

INCLUDES+= -I../../../modules/FreeRTOSv10.5.1/Source/include
INCLUDES+= -I../../../modules/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo4

VPATH+=:../../../modules/FreeRTOSv10.5.1/Source
VPATH+=:../../../modules/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo4
VPATH+=:../../../modules/FreeRTOSv10.5.1/Source/portable/MemMang

SRC += event_groups.c
SRC += port.c
SRC += queue.c

SRC += tasks.c
SRC += timers.c
SRC += startup_gcc.c
SRC += heap_4.c
SRC += list.c

# lvgl_test
INCLUDES+= -I../src

VPATH+=:../src

SRC += rtos.c
SRC += am_resources.c
SRC += gui_task.c
SRC += lvgl_test.c



# CSRCS is defined in lvgl.mk.
# Some of files in CSRCS include file path, 
# we should remove the file path and add them to VPATH.
CSRC += $(notdir $(CSRCS))
VPATH += $(sort $(dir $(CSRCS)))

CSRC += $(filter %.c,$(SRC))
ASRCS += $(filter %.s,$(SRC))


OBJS = $(CSRC:%.c=$(CONFIG)/%.o)
OBJS+= $(ASRCS:%.s=$(CONFIG)/%.o)

DEPS = $(CSRC:%.c=$(CONFIG)/%.d)
DEPS+= $(ASRCS:%.s=$(CONFIG)/%.d)


CFLAGS+= -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CFLAGS+= -ffunction-sections -fdata-sections -fomit-frame-pointer
CFLAGS+= -MMD -MP -std=c99 -Wall -g
CFLAGS+= -O0
CFLAGS+= $(DEFINES)
CFLAGS+= $(INCLUDES)
CFLAGS+= 

LFLAGS = -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
LFLAGS+= -nostartfiles -static
LFLAGS+= -Wl,--gc-sections,--entry,Reset_Handler,-Map,$(CONFIG)/$(TARGET).map
LFLAGS+= -Wl,--start-group -lm -lc -lgcc -lnosys $(LIBS) -Wl,--end-group
LFLAGS+= 

# Additional user specified CFLAGS
CFLAGS+=$(EXTRA_CFLAGS)

CPFLAGS = -Obinary

ODFLAGS = -S

#### Rules ####
all: directories $(CONFIG)/$(TARGET).bin

directories: $(CONFIG)

$(CONFIG):
	@mkdir -p $@

$(CONFIG)/%.o: %.c $(CONFIG)/%.d
	@echo " Compiling $(COMPILERNAME) $<"
	$(Q) $(CC) -c $(CFLAGS) $< -o $@

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

