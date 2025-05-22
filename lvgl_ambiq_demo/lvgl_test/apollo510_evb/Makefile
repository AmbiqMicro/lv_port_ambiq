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
TARGET := lvgl_test
COMPILERNAME := gcc
PROJECT := lvgl_test_gcc
CONFIG := bin
SHELL:=/bin/bash
TOP_DIR := ../../..


# Enable printing explicit commands with 'make VERBOSE=1'
ifneq ($(VERBOSE),1)
Q:=@
endif

#### Setup ####

TOOLCHAIN ?= arm-none-eabi
PART = apollo510
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

DEFINES+= -DAM_PART_APOLLO510
DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESX=464
DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESY=464
DEFINES+= -DLV_AMBIQ_USE_DIRECT_MODE=1
DEFINES+= -DLV_AMBIQ_DRAW_BUFFER_RATIO=1
DEFINES+= -DLV_CONF_INCLUDE_SIMPLE
DEFINES+= -DLV_LVGL_H_INCLUDE_SIMPLE

DEFINES+= -DNEMA_PLATFORM=apollo510_nemagfx
DEFINES+= -DVMEM_SIZE=0x3FFFF
DEFINES+= -DWAIT_IRQ_BINARY_SEMAPHORE=1

DEFINES+= -Dgcc
DEFINES+= -Dapollo510_evb

# AmbiqSuite/boards
INCLUDES+= -I../../../AmbiqSuite/boards/apollo510_evb/bsp

VPATH+=:../../../AmbiqSuite/boards/apollo510_evb/bsp

SRC += am_bsp.c
SRC += am_bsp_pins.c

# AmbiqSuite/cmsis
INCLUDES+= -I../../../AmbiqSuite/CMSIS/ARM/Include
INCLUDES+= -I../../../AmbiqSuite/CMSIS/AmbiqMicro/Include

LIBS += ../../../AmbiqSuite/CMSIS/ARM/Lib/ARM/DSP_LIB_CM55/libarm_cortexM55f_math.a

VPATH += ../../../AmbiqSuite/CMSIS/AmbiqMicro/Source
SRC += system_apollo510.c

# AmbiqSuite/devices
INCLUDES+= -I../../../AmbiqSuite/devices
VPATH += ../../../AmbiqSuite/devices
SRC += am_devices_mspi_psram_aps25616ba_1p2v.c
SRC += am_devices_display_generic.c
# SRC += am_devices_tma525.c
SRC += am_devices_chsc5816_ap5.c

# AmbiqSuite/gpu
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/NemaDC
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/NemaGFX
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/NemaGFX/Nema
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/common/mem
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaDC
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaGFX
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaVG
INCLUDES+= -I../../../AmbiqSuite/third_party/ThinkSi/NemaGFX_SDK/include/tsi/common


# NemaGFX_hal
INCLUDES+= -I../../../lvgl_ambiq_porting/NemaGFX_hal/apollo510_nemagfx


VPATH+=:../../../lvgl_ambiq_porting/NemaGFX_hal/apollo510_nemagfx

SRC += nema_dc_hal.c
SRC += nema_hal.c

LIBS += ../../../AmbiqSuite/third_party/ThinkSi/config/apollo510_nemagfx/gcc/bin/lib_nema_apollo510_nemagfx.a

# AmbiqSuite/mcu
INCLUDES+= -I../../../AmbiqSuite/mcu/apollo510
INCLUDES+= -I../../../AmbiqSuite/mcu/apollo510/hal

LIBS += ../../../AmbiqSuite/mcu/apollo510/hal/mcu/gcc/bin/libam_hal.a
# VPATH+=:../../../AmbiqSuite/mcu/apollo510/hal/
# VPATH+=:../../../AmbiqSuite/mcu/apollo510/hal/mcu

