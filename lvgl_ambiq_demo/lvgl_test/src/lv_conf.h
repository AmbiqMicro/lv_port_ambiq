#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*suppress warnings*/
#if defined(__IAR_SYSTEMS_ICC__)
#pragma diag_suppress = Pa089
#pragma diag_suppress = Pe188
#pragma diag_suppress = Pe236
#pragma diag_suppress = Pe111
#pragma diag_suppress = Pe550
#pragma diag_suppress = Pe177
#elif defined(__ARMCC_VERSION)
#pragma diag_suppress 111
#pragma diag_suppress 188
#pragma diag_suppress 68
#pragma diag_suppress 1295
#pragma diag_suppress 177
#pragma diag_suppress 546
#endif

/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `lv_tick_inc()`)*/
#define LV_TICK_CUSTOM     1
#define LV_TICK_CUSTOM_INCLUDE  "lvgl_test.h"         /*Header for the system time function*/
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (xTaskGetTickCount())     /*Expression evaluating to current system time in ms*/

/*Enable the log module*/
#define LV_USE_LOG      1
#define LV_LOG_LEVEL    LV_LOG_LEVEL_ERROR

/*1: Show CPU usage and FPS count in the right bottom corner*/
#define LV_USE_PERF_MONITOR 1
#define LV_GET_CPU_OCCUPATION_INCLUDE "rtos.h"         /*Header for the getCpuOccupationRate API*/
#define LV_GET_CPU_OCCUPATION (getCpuOccupationRate())    /*get cpu occupation rate api, uint32_t getCpuOccupationRate(void)*/


#define LV_MEM_CUSTOM 0

/*Size of the memory available for TCM heap in bytes (>= 2kB)*/
#  define LV_MEM_TCM_SIZE (32UL * 1024UL)          /*[bytes]*/
/*Set an address for the TCM heap.*/
#  define LV_MEM_TCM_ADR (0x10060000UL - 32UL * 1024UL)   /*0: unused*/

/*Size of the memory available for SSRAM heap in bytes*/
#  define LV_MEM_SSRAM_SIZE (NONTCM_MAX_SIZE - 0x40UL)          /*[bytes]*/
/*Set an address for the SSRAM heap.*/
#  define LV_MEM_SSRAM_ADR (SSRAM0_BASEADDR + 0x40UL)     /*0: unused*/

/*Size of the memory available for PSRAM heap in bytes*/
#  define LV_MEM_PSRAM_SIZE (16UL * 1024UL * 1024UL)         /*[bytes]*/
/*Set an address for the PSRAM heap*/
#  define LV_MEM_PSRAM_ADR (0x14000000UL)    /*0: unused*/

#ifdef __GNUC__
/*Complier prefix for a big array declaration in RAM*/
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY  __attribute__((section(".shared")))
/*Prefix variables that are used in GPU accelerated operations, often these need to be placed in RAM sections that are DMA accessible*/
#define LV_ATTRIBUTE_DMA __attribute__((section(".shared")))
/*Export integer constant to binding. This macro is used with constants in the form of LV_<CONST> that
 *should also appear on LVGL binding API such as Micropython.*/
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*The default value just prevents GCC warning*/
#elif defined(__ARMCC_VERSION)
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY  __attribute__((section("SHARED_RW"))) __attribute__((used))
#define LV_ATTRIBUTE_DMA __attribute__((section("SHARED_RW"))) __attribute__((used))
#define LV_EXPORT_CONST_INT(int_value)
#elif defined(__IAR_SYSTEMS_ICC__)
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY  __attribute__((section("SHARED_RW"))) __root
#define LV_ATTRIBUTE_DMA __attribute__((section("SHARED_RW"))) __root
#define LV_EXPORT_CONST_INT(int_value)
#endif

#endif /*LV_CONF_H*/