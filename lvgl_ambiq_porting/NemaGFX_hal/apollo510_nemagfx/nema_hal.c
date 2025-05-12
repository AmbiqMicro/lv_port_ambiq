/*******************************************************************************
 * Copyright (c) 2022 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/
#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"
#include "nema_hal.h"
#include "nema_regs.h"
#include "nema_ringbuffer.h"
#include "nema_error.h"
#include "nema_vg_context.h"
#include "am_mcu_apollo.h"
#include "am_hal_global.h"
#include "am_util_delay.h"
#include "am_mem.h"
#include "am_mcu_apollo.h"

#include <stdlib.h>

#define NEMA_BASEADDR       GPU_BASE

// IRQ number
#define NEMA_IRQ            ((IRQn_Type)28U)

// MAX pending CL in the core ring buffer
#ifndef MAX_PENDING_CL
#define MAX_PENDING_CL (100UL)
#endif

#if (MAX_PENDING_CL < 10)
#error "max pending CL must be bigger than 10"
#endif

#include "semphr.h"
static SemaphoreHandle_t xSemaphore = NULL;

static nema_gfx_interrupt_callback nemagfx_cb = NULL;
static const uintptr_t nema_regs = (uintptr_t) NEMA_BASEADDR;
static TaskHandle_t xHandlingTask = 0;
static volatile int last_cl_id = -1;
static nema_ringbuffer_t ring_buffer_str = {{0}};

static void prvNemaInterruptHandler( void *pvUnused )
{

    int current_cl_id = (int)nema_reg_read(NEMA_CLID);
    int previous_cl_id;

    do
    {
        /* Clear the interrupt */
        nema_reg_write(NEMA_INTERRUPT, 0);

        /* Clear pending register*/
        NVIC_ClearPendingIRQ(NEMA_IRQ);

        previous_cl_id = current_cl_id;

        /* Read again and compare with the previous one to prevent 
         * a corner case where a new gpu_irq is triggered after reading the NEMA_CLID register 
         * and before cleaning the NEMA_INTERRUPT register. */
        current_cl_id = (int)nema_reg_read(NEMA_CLID);
    }while(current_cl_id != previous_cl_id);

    /* Public the last_cl_id */
    last_cl_id = current_cl_id;

    BaseType_t xHigherPriorityTaskWoken;

    xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

    /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context switch
    should be performed to ensure the interrupt returns directly to the highest
    priority task.  The macro used for this purpose is dependent on the port in
    use and may be called portEND_SWITCHING_ISR(). */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    if (nemagfx_cb != NULL)
    {
        nemagfx_cb(last_cl_id);
    }
}

//*****************************************************************************
//
//! @brief GFX interrupt callback initialize function
//!
//! @param  fnGFXCallback                - GFX interrupt callback function
//!
//! this function hooks the Nema GFX GPU interrupt with a callback function.
//!
//! The fisrt paramter to the callback is a volatile int containing the ID of
//! the last serviced command list. This is useful for quickly responding
//! to the completion of an issued CL.
//!
//! @return None.
//
//*****************************************************************************
void
nemagfx_set_interrupt_callback(nema_gfx_interrupt_callback fnGFXCallback)
{
    nemagfx_cb = fnGFXCallback;
}

void am_gpu_isr()
{
    // Nema GPU interrupt handler
    prvNemaInterruptHandler(NULL);
}