# SRC += am_hal_access.c
# SRC += am_hal_adc.c
# SRC += am_hal_audadc.c
# SRC += am_hal_dcu.c
# SRC += am_hal_global.c
# SRC += am_hal_gpio.c
# SRC += am_hal_i2s.c
# SRC += am_hal_info.c
# SRC += am_hal_infoc.c
# SRC += am_hal_pdm.c
# SRC += am_hal_pin.c
# SRC += am_hal_pwrctrl.c
# SRC += am_hal_queue.c
# SRC += am_hal_security.c
# SRC += am_hal_stimer.c
# SRC += am_hal_timer.c
# SRC += am_hal_usb.c
# SRC += am_hal_wdt.c
# SRC += am_hal_bootrom_helper.c
# SRC += am_hal_cachectrl.c
# SRC += am_hal_card.c
# SRC += am_hal_card_host.c
# SRC += am_hal_clkgen.c
# SRC += am_hal_cmdq.c
# SRC += am_hal_dsi.c
# SRC += am_hal_interrupt.c
# SRC += am_hal_iom.c
# SRC += am_hal_ios.c
# SRC += am_hal_itm.c
# SRC += am_hal_mcuctrl.c
# SRC += am_hal_mpu.c
# SRC += am_hal_mram.c
# SRC += am_hal_mspi.c
# SRC += am_hal_reset.c
# SRC += am_hal_rtc.c
# SRC += am_hal_sdhc.c
# SRC += am_hal_secure_ota.c
# SRC += am_hal_sysctrl.c
# SRC += am_hal_syspll.c
# SRC += am_hal_systick.c
# SRC += am_hal_tpiu.c
# SRC += am_hal_uart.c
# SRC += am_hal_utils.c


# AmbiqSuite/utils
INCLUDES+= -I../../../AmbiqSuite/utils

VPATH+=:../../../AmbiqSuite/utils

SRC += am_util_delay.c
SRC += am_util_stdio.c

# lvgl

LVGL_PATH = ../../../LVGL

INCLUDES+= -I../../../LVGL
INCLUDES+= -I../../../LVGL/src/draw/ambiq
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
LVGL_CSRCS += $(shell find $(LVGL_PATH)/demos/render -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/demos/vector_graphic -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/demos/widgets -type f -name '*.c')
LVGL_CSRCS += $(shell find $(LVGL_PATH)/demos/benchmark -type f -name '*.c')

CSRC += $(notdir $(LVGL_CSRCS))

VPATH += $(sort $(dir $(LVGL_CSRCS)))

# freertos

INCLUDES+= -I../../../AmbiqSuite/third_party/FreeRTOSv10.5.1/Source/include
INCLUDES+= -I../../../AmbiqSuite/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5

VPATH+=:../../../AmbiqSuite/third_party/FreeRTOSv10.5.1/Source
VPATH+=:../../../AmbiqSuite/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5
VPATH+=:../../../AmbiqSuite/third_party/FreeRTOSv10.5.1/Source/portable/MemMang

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

# tlsf
INCLUDES+= -I../../../modules/tlsf

VPATH+=:../../../modules/tlsf

SRC += tlsf.c


# lvgl_ambiq_porting
INCLUDES+= -I../../../lvgl_ambiq_porting
LIBS += ../../../lvgl_ambiq_porting/gpu_patch/bin/lvgl_ambiq_porting.a
VPATH+=:../../../lvgl_ambiq_porting

SRC += am_common.c
SRC += am_mem.c
SRC += lv_ambiq_touch.c
# SRC += lv_ambiq_fs.c
SRC += lv_ambiq_display.c

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
CFLAGS+= -fno-common -fno-exceptions -fstack-protector
CFLAGS+= -MMD -MP -std=c99 -Wall -g
CFLAGS+= -Wimplicit-fallthrough -Wundef -Wpointer-arith
CFLAGS+= -Wshadow -Wredundant-decls -Wstrict-prototypes
CFLAGS+= -Wno-sign-compare -Wno-unknown-pragmas -Wno-psabi
CFLAGS+= -O0
CFLAGS+= $(DEFINES)
CFLAGS+= $(INCLUDES)
CFLAGS+= 

# Compiler cxxflags
CXXFLAGS+= -mthumb -mcpu=$(CPU) -mfpu=$(FPU) -mfloat-abi=$(FABI)
CXXFLAGS+= -ffunction-sections -fdata-sections -fomit-frame-pointer
CXXFLAGS+= -fno-common -fno-exceptions -fstack-protector -fno-use-cxa-atexit
CXXFLAGS+= -MMD -MP -std=c++11 -Wall -g
CXXFLAGS+= -Wimplicit-fallthrough -Wundef -Wpointer-arith
CXXFLAGS+= -Wshadow -Wredundant-decls
CXXFLAGS+= -Wno-sign-compare -Wno-unknown-pragmas -Wno-psabi
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

