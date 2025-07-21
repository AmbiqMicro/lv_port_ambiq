//*****************************************************************************
//
//! @file ambiq_mem.c
//!
//! @brief This file implemented the memory heap management functions.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
#include <string.h>
#include "tlsf.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "nema_regs.h"
#include "lvgl.h"
#include "am_mem.h"

#ifndef DTCM_POOL_SIZE
#define DTCM_POOL_SIZE 0x40000
#endif

#ifndef SSRAM_POLL_SIZE
#define SSRAM_POOL_SIZE 0x280000
#endif

#ifndef PSRAM_POOL_SIZE
#define PSRAM_POOL_SIZE 0x1F00000
#endif

#ifndef SSRAM_CACHE_ENABLE
#define SSRAM_CACHE_ENABLE 0
#endif

#ifndef PSRAM_CACHE_ENABLE
#define PSRAM_CACHE_ENABLE 0
#endif

#if SSRAM_CACHE_ENABLE
    #define SSRAM_BLOCK_ALIGN 32
#else
    #define SSRAM_BLOCK_ALIGN 4
#endif

#if PSRAM_CACHE_ENABLE
    #define PSRAM_BLOCK_ALIGN 32
#else
    #define PSRAM_BLOCK_ALIGN 4
#endif

#define SSRAM_MPU_REGION_NUMBER 6
#define PSRAM_MPU_REGION_NUMBER 7
#define NON_CACHEABLE_MPU_ATTRIBUTE_ENTRY 0

// typedef struct {
//     SemaphoreHandle_t mutex;
//     tlsf_t tlsf;
//     size_t cur_used;
//     size_t max_used;
//     size_t pool_size;
//     bool cacheable;
// } am_mem_control_t;

am_mem_control_t dtcm_heap = {
    .tlsf = NULL,
    .mutex = NULL,
    .cur_used = 0,
    .max_used = 0,
    .pool_size = DTCM_POOL_SIZE,
    .cacheable = false
};
am_mem_control_t ssram_heap = {
    .tlsf = NULL,
    .mutex = NULL,
    .cur_used = 0,
    .max_used = 0,
    .pool_size = SSRAM_POOL_SIZE,
    .cacheable = SSRAM_CACHE_ENABLE
};
am_mem_control_t psram_heap = {
    .tlsf = NULL,
    .mutex = NULL,
    .cur_used = 0,
    .max_used = 0,
    .pool_size = PSRAM_POOL_SIZE,
    .cacheable = PSRAM_CACHE_ENABLE
};


static uint8_t dtcm_pool[DTCM_POOL_SIZE] __attribute__((section(".bss"), aligned(4)));
static uint8_t ssram_pool[SSRAM_POOL_SIZE] __attribute__((section(".shared"), aligned(4)));
static uint8_t psram_pool[PSRAM_POOL_SIZE] __attribute__((section(".external"), aligned(4)));



/*
 * Set the graphics heap to non-cacheable,
 */
static void set_ssram_psram_heap_noncacheable(void)
{
#if (!SSRAM_CACHE_ENABLE) || (!PSRAM_CACHE_ENABLE)

    //
    // Disable MPU
    //
    am_hal_mpu_disable();

    //
    //MPU non-cacheable attribute setting.
    //OuterAttr and InnerAttr is set to 0b0100 to follow the requirement in ArmV8-m Architecture Reference Manual
    //
    am_hal_mpu_attr_t sMPUAttr = 
    {   
        .ui8AttrIndex = NON_CACHEABLE_MPU_ATTRIBUTE_ENTRY,
        .bNormalMem = true,
        .sOuterAttr = {
                        .bNonTransient = false, 
                        .bWriteBack = true, 
                        .bReadAllocate = false, 
                        .bWriteAllocate = false
                      },
        .sInnerAttr = {
                        .bNonTransient = false, 
                        .bWriteBack = true, 
                        .bReadAllocate = false, 
                        .bWriteAllocate = false
                      },
        .eDeviceAttr = 0,
    };

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);

