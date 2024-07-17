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
#
#******************************************************************************
TARGET := lvgl_test
COMPILERNAME := Keil6
PROJECT := apollo4p_evb_keil6
CONFIG := bin
AM_SoftwareRoot ?= ../../../

SHELL:=/bin/bash
#### Required Executables ####
K := $(shell type -p UV5.exe)
RM := $(shell which rm 2>/dev/null)

ifeq ($(K),)
all clean:
	$(info Tools w/$(COMPILERNAME) not installed.)
	$(RM) -rf bin
else
LIBS = ../../../lvgl_ambiq_porting/gpu_lib_apollo4/keil6/bin/lvgl_ambiq_porting.lib
LIBS+= ../../../bsp/gpu/config/apollo4p_nemagfx/keil6/bin/lib_nema_apollo4p_nemagfx.lib
LIBS+= ../../../bsp/CMSIS/ARM/Lib/ARM/arm_cortexM4lf_math.lib
INCS = ../../../bsp/boards/apollo4p_evb
INCS+= ../../../modules/FreeRTOSv10.5.1/Source/portable/Keil6/AMapollo4
INCS+= ../../../bsp/gpu/NemaGFX_SDK/common/mem
INCS+= ../../..
INCS+= ../src
INCS+= ../../../lvgl_ambiq_porting
INCS+= ../../../bsp/gpu/NemaGFX_SDK/include/tsi/NemaDC
INCS+= ../../../modules/FreeRTOSv10.5.1/Source/include
INCS+= ../../../LVGL_V8/src
INCS+= ../../../bsp/utils
INCS+= ../../../bsp/mcu/apollo4p/hal
INCS+= ../../../bsp/gpu/NemaGFX_SDK/NemaGFX
INCS+= ../../../bsp/devices
INCS+= ../../../bsp/gpu/NemaGFX_SDK/include/tsi/common
INCS+= ../../../bsp/gpu/NemaGFX_SDK/NemaDC
INCS+= ../../../bsp/gpu/NemaGFX_SDK/include/tsi/NemaVG
INCS+= ../../../bsp/gpu/NemaGFX_SDK/include/tsi/NemaGFX
INCS+= ../../../bsp/mcu/apollo4p
INCS+= ../../../bsp/CMSIS/AmbiqMicro/Include
INCS+= ../../../bsp/gpu/config/apollo4p_nemagfx
INCS+= ../../../bsp/CMSIS/ARM/Include
INCS+= ../../../LVGL_V8/src/gpu

all: directories $(CONFIG)/$(TARGET).bin