int32_t nema_sys_init (void)
{
    nema_reg_write(NEMA_INTERRUPT, 0);

    /* Install Interrupt Handler */
    NVIC_SetPriority(NEMA_IRQ, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(NEMA_IRQ);

        if (xSemaphore == NULL)
        {
            xSemaphore = xSemaphoreCreateBinary();
        }


    //ring_buffer_str.bo may be already allocated
    if ( ring_buffer_str.bo.base_phys == 0U )
    {
        //allocate ring_buffer memory
        ring_buffer_str.bo = nema_buffer_create_pool(NEMA_MEM_POOL_CL, (MAX_PENDING_CL*12 + 16)*4);
        (void)nema_buffer_map(&ring_buffer_str.bo);
    }

    //Initialize Ring BUffer
    int ret = nema_rb_init(&ring_buffer_str, 1);
    if (ret) {
        // am_util_stdio_printf("nema_rb_init FAILED\n");
        return ret;
    }

    last_cl_id = -1;

    return 0;
}

int nema_wait_irq (void)
{
    /* Wait for the interrupt */
    BaseType_t xResult;

    TickType_t block_ms = pdMS_TO_TICKS(1000);
    xResult = xSemaphoreTake( xSemaphore, block_ms );

    if(xResult != pdTRUE)
    {
        while(1);
    }


    return (int)xResult;
}

int nema_wait_irq_cl (int cl_id)
{
    while ( last_cl_id < cl_id) {
        int ret = nema_wait_irq();
        (void)ret;
    }

    return 0;
}

uint32_t nema_reg_read (uint32_t reg)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(nema_regs + reg);
    return *ptr;
}

void nema_reg_write (uint32_t reg,uint32_t value)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(nema_regs + reg);
    *ptr = value;
}

/**
 * @brief Retrieve the memory heap based on the specified memory pool.
 *
 * This function returns a pointer to the memory heap corresponding to the given memory pool identifier.
 *
 * @param pool The memory pool identifier. Possible values are:
 *             - NEMA_MEM_POOL_ASSETS: Returns the psram_heap.
 *             - NEMA_MEM_POOL_CL: Returns the ssram_heap.
 *             - NEMA_MEM_POOL_FB: Returns the ssram_heap.
 *             - NEMA_MEM_POOL_CLIPPED_PATH: Returns the dtcm_heap.
 *             - Any other value: Returns the ssram_heap.
 *
 * @return A pointer to the corresponding memory heap.
 */
static am_mem_control_t* get_heap(int pool)
{
    switch (pool)
    {
        case NEMA_MEM_POOL_ASSETS:
            return &psram_heap;
        case NEMA_MEM_POOL_CL:
        case NEMA_MEM_POOL_FB:
            return &ssram_heap;
        case NEMA_MEM_POOL_CLIPPED_PATH:
            return &dtcm_heap;
        default:
            return &ssram_heap;
    }
}

nema_buffer_t nema_buffer_create (int size)
{
    return nema_buffer_create_pool(NEMA_MEM_POOL_CL, size);
}

nema_buffer_t nema_buffer_create_pool (int pool, int size)
{

    nema_buffer_t bo;
    void* ptr = NULL;

    am_mem_control_t* heap = get_heap(pool);

    if (heap->cacheable)
    {
        size = (size + 31) & ~31; // Align size to the next multiple of 32
        ptr = am_mem_heap_malloc_align(heap, size, 32);
    }
    else
    {
        if(pool == NEMA_MEM_POOL_FB || pool == NEMA_MEM_POOL_ASSETS || pool == NEMA_MEM_POOL_CL)
        {
            ptr = am_mem_heap_malloc_align(heap, size, 8);
        }
        else
        {
            ptr = am_mem_heap_malloc(heap, size);
        }
    }

    bo.base_virt = ptr;
    bo.base_phys = (uintptr_t)ptr;
    bo.size      = size;
    bo.fd        = pool;

    return bo;
}

void *nema_buffer_map (nema_buffer_t * bo)
{
    return bo->base_virt;
}

void nema_buffer_unmap (nema_buffer_t * bo)
{

}

void nema_buffer_destroy (nema_buffer_t * bo)
{

    am_mem_control_t* heap = get_heap(bo->fd);

    am_mem_heap_free(heap, bo->base_virt);


    bo->base_virt = (void *)NULL;
    bo->base_phys = 0;
    bo->size      = 0;
    bo->fd        = -1;
}

uintptr_t nema_buffer_phys (nema_buffer_t * bo)
{
    return bo->base_phys;
}

void nema_buffer_flush(nema_buffer_t * bo)
{
    am_mem_control_t* heap = get_heap(bo->fd);
    bool need_flush = (heap->cacheable) ? true : false;

    if(need_flush)
    {
        am_hal_cachectrl_range_t Range;
        Range.ui32Size = bo->size;
        Range.ui32StartAddr = bo->base_phys;
        am_hal_cachectrl_dcache_clean(&Range);
    }


    __DSB();
}