#if !SSRAM_CACHE_ENABLE
    //
    // Set up the regions.
    //
    am_hal_mpu_region_config_t sMPUCfg_SSRAM = 
    { 
        .ui32RegionNumber = SSRAM_MPU_REGION_NUMBER,
        .ui32BaseAddress = (uintptr_t)ssram_pool,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = (uintptr_t)ssram_pool + SSRAM_POOL_SIZE - 1,
        .ui32AttrIndex = NON_CACHEABLE_MPU_ATTRIBUTE_ENTRY,
        .bEnable = true,
    };
    am_hal_mpu_region_configure(&sMPUCfg_SSRAM, 1);
#endif

#if !PSRAM_CACHE_ENABLE
    am_hal_mpu_region_config_t sMPUCfg_PSRAM = 
    { 
        .ui32RegionNumber = PSRAM_MPU_REGION_NUMBER,
        .ui32BaseAddress = (uintptr_t)psram_pool,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = (uintptr_t)psram_pool + PSRAM_POOL_SIZE - 1,
        .ui32AttrIndex = NON_CACHEABLE_MPU_ATTRIBUTE_ENTRY,
        .bEnable = true,
    };
    am_hal_mpu_region_configure(&sMPUCfg_PSRAM, 1);
#endif

    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF. 
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

#endif
}

static void _am_mem_heap_init(am_mem_control_t *heap, void *pool, size_t pool_size)
{
    heap->tlsf = tlsf_create_with_pool(pool, pool_size);
    if (heap->tlsf == NULL)
    {
        am_util_stdio_printf("TLSF memory pool creation failed!\n");
        while(1);
    }

    /*Init mutex for exclusive access*/
    heap->mutex = xSemaphoreCreateMutex();
    if (heap->mutex == NULL)
    {
        am_util_stdio_printf("TLSF mutex creation failed!\n");
        while(1);
    }

    heap->cur_used = 0;
    heap->max_used = 0;
    heap->pool_size = pool_size;
    heap->start_addr = (uint32_t)pool;
}

void *am_mem_heap_malloc(am_mem_control_t *heap, size_t size)
{
    void *p = NULL;

    if (size == 0)
    {
        return NULL;
    }

    if (xSemaphoreTake(heap->mutex, portMAX_DELAY) == pdTRUE)
    {
        p = tlsf_malloc(heap->tlsf, size);
        if (p)
        {
            heap->cur_used += tlsf_block_size(p);
            heap->max_used = heap->cur_used > heap->max_used ? heap->cur_used : heap->max_used;
        }
        xSemaphoreGive(heap->mutex);
    }

    return p;
}

void *am_mem_heap_malloc_align(am_mem_control_t *heap, size_t size, size_t align)
{
    void *p = NULL;

    if (size == 0)
    {
        return NULL;
    }

    if (xSemaphoreTake(heap->mutex, portMAX_DELAY) == pdTRUE)
    {
        p = tlsf_memalign(heap->tlsf, align, size);
        if (p)
        {
            heap->cur_used += tlsf_block_size(p);
            heap->max_used = heap->cur_used > heap->max_used ? heap->cur_used : heap->max_used;
        }
        xSemaphoreGive(heap->mutex);
    }

    return p;
}

void *am_mem_heap_realloc(am_mem_control_t *heap, void * p, size_t new_size)
{
    void * p_new = NULL;

    if (xSemaphoreTake(heap->mutex, portMAX_DELAY) == pdTRUE)
    {

        size_t old_size = tlsf_block_size(p);
        p_new = tlsf_realloc(heap->tlsf, p, new_size);

        if(p_new) {
            heap->cur_used -= old_size;
            heap->cur_used += tlsf_block_size(p_new);
            heap->max_used = heap->cur_used > heap->max_used ? heap->cur_used : heap->max_used;
        }
        xSemaphoreGive(heap->mutex);
    }

    return p_new;


}

