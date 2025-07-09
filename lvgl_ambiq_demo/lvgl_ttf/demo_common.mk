TARGET := lvgl_ttf

DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESX=464
DEFINES+= -DLV_AMBIQ_DISPLAY_BUFFER_RESY=464
DEFINES+= -DCPU_RUN_IN_HP_MODE
DEFINES+= -DGPU_RUN_IN_HP_MODE
DEFINES+= -DLV_AMBIQ_USE_DIRECT_MODE=1
DEFINES+= -DLV_AMBIQ_DRAW_BUFFER_RATIO=1
DEFINES+= -DSSRAM_CACHE_ENABLE=1

# lvgl_ttf
INCLUDES+= -I../src
VPATH+=:../src

SRC += rtos.c
SRC += am_resources.c
SRC += gui_task.c
SRC += lvgl_ttf.c
SRC += am_ftsystem.c

# FreeType custom configuration header file
DEFINES += -DFT2_BUILD_LIBRARY
DEFINES += -DFT_CONFIG_MODULES_H=\"../../../../LVGL/src/libs/freetype/ftmodule.h\"
DEFINES += -DFT_CONFIG_OPTIONS_H=\"../../../../LVGL/src/libs/freetype/ftoption.h\"

# FreeType include path
INCLUDES += -I$(FREETYPE_PATH)
INCLUDES += -I$(FREETYPE_PATH)/include

VPATH+=:$(FREETYPE_PATH)/src/base/
VPATH+=:$(FREETYPE_PATH)/src/cache/
VPATH+=:$(FREETYPE_PATH)/src/gzip/
VPATH+=:$(FREETYPE_PATH)/src/sfnt/
VPATH+=:$(FREETYPE_PATH)/src/smooth/
VPATH+=:$(FREETYPE_PATH)/src/truetype/

# FreeType C source file
SRC += ftbase.c
SRC += ftbitmap.c
SRC += ftdebug.c
SRC += ftglyph.c
SRC += ftinit.c
SRC += ftstroke.c
SRC += ftcache.c
SRC += ftgzip.c
SRC += sfnt.c
SRC += smooth.c
SRC += truetype.c