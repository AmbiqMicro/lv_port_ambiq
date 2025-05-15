//*****************************************************************************
//
//! @file lv_ambiq_fs.c
//!
//! @brief Ambiq fs porting.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

/*********************
 *      INCLUDES
 *********************/
#include "lv_ambiq_fs.h"
#include "lvgl.h"
#include "ff.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static void * fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);

static void * fs_dir_open (lv_fs_drv_t * drv, const char *path);
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * rddir_p, char *fn);
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * rddir_p);

/**********************
 *  STATIC VARIABLES
 **********************/
FATFS lv_FatFs;
char lv_Path[4];
uint8_t lv_work_buf[FF_MAX_SS];


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_ambiq_fs_init(void)
{
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    /*Add a simple drive to open images*/
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter = 'E';
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_read_cb = fs_dir_read;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init(void)
{
    /*for FatFS initialize the EMMC card and FatFS itself*/
    FRESULT res;

    res = f_mount(&lv_FatFs, (TCHAR const*)lv_Path, 1);
    if(res != FR_OK)
    {
        res = f_mkfs((TCHAR const*)lv_Path, 0, lv_work_buf, sizeof(lv_work_buf));
    }
}

/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          a file descriptor or NULL on error
 */
static void * fs_open (lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    FRESULT res;
    uint8_t fs_mode = 0;

    if(mode == LV_FS_MODE_WR)
    {
        /*Open a file for write*/
        fs_mode = FA_OPEN_ALWAYS | FA_WRITE;
    }
    else if(mode == LV_FS_MODE_RD)
    {
        /*Open a file for read*/
        fs_mode = FA_READ;
    }
    else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
    {
        /*Open a file for read and write*/
        fs_mode = FA_WRITE | FA_READ | FA_OPEN_ALWAYS;
    }
    else
    {
        return NULL;
    }

    FIL * lv_file = lv_malloc_zeroed(sizeof(FIL));
    if(lv_file == NULL)
    {
      return NULL;
    }

    res = f_open(lv_file, path, fs_mode);
	if (res != FR_OK)
    {
        lv_free(lv_file);
    	return NULL;
  	}

    return lv_file;
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with lv_ufs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p)
{
    lv_fs_res_t res;

    if (f_close((FIL*)file_p) == FR_OK)
    {
        res = LV_FS_RES_OK;
    }
    else
    {
        res = LV_FS_RES_UNKNOWN;
    }

    lv_free(file_p);

    return res;
}

/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    lv_fs_res_t res;

    if(f_read((FIL*)file_p, buf, btr, (UINT*)br) == FR_OK)
    {
        res = LV_FS_RES_OK;
    }
    else
    {
        res = LV_FS_RES_UNKNOWN;
    }

    return res;
}

/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btr       Bytes To Write
 * @param br        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    lv_fs_res_t res;

    if(f_write((FIL*)file_p, buf, btw, (UINT*)bw) == FR_OK)
    {
        res = LV_FS_RES_OK;
    }
    else
    {
        res = LV_FS_RES_UNKNOWN;
    }

    return res;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with lv_ufs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    lv_fs_res_t res;
    FRESULT f_res;

    switch(whence)
    {
        case LV_FS_SEEK_SET:
            f_res = f_lseek(file_p, pos);
            break;
        case LV_FS_SEEK_CUR:
            f_res = f_lseek(file_p, f_tell((FIL*)file_p) + pos);
            break;
        case LV_FS_SEEK_END:
            f_res = f_lseek(file_p, f_size((FIL*)file_p) + pos);
            break;
        default:
            f_res = FR_INVALID_PARAMETER;
            break;
    }

    if(f_res == FR_OK)
    {
        res = LV_FS_RES_OK;
    }
    else
    {
        res = LV_FS_RES_UNKNOWN;
    }

    return res;
}
/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param pos_p     pointer to to store the result
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
	FSIZE_t t = f_tell((FIL*)file_p);
	*pos_p = t;

    return LV_FS_RES_OK;
}

/**
 * Initialize a 'lv_fs_dir_t' variable for directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void * fs_dir_open (lv_fs_drv_t * drv, const char *path)
{
    DIR * lv_dir = lv_malloc_zeroed(sizeof(DIR));
    if(lv_dir == NULL)
    {
      return NULL;
    }

	if(f_opendir(lv_dir , (TCHAR*)path) == FR_OK)
    {
        return lv_dir ;
    }
    else
    {
        lv_free(lv_dir);
        return NULL;
    }
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @param fn        pointer to a buffer to store the filename
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * rddir_p, char *fn)
{
    lv_fs_res_t res;
    FILINFO fno;

    if(f_readdir((DIR*)rddir_p, &fno) == FR_OK)
    {
        strcpy(fn,(char*)fno.fsize);
        res = LV_FS_RES_OK;
    }
    else
    {
        res = LV_FS_RES_UNKNOWN;
    }

    return res;
}

/**
 * Close the directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * rddir_p)
{
    lv_fs_res_t res;

    if(f_closedir((DIR*)rddir_p) == FR_OK)
    {
        res = LV_FS_RES_OK;
    }
    else
    {
        res = LV_FS_RES_UNKNOWN;
    }

    lv_free(rddir_p);
    return res;
}