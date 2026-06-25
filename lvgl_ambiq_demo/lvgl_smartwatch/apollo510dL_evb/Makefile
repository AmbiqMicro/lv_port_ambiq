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
# CPU
PART = apollo510L
CPU = cortex-m55
FPU = auto
FABI = hard

DEFINES+= -DAM_PART_APOLLO510L

# BSP
BSP = apollo510dL_evb
DEFINES+= -Dapollo510dL_evb
# AmbiqSuite/boards
INCLUDES+= -I$(AMBIQSUITE_PATH)/boards/$(BSP)/bsp
VPATH+=:$(AMBIQSUITE_PATH)/boards/$(BSP)/bsp

SRC += am_bsp.c
SRC += am_bsp_pins.c
SRC += system_apollo510L.c

# AmbiqSuite/devices
SRC += am_devices_mspi_psram_aps25616ba_1p2v.c
SRC += am_devices_display_generic.c
# SRC += am_devices_tma525.c
SRC += am_devices_chsc5816_ap5.c


# Lite demo: digital/analog watch faces, call, weather, SpO2; wallpaper 63 frames preloaded to PSRAM at boot.
APOLLO510DL_LITE = 1

# Board-specific demo profile
include  ../demo_dL.mk
include  ../demo_lite.mk
include ../../common.mk

