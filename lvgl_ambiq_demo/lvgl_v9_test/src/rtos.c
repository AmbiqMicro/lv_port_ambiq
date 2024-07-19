//*****************************************************************************
//
//! @file rtos.c
//!
//! @brief Essential functions to make the RTOS run correctly.
//!
//! These functions are required by the RTOS for ticking, sleeping, and basic
//! error checking.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "lvgl_test.h"
#include "lvgl.h"

//*****************************************************************************
//
// Task handle for the initial setup task.
//
//*****************************************************************************
TaskHandle_t xSetupTask;


//*****************************************************************************
//
// Sleep function called from FreeRTOS IDLE task.
// Do necessary application specific Power down operations here
// Return 0 if this function also incorporates the WFI, else return value same
// as idleTime
//
//*****************************************************************************
uint32_t am_freertos_sleep(uint32_t idleTime)
{
// #### INTERNAL BEGIN ####
#ifndef APOLLO5_FPGA
// #### INTERNAL END ####

    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    return 0;
}

//*****************************************************************************
//
// Recovery function called from FreeRTOS IDLE task, after waking up from Sleep
// Do necessary 'wakeup' operations here, e.g. to power up/enable peripherals etc.
//
//*****************************************************************************
void am_freertos_wakeup(uint32_t idleTime)
{
    return;
}


//*****************************************************************************
//
// FreeRTOS debugging functions.
//
//*****************************************************************************
void
vApplicationMallocFailedHook(void)
{
    const char* pTaskName;

    //
    // Called if a call to pvPortMalloc() fails because there is insufficient
    // free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    // internally by FreeRTOS API functions that create tasks, queues, software
    // timers, and semaphores.  The size of the FreeRTOS heap is set by the
    // configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h.
    //
    am_util_stdio_printf("Heap malloc failed!\r\n");

    pTaskName = pcTaskGetName(NULL);

    am_util_stdio_printf("Suspend %s task...\r\n", pTaskName);

    vTaskSuspend(NULL);
}

void
vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    //
    // Run time stack overflow checking is performed if
    // configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    // function is called if a stack overflow is detected.
    //
    while (1)
    {
        __asm("BKPT #0\n") ; // Break into the debugger
    }
}

//*****************************************************************************
//
// Get CPU occupation rate.
//
//*****************************************************************************
#if LV_USE_AMBIQ_PERF_MONITOR_REFINED
uint32_t getCpuOccupationRate(void)
{
    TaskHandle_t idleHandle = xTaskGetIdleTaskHandle();

    TaskStatus_t xTaskDetails;

    // Use the handle to obtain further information about the task.
    vTaskGetInfo( idleHandle,
                  &xTaskDetails,
                  pdTRUE, // Include the high water mark in xTaskDetails.
                  eSuspended ); // Include the task state in xTaskDetails.

    //Counter overflow has not been considered.
    uint32_t idleRunTime = xTaskDetails.ulRunTimeCounter;
    uint32_t totalRunTime = xTaskGetTickCount();

    static uint32_t lastIdleRunTime = 0;
    static uint32_t lastTotalRunTime = 0;

    uint32_t runTimeElaps;
    uint32_t totalTimeElaps;

    //If counter overflow
    if ( totalRunTime < lastTotalRunTime )
    {
        totalTimeElaps = (uint64_t)totalRunTime + 0xffffffff - lastTotalRunTime;
    }
    else
    {
        totalTimeElaps = totalRunTime - lastTotalRunTime;
    }

    //If counter overflow
    if ( idleRunTime < lastIdleRunTime )
    {
        runTimeElaps = (uint64_t)idleRunTime + 0xffffffff - lastIdleRunTime;
    }
    else
    {
        runTimeElaps = idleRunTime - lastIdleRunTime;
    }

    //Update record time
    lastIdleRunTime = idleRunTime;
    lastTotalRunTime = totalRunTime;


    //if not overflow
    if ( runTimeElaps < totalTimeElaps )
    {
        return 100 - runTimeElaps * 100 / totalTimeElaps ;
    }
    else
    {
        return 0;
    }
}
#endif

//*****************************************************************************
//
// High priority task to run immediately after the scheduler starts.
//
// This task is used for any global initialization that must occur after the
// scheduler starts, but before any functional tasks are running. This can be
// useful for enabling events, semaphores, and other global, RTOS-specific
// features.
//
//*****************************************************************************
void
setup_task(void *pvParameters)
{
    //
    // Print a debug message.
    //
    am_util_stdio_printf("Running setup tasks...\r\n");

    //
    // Stop task switch
    //
    vTaskSuspendAll();

    // //
    // // Create the Gui tasks, this task will control render task and display task
    // //
    // xTaskCreate(GuiTask, "GuiTask", 1024, 0, 1, &GuiTaskHandle);

    //
    // Create the display tasks, put at the highest priority to save power.
    //
    xTaskCreate(DisplayTask, "DisplayTask", 4096, 0, 2, &DisplayTaskHandle);

    //
    // Resume task switch
    //
    xTaskResumeAll();

    //
    // The setup operations are complete, so suspend the setup task now.
    //
    vTaskDelete(NULL);
}

extern void prvHeapInit_external( void* address, size_t length );

//*****************************************************************************
//
// Initializes all tasks
//
//*****************************************************************************
void
run_tasks(void)
{
    //
    // Create essential tasks.
    //
    xTaskCreate(setup_task, "Setup", 512, 0, 1, &xSetupTask);


    //
    // Start the scheduler.
    //
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following
     * line will never be reached.  If the following line does execute, then
     * there was insufficient FreeRTOS heap memory available for the idle and/or
     * timer tasks to be created.  See the memory management section on the
     * FreeRTOS web site for more details (this is standard text that is not
     * really applicable to the Win32 simulator port). */
    while (1);
}

