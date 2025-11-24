
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
COMPILERNAME := gcc
CONFIG := bin
SHELL:=/bin/bash


# TOP_DIR should be set by calling Makefile via export
# If not set (backward compatibility), calculate from common.mk location
ifndef TOP_DIR
    # Calculate TOP_DIR based on common.mk location
    # common.mk is in lvgl_ambiq_demo/, TOP_DIR should be parent directory
    COMMON_MK_PATH := $(lastword $(filter %common.mk,$(MAKEFILE_LIST)))
    COMMON_MK_DIR := $(dir $(abspath $(COMMON_MK_PATH)))
    TOP_DIR := $(abspath $(COMMON_MK_DIR)/..)
endif

# Check for Bash availability
ifeq (, $(shell which bash))
    $(error "Bash is not found in PATH. Please install Bash (e.g., via Cygwin) and ensure it's in your PATH before running make.")
endif

# Enable printing explicit commands with 'make VERBOSE=1'
ifneq ($(VERBOSE),1)
Q:=@
endif

# Set the default AmbiqSuite path
AMBIQSUITE_PATH ?= $(TOP_DIR)/AmbiqSuite
# Set your AmbiqSuit path
AMBIQSUITE_PATH = $(TOP_DIR)/../ambiqsuite

LVGL_PATH = $(TOP_DIR)/LVGL
LVGL_AMBIQ_PORTING_PATH = $(TOP_DIR)/lvgl_ambiq_porting
FREETYPE_PATH = $(TOP_DIR)/modules/freetype

ifeq ($(wildcard $(AMBIQSUITE_PATH)),)
$(error The AmbiqSuite directory $(AMBIQSUITE_PATH) does not exist.)
endif
ifeq ($(wildcard $(LVGL_PATH)),)
$(error The LVGL directory $(LVGL_PATH) does not exist.)
endif
ifeq ($(wildcard $(FREETYPE_PATH)),)
$(error The FREETYPE_PATH directory $(FREETYPE_PATH) does not exist.)
endif

# At the beginning of common.mk, after TOP_DIR definition
# Support for top-level building
ifneq ($(PROJECT),)
    # Building from top-level, set paths accordingly
    PROJECT_SRC := $(PROJECT)/src
    # Output goes to build/$(PROJECT)/$(BOARD)/
    CONFIG ?= build/$(PROJECT)/$(BOARD)
else
    # Building from subdirectory (backward compatibility)
    CONFIG ?= bin
endif

# # After VPATH setup, add:
# $(info LVGL_CSRCS sample: $(wordlist 1,3,$(LVGL_CSRCS)))
# $(info LVGL_DIRS sample: $(wordlist 1,3,$(LVGL_DIRS)))
# $(info VPATH sample: $(wordlist 1,5,$(VPATH)))

#### Setup ####

TOOLCHAIN ?= arm-none-eabi

#### BSP Configuration ####
# BSP should be set in board-specific Makefile (e.g., apollo510_evb, apollo510b_evb, apollo5b_eb_revb)
# BSP_DIR points to the shared BSP files for this board
# common.mk is always in lvgl_ambiq_demo/, so bsp/ is in the same directory

# Determine common.mk directory
# When building from top-level (PROJECT is set), CURDIR is lvgl_ambiq_demo/
# When building from subdirectory, calculate from common.mk location
ifneq ($(PROJECT),)
    # Building from top-level Makefile
    COMMON_MK_DIR := $(CURDIR)/
else
    # Building from subdirectory, find common.mk location
    COMMON_MK_PATH := $(lastword $(filter %common.mk,$(MAKEFILE_LIST)))
    ifeq ($(COMMON_MK_PATH),)
        # Fallback: try to find common.mk
        COMMON_MK_PATH := $(abspath common.mk)
        ifeq ($(wildcard $(COMMON_MK_PATH)),)
            COMMON_MK_PATH := $(abspath ../common.mk)
            ifeq ($(wildcard $(COMMON_MK_PATH)),)
                COMMON_MK_PATH := $(abspath ../../common.mk)
            endif
        endif
    endif
    COMMON_MK_DIR := $(dir $(abspath $(COMMON_MK_PATH)))
    COMMON_MK_DIR := $(if $(filter %/,$(COMMON_MK_DIR)),$(COMMON_MK_DIR),$(COMMON_MK_DIR)/)
endif

# BSP directory is relative to common.mk location
BSP_DIR ?= $(COMMON_MK_DIR)bsp/$(BSP)

# Verify BSP is set
ifeq ($(BSP),)
$(error BSP variable must be set in board Makefile (e.g., BSP=apollo510_evb))
endif

# Verify BSP directory exists
ifeq ($(wildcard $(BSP_DIR)),)
$(error BSP directory $(BSP_DIR) does not exist. Please create it or set BSP correctly.)
endif

# Set linker and startup file paths (can be overridden in board Makefile)
LINKER_FILE ?= $(BSP_DIR)/linker_script.ld
STARTUP_FILE ?= $(BSP_DIR)/startup_$(COMPILERNAME).c

