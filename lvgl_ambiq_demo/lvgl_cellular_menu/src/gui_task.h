//*****************************************************************************
//
//! @file gui_task.h
//!
//! @brief Functions and variables related to the gui task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef GUI_TASK_H
#define GUI_TASK_H

#define ANIMATION

//*****************************************************************************
//
// Struct definitions.
//
//*****************************************************************************
#define PI 3.141593

#define STEP_MAX 1024
#define SPEED_MAX 100

// panel size is 454*454
#define DIS_CENTER_X 0
#define DIS_CENTER_Y 0
#define ICON_MAX 19
#define LAYOUT_ICON_MAX 61

#define ICON_D 128

#define CYC_0_FACTOR 100
#define CYC_1_FACTOR 90
#define CYC_2_FACTOR_1 80
#define CYC_2_FACTOR_2 80
#define CYC_3_FACTOR 72
#define CYC_4_FACTOR 64

#define ICON_REMOTE 200
#define ICON_ZOOM_MIN 128
#define ICON_ZOOM_MAX 256

#define R_CYCLE_FACTOR_1 1.10
#define R_CYCLE_FACTOR_2 0.90
#define R_CYCLE_FACTOR_3 1.00
#define R_CYCLE_FACTOR_4 1.00

#define PROXIMITY 1 //Proximity

#define ROOT3 (float)(1.732051)
#define ROOT2 (float)(1.414214)

typedef struct
{
    int ac_x;        //Axial Coordinates x
    int ac_y;        //Axial Coordinates y
    int ac_z;        //Axial Coordinates z
    int cycle;        //Currentlly belong to which cycle
    float origin_x;    //Plane Coordinates x
    float origin_y;    //Plane Coordinates y
    float distance;    //distance from the center of the panel
    int r;            //Current icon radius
    lv_obj_t * icon;
} icon_t;

typedef struct
{
    icon_t Icon[ICON_MAX];
} icons_t;

typedef struct
{
    icon_t Icon[LAYOUT_ICON_MAX];
} icon_layout_t;

typedef struct
{
    float direction;
    float speed;
    float distance;
    int steps;
} vector_t;

typedef struct
{
    float inc_x;
    float inc_y;
} step_t;

typedef struct
{
    int total_step;
    int flag;
    int step_counter;
    step_t step[STEP_MAX];
} steps_t;

typedef struct
{
    float x;
    float y;
} coordinate_t;

typedef struct
{
    float a;
    float b;
    float c;
} factor_t;

typedef struct
{
    float x;
    float y;
    int r;
} rb_icon_t; //rebuild_icon_t

typedef struct
{
    float min_d_0;
    float min_d_1;
    float min_d_2;
    float min_d_3;
    int cii_0;  //center_icon_index_0;
    int cii_1;  //center_icon_index_1;
    int cii_2;  //center_icon_index_2;
    int cii_3;  //center_icon_index_3;

    float S, Sa, Sb, Sc, Ssum;
    float sum_factor;
} log_t; //Plane coordinate system

//*****************************************************************************
//
// Gui task handle.
//
//*****************************************************************************
extern TaskHandle_t GuiTaskHandle;

//*****************************************************************************
//
// Semaphores
//
//*****************************************************************************
extern SemaphoreHandle_t lvgl_mutex;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void GuiTask(void *pvParameters);

#endif // GUI_TASK_H