void nema_buffer_invalidate(nema_buffer_t * bo)
{

    am_mem_control_t* heap = get_heap(bo->fd);
    bool need_flush = (heap->cacheable) ? true : false;

    if(need_flush)
    {
        am_hal_cachectrl_range_t Range;
        Range.ui32Size = bo->size;
        Range.ui32StartAddr = bo->base_phys;
        am_hal_cachectrl_dcache_invalidate(&Range, false);
    }
}

bool nema_buffer_is_within_pool(int pool, uint32_t buf_start, uint32_t buf_length)
{
    am_mem_control_t* heap = get_heap(pool);

    uint32_t start_addr_pool = heap->start_addr;
    uint32_t end_addr_pool = heap->start_addr + heap->pool_size;

    if (buf_start < start_addr_pool || buf_start + buf_length > end_addr_pool)
    {
        return false;
    }

    return true;
}

void * nema_host_malloc (size_t size)
{
    return am_mem_dtcm_malloc(size);
}

void nema_host_free (void * ptr)
{
    am_mem_dtcm_free(ptr);
}

int nema_mutex_lock (int mutex_id)
{
	return 0;
}

int nema_mutex_unlock (int mutex_id)
{
	return 0;
}

am_hal_status_e nema_get_cl_status (int32_t cl_id)
{
    // CL_id is negative
    if ( cl_id < 0) {
        return AM_HAL_STATUS_SUCCESS;
    }

    // NemaP is IDLE, so CL has to be finished
    if ( nema_reg_read(NEMA_STATUS) == 0 ) {
        return AM_HAL_STATUS_SUCCESS;
    }

    int _last_cl_id = (int)nema_reg_read(NEMA_CLID);
    if ( _last_cl_id >= cl_id) {
        return AM_HAL_STATUS_SUCCESS;
    }
    return AM_HAL_STATUS_IN_USE;
}

//*****************************************************************************
//
//! @brief Check wether the core ring buffer is full or not
//!
//! @return True, the core ring buffer is full, we need wait for GPU before 
//!         submit the next CL.
//!         False, the core ring buffer is not full, we can submit the next CL.
//*****************************************************************************
bool nema_rb_check_full(void)
{
    uint32_t total_pending_cl = 0;
    if(ring_buffer_str.last_submission_id > last_cl_id)
    {
        total_pending_cl = ring_buffer_str.last_submission_id - last_cl_id;
    }
    else if(ring_buffer_str.last_submission_id < last_cl_id)
    {
        if(last_cl_id == 0xFFFFFF)
        {
            total_pending_cl = ring_buffer_str.last_submission_id + 1;
        }
        else
        {
            //should never got here.
        }
    }

    return (total_pending_cl >= MAX_PENDING_CL);
}

void nema_reset_last_cl_id (void)
{
    last_cl_id = -1;
}
int nema_get_last_cl_id(void)
{
    return last_cl_id;
}

int nema_get_last_submission_id(void)
{
    return ring_buffer_str.last_submission_id;
}