# Verify files exist
ifeq ($(wildcard $(LINKER_FILE)),)
$(error Linker script $(LINKER_FILE) not found in BSP directory $(BSP_DIR))
endif
ifeq ($(wildcard $(STARTUP_FILE)),)
$(error Startup file $(STARTUP_FILE) not found in BSP directory $(BSP_DIR))
endif

STARTUP_FILE ?= $(BSP_DIR)/startup_$(COMPILERNAME).c

# Verify files exist
ifeq ($(wildcard $(LINKER_FILE)),)
$(error Linker script $(LINKER_FILE) not found in BSP directory $(BSP_DIR))
endif
ifeq ($(wildcard $(STARTUP_FILE)),)
$(error Startup file $(STARTUP_FILE) not found in BSP directory $(BSP_DIR))
endif

# Verify files exist
ifeq ($(wildcard $(LINKER_FILE)),)
$(error Linker script $(LINKER_FILE) not found in BSP directory $(BSP_DIR))
endif
ifeq ($(wildcard $(STARTUP_FILE)),)
$(error Startup file $(STARTUP_FILE) not found in BSP directory $(BSP_DIR))
endif

# Add startup file to source list
STARTUP_SRC := $(notdir $(STARTUP_FILE))
ifneq ($(STARTUP_SRC),)
    SRC += $(STARTUP_SRC)
endif

#### Required Executables ####


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

# Check for Bash availability
ifeq (, $(shell which bash))
    $(error "Bash is not found in PATH. Please install Bash (e.g., via Cygwin) and ensure it's in your PATH before running make.")
endif

# Set shell to Bash for consistent behavior
SHELL:=bash
.SHELLFLAGS:=-euo pipefail -c


DEFINES+= -DLV_CONF_INCLUDE_SIMPLE
DEFINES+= -DLV_LVGL_H_INCLUDE_SIMPLE
DEFINES+= -DNEMA_PLATFORM=apollo510_nemagfx
DEFINES+= -DVMEM_SIZE=0x3FFFF
DEFINES+= -DWAIT_IRQ_BINARY_SEMAPHORE=1

DEFINES+= -Dgcc

# AmbiqSuite/gpu
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/NemaDC
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/NemaGFX
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/NemaGFX/Nema
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/common/mem
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaDC
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaGFX
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/include/tsi/NemaVG
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/ThinkSi/NemaGFX_SDK/include/tsi/common

# AmbiqSuite/FatFs
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/FatFs/apollo_driver/mmc_apollo5
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/FatFs/source

VPATH+=:$(AMBIQSUITE_PATH)/third_party/FatFs/apollo_driver/mmc_apollo5
VPATH+=:$(AMBIQSUITE_PATH)/third_party/FatFs/source

SRC += mmc_apollo5.c
SRC += ff.c
SRC += ffsystem.c
SRC += ffunicode.c
SRC += diskio.c

# NemaGFX_hal
INCLUDES+= -I$(LVGL_AMBIQ_PORTING_PATH)/NemaGFX_hal/apollo510_nemagfx
VPATH+=:$(LVGL_AMBIQ_PORTING_PATH)/NemaGFX_hal/apollo510_nemagfx

SRC += nema_dc_hal.c
SRC += nema_hal.c

LIBS += $(AMBIQSUITE_PATH)/third_party/ThinkSi/config/apollo510_nemagfx/gcc/bin/lib_nema_apollo510_nemagfx.a

# AmbiqSuite/mcu
INCLUDES+= -I$(AMBIQSUITE_PATH)/mcu/apollo510
INCLUDES+= -I$(AMBIQSUITE_PATH)/mcu/apollo510/hal
LIBS += $(AMBIQSUITE_PATH)/mcu/apollo510/hal/mcu/gcc/bin/libam_hal.a
# VPATH+=:$(AMBIQSUITE_PATH)/mcu/apollo510/hal/
# VPATH+=:$(AMBIQSUITE_PATH)/mcu/apollo510/hal/mcu

# AmbiqSuite/utils
INCLUDES+= -I$(AMBIQSUITE_PATH)/utils
VPATH+=:$(AMBIQSUITE_PATH)/utils

SRC += am_util_delay.c
SRC += am_util_stdio.c

# lvgl
INCLUDES+= -I$(LVGL_PATH)
INCLUDES+= -I$(LVGL_PATH)/src/draw/ambiq

