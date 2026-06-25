#include "../ui.h"

#include "lvgl_private.h"
#define BIN_PATH_PREFIX "E:wallpaper"
#define BIN_PATH_SUFFIX ".bin"
#ifndef WALLPAPER_NUMBER
#define WALLPAPER_NUMBER  63
#endif
#define WALLPAPER_WIDTH   397
#define WALLPAPER_HEIGHT  370
#define WALLPAPER_FORMAT  LV_COLOR_FORMAT_NATIVE

// component dynamic wallpaper
lv_img_dsc_t* img_wallpaper[WALLPAPER_NUMBER];
lv_img_dsc_t  img_wallpaper_arry_default[WALLPAPER_NUMBER];

//*****************************************************************************
//
// Read files from EMMC
//
//*****************************************************************************
lv_fs_res_t
load_emmc_file(char *str, uint8_t *buf, uint32_t len)
{
    lv_fs_file_t f;
    lv_fs_res_t res;
    uint32_t read_cnt = 0;

    res = lv_fs_open(&f, str, LV_FS_MODE_RD);
    if(res == LV_FS_RES_OK)
    {
         res = lv_fs_read(&f, buf, len, &read_cnt);
         if((res != LV_FS_RES_OK) || (read_cnt == 0))
         {
           LV_LOG_ERROR("File Read Error!\n");
           return res;
         }

         res = lv_fs_close(&f);
         if(res != LV_FS_RES_OK)
         {
           LV_LOG_ERROR("Fail to close file\n");
         }
    }
    else
    {
        LV_LOG_ERROR("Fail to open file!\n");
    }
    return res;
}

void wallpaper_texture_init(void)
{
    char path[64];
    uint32_t i;
    for(i = 0; i < WALLPAPER_NUMBER; i++)
    {
        lv_draw_buf_t* buf = lv_draw_buf_create_ex(&LV_GLOBAL_DEFAULT()->image_cache_draw_buf_handlers, WALLPAPER_WIDTH, WALLPAPER_HEIGHT, WALLPAPER_FORMAT, 0);

        lv_draw_buf_to_image(buf, &img_wallpaper_arry_default[i]);
        lv_snprintf(path, sizeof(path), "%s%d%s", BIN_PATH_PREFIX, i*2, BIN_PATH_SUFFIX);
        load_emmc_file(path, (void*)img_wallpaper_arry_default[i].data, img_wallpaper_arry_default[i].data_size);
        img_wallpaper[i] = &img_wallpaper_arry_default[i];
    }
}

lv_obj_t *ui_dynamic_wallpaper_create(lv_obj_t *comp_parent)
{
    lv_obj_t* lv_anim = lv_animimg_create(comp_parent);
    lv_obj_align(lv_anim, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(lv_anim, 397, 370);
    lv_obj_set_style_bg_color(lv_anim, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(lv_anim, LV_OPA_COVER, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_remove_flag(lv_anim, LV_OBJ_FLAG_SCROLLABLE );    /// Flags
    lv_animimg_set_src(lv_anim, (const void **)img_wallpaper, WALLPAPER_NUMBER);
    lv_animimg_set_duration(lv_anim, 3500);
    lv_animimg_set_repeat_count(lv_anim, LV_ANIM_REPEAT_INFINITE);
    lv_obj_move_background(lv_anim);
    lv_animimg_start(lv_anim);
    return lv_anim;
}