# Source Dependencies must be defined before they are used.
SRCS =  ../../../LVGL_V8/src/widgets/lv_textarea.c
SRCS += ../../../LVGL_V8/src/misc/lv_anim.c
SRCS += ../../../LVGL_V8/src/misc/lv_anim_timeline.c
SRCS += ../../../LVGL_V8/src/misc/lv_area.c
SRCS += ../../../LVGL_V8/src/misc/lv_async.c
SRCS += ../../../LVGL_V8/src/misc/lv_bidi.c
SRCS += ../../../LVGL_V8/src/misc/lv_color.c
SRCS += ../../../LVGL_V8/src/misc/lv_fs.c
SRCS += ../../../LVGL_V8/src/misc/lv_gc.c
SRCS += ../../../LVGL_V8/src/misc/lv_ll.c
SRCS += ../../../LVGL_V8/src/misc/lv_log.c
SRCS += ../../../LVGL_V8/src/misc/lv_math.c
SRCS += ../../../LVGL_V8/src/misc/lv_mem.c
SRCS += ../../../LVGL_V8/src/misc/lv_printf.c
SRCS += ../../../LVGL_V8/src/misc/lv_style.c
SRCS += ../../../LVGL_V8/src/misc/lv_style_gen.c
SRCS += ../../../LVGL_V8/src/misc/lv_templ.c
SRCS += ../../../LVGL_V8/src/misc/lv_timer.c
SRCS += ../../../LVGL_V8/src/misc/lv_tlsf.c
SRCS += ../../../LVGL_V8/src/misc/lv_txt.c
SRCS += ../../../LVGL_V8/src/misc/lv_txt_ap.c
SRCS += ../../../LVGL_V8/src/misc/lv_utils.c
SRCS += ../../../LVGL_V8/src/extra/widgets/calendar/lv_calendar.c
SRCS += ../../../LVGL_V8/src/extra/widgets/calendar/lv_calendar_header_arrow.c
SRCS += ../../../LVGL_V8/src/extra/widgets/calendar/lv_calendar_header_dropdown.c
SRCS += ../../../LVGL_V8/src/widgets/lv_btnmatrix.c
SRCS += ../../../LVGL_V8/src/extra/libs/qrcode/lv_qrcode.c
SRCS += ../../../LVGL_V8/src/extra/libs/qrcode/qrcodegen.c
SRCS += ../../../LVGL_V8/src/extra/libs/png/lodepng.c
SRCS += ../../../LVGL_V8/src/extra/libs/png/lv_png.c
SRCS += ../../../LVGL_V8/src/widgets/lv_objx_templ.c
SRCS += ../../../LVGL_V8/src/core/lv_disp.c
SRCS += ../../../LVGL_V8/src/core/lv_event.c
SRCS += ../../../LVGL_V8/src/core/lv_group.c
SRCS += ../../../LVGL_V8/src/core/lv_indev.c
SRCS += ../../../LVGL_V8/src/core/lv_indev_scroll.c
SRCS += ../../../LVGL_V8/src/core/lv_obj.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_class.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_draw.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_pos.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_scroll.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_style.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_style_gen.c
SRCS += ../../../LVGL_V8/src/core/lv_obj_tree.c
SRCS += ../../../LVGL_V8/src/core/lv_refr.c
SRCS += ../../../LVGL_V8/src/core/lv_theme.c
SRCS += ../../../LVGL_V8/src/extra/widgets/spinner/lv_spinner.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_arc.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_blend.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_img.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_label.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_line.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_mask.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_rect.c
SRCS += ../../../LVGL_V8/src/draw/lv_draw_triangle.c
SRCS += ../../../LVGL_V8/src/draw/lv_img_buf.c
SRCS += ../../../LVGL_V8/src/draw/lv_img_cache.c
SRCS += ../../../LVGL_V8/src/draw/lv_img_decoder.c
SRCS += ../../../LVGL_V8/src/widgets/lv_bar.c
SRCS += ../../../LVGL_V8/src/extra/themes/basic/lv_theme_basic.c
SRCS += ../../../LVGL_V8/src/extra/widgets/span/lv_span.c
SRCS += ../../../LVGL_V8/src/widgets/lv_switch.c
SRCS += ../../../LVGL_V8/src/extra/widgets/chart/lv_chart.c
SRCS += ../../../LVGL_V8/src/extra/themes/mono/lv_theme_mono.c
SRCS += ../../../LVGL_V8/src/widgets/lv_slider.c
SRCS += ../../../LVGL_V8/src/widgets/lv_roller.c
SRCS += ../../../LVGL_V8/src/extra/widgets/led/lv_led.c
SRCS += ../../../LVGL_V8/src/extra/widgets/colorwheel/lv_colorwheel.c
SRCS += ../../../LVGL_V8/src/extra/widgets/keyboard/lv_keyboard.c
SRCS += ../../../LVGL_V8/src/extra/widgets/meter/lv_meter.c
SRCS += ../../../LVGL_V8/src/extra/widgets/tabview/lv_tabview.c
SRCS += ../../../LVGL_V8/src/font/lv_font.c
SRCS += ../../../LVGL_V8/src/font/lv_font_dejavu_16_persian_hebrew.c
SRCS += ../../../LVGL_V8/src/font/lv_font_fmt_txt.c
SRCS += ../../../LVGL_V8/src/font/lv_font_loader.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_10.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_12.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_12_subpx.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_14.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_16.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_18.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_20.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_22.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_24.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_26.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_28.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_28_compressed.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_30.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_32.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_34.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_36.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_38.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_40.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_42.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_44.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_46.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_48.c
SRCS += ../../../LVGL_V8/src/font/lv_font_montserrat_8.c
SRCS += ../../../LVGL_V8/src/font/lv_font_simsun_16_cjk.c
SRCS += ../../../LVGL_V8/src/font/lv_font_unscii_16.c
SRCS += ../../../LVGL_V8/src/font/lv_font_unscii_8.c
SRCS += ../../../LVGL_V8/src/extra/themes/default/lv_theme_default.c
SRCS += ../../../LVGL_V8/src/widgets/lv_canvas.c
SRCS += ../../../LVGL_V8/src/widgets/lv_btn.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/portable/Keil6/AMapollo4/port.c
SRCS += ../../../LVGL_V8/src/extra/widgets/imgbtn/lv_imgbtn.c
SRCS += ../../../LVGL_V8/src/extra/libs/freetype/lv_freetype.c
SRCS += ../src/am_resources.c
SRCS += ../src/gui_task.c
SRCS += ../src/lvgl_test.c
SRCS += ../src/rtos.c
SRCS += ../../../LVGL_V8/src/widgets/lv_img.c
SRCS += ../../../LVGL_V8/src/widgets/lv_table.c
SRCS += ../../../LVGL_V8/src/extra/libs/fsdrv/lv_fs_fatfs.c
SRCS += ../../../LVGL_V8/src/extra/libs/fsdrv/lv_fs_posix.c
SRCS += ../../../LVGL_V8/src/extra/libs/fsdrv/lv_fs_stdio.c
SRCS += ../../../LVGL_V8/src/extra/libs/fsdrv/lv_fs_win32.c
SRCS += ../../../LVGL_V8/src/hal/lv_hal_disp.c
SRCS += ../../../LVGL_V8/src/hal/lv_hal_indev.c
SRCS += ../../../LVGL_V8/src/hal/lv_hal_tick.c
SRCS += ../../../LVGL_V8/src/extra/libs/gif/gifdec.c
SRCS += ../../../LVGL_V8/src/extra/libs/gif/lv_gif.c
SRCS += ../../../LVGL_V8/src/extra/layouts/flex/lv_flex.c
SRCS += ../../../LVGL_V8/src/extra/layouts/grid/lv_grid.c
SRCS += ../../../LVGL_V8/src/widgets/lv_checkbox.c
SRCS += ../../../LVGL_V8/src/extra/widgets/animimg/lv_animimg.c
SRCS += ../../../LVGL_V8/src/extra/widgets/tileview/lv_tileview.c
SRCS += ../../../LVGL_V8/src/extra/widgets/spinbox/lv_spinbox.c
SRCS += ../../../LVGL_V8/src/extra/widgets/win/lv_win.c
SRCS += ../../../LVGL_V8/src/extra/libs/rlottie/lv_rlottie.c
SRCS += ../../../LVGL_V8/src/widgets/lv_label.c
SRCS += ../../../LVGL_V8/src/widgets/lv_arc.c
SRCS += ../../../LVGL_V8/src/extra/widgets/list/lv_list.c
SRCS += ../../../LVGL_V8/src/extra/others/snapshot/lv_snapshot.c
SRCS += ../../../LVGL_V8/src/extra/libs/sjpg/lv_sjpg.c
SRCS += ../../../LVGL_V8/src/extra/libs/sjpg/tjpgd.c
SRCS += ../../../bsp/gpu/config/apollo4p_nemagfx/nema_dc_hal.c
SRCS += ../../../bsp/gpu/config/apollo4p_nemagfx/nema_hal.c
SRCS += ../../../LVGL_V8/src/extra/widgets/msgbox/lv_msgbox.c
SRCS += ../../../LVGL_V8/src/widgets/lv_dropdown.c
SRCS += ../../../LVGL_V8/src/extra/libs/bmp/lv_bmp.c
SRCS += ../../../LVGL_V8/src/widgets/lv_line.c
SRCS += ../../../bsp/utils/am_util_delay.c
SRCS += ../../../bsp/utils/am_util_stdio.c
# SRCS += ../../../bsp/utils/am_util_syscalls.c
SRCS += ../../../bsp/devices/am_devices_display_generic.c
SRCS += ../../../bsp/devices/am_devices_tma525.c
SRCS += ../../../lvgl_ambiq_porting/display_task_fake.c
SRCS += ../../../lvgl_ambiq_porting/display_task_one_and_partial_fb.c
SRCS += ../../../lvgl_ambiq_porting/display_task_one_fb.c
SRCS += ../../../lvgl_ambiq_porting/display_task_two_fb.c
SRCS += ../../../LVGL_V8/src/gpu/lv_ambiq_decoder.c
SRCS += ../../../lvgl_ambiq_porting/lv_ambiq_font_align.c
SRCS += ../../../lvgl_ambiq_porting/lv_ambiq_fs.c
SRCS += ../../../lvgl_ambiq_porting/lv_ambiq_misc.c
SRCS += ../../../lvgl_ambiq_porting/lv_ambiq_nema_hal.c
SRCS += ../../../lvgl_ambiq_porting/lv_ambiq_touch.c
SRCS += ../../../LVGL_V8/src/gpu/lv_gpu_ambiq_nema.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/portable/MemMang/heap_4.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/event_groups.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/list.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/queue.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/tasks.c
SRCS += ../../../modules/FreeRTOSv10.5.1/Source/timers.c

$(CONFIG)/$(TARGET).bin: $(LIBS) $(INCS) $(SRCS)
	UV5.exe -b -t "lvgl_test" lvgl_test.uvprojx -j0 || [ $$? -eq 1 ]

directories: $(CONFIG)

$(CONFIG):
	@mkdir -p $@

#### BUILD BEGIN ####

${bspincbuild}
${bspbuild}

${halincbuild}
${halbuild}

#### BUILD END ####

clean:
	@echo Cleaning...
	$(RM) -rf $(CONFIG)

endif
.PHONY: all clean directories