LVGL_CSRCS += \
    $(wildcard $(LVGL_PATH)/src/core/*.c) \
    $(wildcard $(LVGL_PATH)/src/display/*.c) \
    $(wildcard $(LVGL_PATH)/src/draw/*.c) \
    $(wildcard $(LVGL_PATH)/src/draw/ambiq/*.c) \
    $(wildcard $(LVGL_PATH)/src/draw/sw/*.c) \
    $(wildcard $(LVGL_PATH)/src/draw/sw/blend/*.c) \
    $(wildcard $(LVGL_PATH)/src/draw/sw/grid/*.c) \
    $(wildcard $(LVGL_PATH)/src/font/*.c) \
    $(wildcard $(LVGL_PATH)/src/indev/*.c) \
    $(wildcard $(LVGL_PATH)/src/layouts/*.c) \
    $(wildcard $(LVGL_PATH)/src/layouts/flex/*.c) \
    $(wildcard $(LVGL_PATH)/src/layouts/grid/*.c) \
    $(wildcard $(LVGL_PATH)/src/misc/*.c) \
    $(wildcard $(LVGL_PATH)/src/misc/cache/*.c) \
    $(wildcard $(LVGL_PATH)/src/misc/cache/class/*.c) \
    $(wildcard $(LVGL_PATH)/src/misc/cache/instance/*.c) \
    $(wildcard $(LVGL_PATH)/src/osal/*.c) \
    $(wildcard $(LVGL_PATH)/src/others/*.c) \
    $(wildcard $(LVGL_PATH)/src/others/sysmon/*.c) \
    $(wildcard $(LVGL_PATH)/src/others/observer/*.c) \
    $(wildcard $(LVGL_PATH)/src/stdlib/*.c) \
    $(wildcard $(LVGL_PATH)/src/stdlib/builtin/*.c) \
    $(wildcard $(LVGL_PATH)/src/stdlib/clib/*.c) \
    $(wildcard $(LVGL_PATH)/src/themes/*.c) \
    $(wildcard $(LVGL_PATH)/src/themes/default/*.c) \
    $(wildcard $(LVGL_PATH)/src/themes/mono/*.c) \
    $(wildcard $(LVGL_PATH)/src/themes/simple/*.c) \
    $(wildcard $(LVGL_PATH)/src/tick/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/animimage/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/arc/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/bar/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/button/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/buttonmatrix/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/calendar/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/canvas/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/chart/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/checkbox/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/dropdown/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/image/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/imagebutton/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/keyboard/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/label/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/led/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/line/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/list/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/lottie/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/menu/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/msgbox/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/objx_templ/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/objx_templ/property/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/roller/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/scale/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/slider/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/span/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/spinbox/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/spinner/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/switch/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/table/*.c) \
    $(wildcard $(LVGL_PATH)/src/widgets/tabview/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/textarea/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/tileview/*.c) \
	$(wildcard $(LVGL_PATH)/src/widgets/win/*.c) \
    $(wildcard $(LVGL_PATH)/src/libs/bin_decoder/*.c) \
    $(wildcard $(LVGL_PATH)/src/libs/fsdrv/*.c) \
    $(wildcard $(LVGL_PATH)/src/libs/gif/*.c) \
    $(wildcard $(LVGL_PATH)/src/libs/lodepng/*.c) \
    $(wildcard $(LVGL_PATH)/src/libs/freetype/*.c)

LVGL_CSRCS += $(LVGL_PATH)/src/lv_init.c

# 生成 CSRC
CSRC += $(notdir $(LVGL_CSRCS))

# 保持相对路径（原来的方式）
VPATH += $(sort $(dir $(LVGL_CSRCS)))

# After VPATH setup, add:
$(info LVGL_CSRCS sample: $(wordlist 1,3,$(LVGL_CSRCS)))
$(info LVGL_DIRS sample: $(wordlist 1,3,$(LVGL_DIRS)))
$(info VPATH sample: $(wordlist 1,5,$(VPATH)))
# freertos
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/FreeRTOSv10.5.1/Source/include
INCLUDES+= -I$(AMBIQSUITE_PATH)/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5

VPATH+=:$(AMBIQSUITE_PATH)/third_party/FreeRTOSv10.5.1/Source
VPATH+=:$(AMBIQSUITE_PATH)/third_party/FreeRTOSv10.5.1/Source/portable/GCC/AMapollo5
VPATH+=:$(AMBIQSUITE_PATH)/third_party/FreeRTOSv10.5.1/Source/portable/MemMang

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
INCLUDES+= -I$(TOP_DIR)/modules/tlsf
VPATH+=:$(TOP_DIR)/modules/tlsf
SRC += tlsf.c

# lvgl_ambiq_porting
INCLUDES+= -I$(LVGL_AMBIQ_PORTING_PATH)
LIBS += $(LVGL_AMBIQ_PORTING_PATH)/gpu_patch/bin/lvgl_ambiq_porting.a
VPATH+=:$(LVGL_AMBIQ_PORTING_PATH)

SRC += am_common.c
SRC += am_mem.c
SRC += lv_ambiq_touch.c
SRC += lv_ambiq_fs.c
SRC += lv_ambiq_display.c

# AmbiqSuite/cmsis
INCLUDES+= -I$(AMBIQSUITE_PATH)/CMSIS/ARM/Include
INCLUDES+= -I$(AMBIQSUITE_PATH)/CMSIS/AmbiqMicro/Include
LIBS += $(AMBIQSUITE_PATH)/CMSIS/ARM/Lib/ARM/DSP_LIB_CM55/libarm_cortexM55f_math.a
VPATH += $(AMBIQSUITE_PATH)/CMSIS/AmbiqMicro/Source

# AmbiqSuite/devices
INCLUDES+= -I$(AMBIQSUITE_PATH)/devices
VPATH += $(AMBIQSUITE_PATH)/devices

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
CFLAGS+= -O3
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
CXXFLAGS+= -O3
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

