// LVGL version: 9.2.2
// Project name: Smartwatch

#ifndef _UI_COMP_WALLPAPER_H
#define _UI_COMP_WALLPAPER_H

#include "../ui.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef APOLLO510DL_LITE
void samatha_texture_init(void);
#endif
void wallpaper_texture_init(void);
lv_obj_t *ui_dynamic_wallpaper_create(lv_obj_t *comp_parent);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
