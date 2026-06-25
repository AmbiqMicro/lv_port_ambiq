#******************************************************************************
#
# demo_lite.mk - Reduced smartwatch demo for Apollo510dL (2MB MRAM)
#
# Keeps digital/analog watch faces; optional SpO2, weather, and call screens.
# Drops other health screens. Wallpaper frames preload into PSRAM at startup.
#
#******************************************************************************

DEFINES += -DAPOLLO510DL_LITE
DEFINES += -DAPOLLO510DL_LITE_BLOOD_OXY
DEFINES += -DAPOLLO510DL_LITE_WEATHER
DEFINES += -DAPOLLO510DL_LITE_CALL

LITE_SCREEN_EXCLUDE := \
	../src/gui/screens/ui_ecg.c \
	../src/gui/screens/ui_blood_pressure.c \
	../src/gui/screens/ui_measuing.c

LITE_COMP_EXCLUDE :=

LITE_IMAGE_EXCLUDE := \
	../src/gui/images/ui_img_bg1_png.c \
	../src/gui/images/ui_img_samatha_png.c \
	../src/gui/images/ui_img_s8_png.c \
	../src/gui/images/ui_img_s9_png.c \
	../src/gui/images/ui_img_x_png.c \
	../src/gui/images/ui_img_wave1_png.c \
	../src/gui/images/ui_img_wave2_png.c

LITE_LVGL_EXCLUDE := \
	$(LVGL_PATH)/src/widgets/chart/lv_chart.c \
	$(LVGL_PATH)/src/widgets/spinner/lv_spinner.c

LVGL_CSRCS := $(filter-out $(LITE_SCREEN_EXCLUDE) $(LITE_COMP_EXCLUDE) $(LITE_IMAGE_EXCLUDE) $(LITE_LVGL_EXCLUDE),$(LVGL_CSRCS))