void am_mem_heap_free(am_mem_control_t *heap, void *p)
{
    if (p == NULL)
    {
        return;
    }

    uint32_t start_addr_pool = heap->start_addr;
    uint32_t end_addr_pool = heap->start_addr + heap->pool_size;
    uint32_t ptr_uint = (uint32_t)p;

    LV_ASSERT_MSG((ptr_uint > start_addr_pool) && (ptr_uint < end_addr_pool), "Pointer address out of heap bounds");

    if (xSemaphoreTake(heap->mutex, portMAX_DELAY) == pdTRUE)
    {
        size_t size = tlsf_block_size(p);
        tlsf_free(heap->tlsf, p);
        if (heap->cur_used > size)
        {
            heap->cur_used -= size;
        }
        else
        {
            heap->cur_used = 0;
        }
        xSemaphoreGive(heap->mutex);
    }
}

void am_mem_heap_init(void)
{
    _am_mem_heap_init(&dtcm_heap, dtcm_pool, DTCM_POOL_SIZE);
    _am_mem_heap_init(&ssram_heap, ssram_pool, SSRAM_POOL_SIZE);
    _am_mem_heap_init(&psram_heap, psram_pool, PSRAM_POOL_SIZE);

    /* Config MPU for non-cacheable memory*/
    set_ssram_psram_heap_noncacheable();

#if SSRAM_CACHE_ENABLE
    ssram_heap.cacheable = true;
#else
    ssram_heap.cacheable = false;
#endif

#if PSRAM_CACHE_ENABLE
    psram_heap.cacheable = true;
#else
    psram_heap.cacheable = false;
#endif
}

static void am_mem_walker(void * ptr, size_t size, int used, void * user)
{
    LV_UNUSED(ptr);

    am_mem_monitor_t * mon_p = user;
    mon_p->total_size += size;
    if(used) {
        mon_p->used_cnt++;
    }
    else {
        mon_p->free_cnt++;
        mon_p->free_size += size;
        if(size > mon_p->free_biggest_size)
            mon_p->free_biggest_size = size;
    }
}

void am_mem_heap_monitor(am_mem_control_t *heap, am_mem_monitor_t * mon_p)
{
    /*Init the data*/
    memset(mon_p, 0, sizeof(am_mem_monitor_t));

    pool_t pool = tlsf_get_pool(heap->tlsf);

    if (xSemaphoreTake(heap->mutex, portMAX_DELAY) == pdTRUE)
    {
        tlsf_walk_pool(pool, am_mem_walker, mon_p);
        xSemaphoreGive(heap->mutex);
    }

    mon_p->used_pct = 100 - (uint64_t)100U * mon_p->free_size / mon_p->total_size;
    if(mon_p->free_size > 0) {
        mon_p->frag_pct = (uint64_t)mon_p->free_biggest_size * 100U / mon_p->free_size;
        mon_p->frag_pct = 100 - mon_p->frag_pct;
    }
    else {
        mon_p->frag_pct = 0; /*no fragmentation if all the RAM is used*/
    }

    mon_p->max_used = heap->max_used;
}

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
void lv_mem_init(void)
{
    /*Nothing need to do, we will call am_mem_init() at main function, before lv_init().*/
}

void lv_mem_deinit(void)
{
    /*We can't release dtcm_heap, because other module may still using it now*/
}

void * lv_malloc_core(size_t size)
{
#if LV_USE_DRAW_AMBIQ
    return am_mem_heap_malloc(&dtcm_heap, size);
#else
    return am_mem_heap_malloc(&ssram_heap, size);
#endif
}

void * lv_realloc_core(void * p, size_t new_size)
{
#if LV_USE_DRAW_AMBIQ
    return am_mem_heap_realloc(&dtcm_heap ,p, new_size);
#else
    return am_mem_heap_realloc(&ssram_heap ,p, new_size);
#endif
}

void lv_free_core(void * p)
{
#if LV_USE_DRAW_AMBIQ
    am_mem_heap_free(&dtcm_heap, p);
#else
    am_mem_heap_free(&ssram_heap, p);
#endif
}

void lv_mem_monitor_core(lv_mem_monitor_t * mon_p)
{
    am_mem_monitor_t* mon_p_new = (am_mem_monitor_t*)mon_p;

#if LV_USE_DRAW_AMBIQ
    am_mem_heap_monitor(&dtcm_heap, mon_p_new);
#else
    am_mem_heap_monitor(&ssram_heap, mon_p_new);
#endif
}

lv_result_t lv_mem_test_core(void)
{
    return LV_RESULT_OK;
}
#endif