//*****************************************************************************
//
//! @brief Controls the power state of the GPU peripheral.
//!
//! @param ePowerState - The desired power state (e.g., wake, normal sleep, deep sleep).
//! @param bRetainState - Indicates whether to reinitialize the NemaSDK and NemaVG
//!                       when waking up the GPU peripheral.
//!
//! This function manages the power state of the GPU peripheral based on
//! the requested power state. It checks the current power status and either
//! powers up or powers down the peripheral as needed.
//!
//! When waking up the GPU (`AM_HAL_SYSCTRL_WAKE`), if \e bRetainState is true,
//! the function reinitializes the NemaSDK and NemaVG. If powering down, the
//! function ensures the peripheral is inactive before disabling power.
//!
//! \e ePowerState The desired power state. Valid values are:
//! - AM_HAL_SYSCTRL_WAKE: Wake up the peripheral.
//! - AM_HAL_SYSCTRL_NORMALSLEEP: Put the peripheral in normal sleep mode.
//! - AM_HAL_SYSCTRL_DEEPSLEEP: Put the peripheral in deep sleep mode.
//!
//! \e bRetainState Determines whether to reinitialize the NemaSDK and NemaVG
//! when powering up. This is used for scenarios where the GPU context needs
//! to be retained.
//!
//! @note Ensure that the GPU peripheral is not in use before attempting to
//! power it down to avoid conflicts. This API is not thread-safe. If it is
//! called from multiple threads or from an interrupt, appropriate critical
//! section protection must be added.
//!
//! @return Returns the status of the operation. Possible return values are:
//! - AM_HAL_STATUS_SUCCESS: Operation was successful.
//! - AM_HAL_STATUS_IN_USE: Peripheral is currently in use and cannot be powered down.
//! - AM_HAL_STATUS_INVALID_OPERATION: Invalid power state requested.
//! - AM_HAL_STATUS_FAIL: Reinitialization of NemaSDK or NemaVG failed.
//
//*****************************************************************************
uint32_t
nemagfx_power_control(am_hal_sysctrl_power_state_e ePowerState,
                   bool bRetainState)
{
    uint32_t ui32Status;
    bool bEnabled;
    uint32_t ui32ErrorCode;

    //
    // Check the current GPU power state
    //
    ui32Status = am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_GFX, &bEnabled);
    if (ui32Status == AM_HAL_STATUS_SUCCESS)
    {
        //
        // Decode the requested power state and take action
        //
        switch (ePowerState)
        {
            case AM_HAL_SYSCTRL_WAKE:
            {
                if (bEnabled)
                {
                    //
                    // GPU is already powered up
                    //
                    ui32Status = AM_HAL_STATUS_SUCCESS;
                }
                else
                {
                    //
                    // Enable power control
                    //
                    ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

                    //
                    // Reinitialize NemaSDK and NemaVG
                    //
                    if (ui32Status == AM_HAL_STATUS_SUCCESS && bRetainState)
                    {

                        nema_get_error(); // clear raster graphics error

                        nema_reset_last_cl_id();
                        nema_reinit();
                        ui32ErrorCode = nema_get_error();
                        if (ui32ErrorCode != NEMA_ERR_NO_ERROR)
                        {
                            ui32Status = AM_HAL_STATUS_FAIL;
                            break;
                        }


#ifndef NEMA_MULTI_THREAD
                        // If NEMA_MULTI_THREAD is not defined, the nemavg_context is defined as global variable,
                        // and if nema_vg_init is not called, nemavg_context will be initialized as NULL, so we should prevent to write to the NULL pointer.
                        struct nema_vg_context_t_;
                        extern struct nema_vg_context_t_* nemavg_context;

                        if(nemavg_context != NULL)
                        {
#endif
                            nema_vg_get_error(); // clear vector graphics error

                            nema_vg_reinit();
                            ui32ErrorCode = nema_vg_get_error();
                            if (ui32ErrorCode != NEMA_VG_ERR_NO_ERROR)
                            {
                                ui32Status = AM_HAL_STATUS_FAIL;
                                break;
                            }
#ifndef NEMA_MULTI_THREAD
                        }
#endif
                    }
                }
                break;
            }
            case AM_HAL_SYSCTRL_NORMALSLEEP:
            case AM_HAL_SYSCTRL_DEEPSLEEP:
            {
                if (!bEnabled)
                {
                    //
                    // GPU is already powered down
                    //
                    ui32Status = AM_HAL_STATUS_SUCCESS;
                }
                else
                {
                    //
                    // Ensure GPU is currently inactive
                    //
                    if (nema_reg_read(NEMA_STATUS) != 0)
                    {
                        ui32Status = AM_HAL_STATUS_IN_USE;
                    }
                    else
                    {
                        //
                        // Power down the GPU
                        //
                        ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
                    }
                }
                break;
            }
            default:
            {
                ui32Status = AM_HAL_STATUS_INVALID_OPERATION;
                break;
            }
        }

    }

    return ui32Status;
}