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

#include "stdint.h"
#include "nema_dc_regs.h"
#include "nema_dc_hal.h"
#include "nema_dc_mipi.h"
#include "nema_dc_intern.h"
#include "nema_dc.h"
#include "nema_dc_dsi.h"
#include "nema_ringbuffer.h"
#include "nema_sys_defs.h"
#include "am_mcu_apollo.h"
#include "nema_dc_jdi.h"
#include "nema_math.h"

#ifdef BAREMETAL

#  ifndef WAIT_IRQ_POLL
#    define WAIT_IRQ_POLL              1
#  endif

#  ifdef WAIT_IRQ_BINARY_SEMAPHORE
#    warning semaphore will not work under baremetal
#    undef WAIT_IRQ_BINARY_SEMAPHORE
#    define WAIT_IRQ_BINARY_SEMAPHORE  0
#  endif

#else  // BAREMETAL

#  ifdef SYSTEM_VIEW
#    include "SEGGER_SYSVIEW_FreeRTOS.h"
#  endif

#  include "FreeRTOS.h"
#  include "task.h"

#  ifndef WAIT_IRQ_POLL
#    define WAIT_IRQ_POLL              0
#  endif

#  ifndef WAIT_IRQ_BINARY_SEMAPHORE
#    define WAIT_IRQ_BINARY_SEMAPHORE  1
#  endif

#  if WAIT_IRQ_BINARY_SEMAPHORE
#    include "semphr.h"
     static SemaphoreHandle_t xSemaphore_vsync = NULL;
     static SemaphoreHandle_t xSemaphore_TE = NULL;
#  endif

#endif // BAREMETAL

#ifndef NEMADC_BASEADDR
#if defined(AM_PART_BRONCO)
#include "bronco.h"
#elif defined(AM_PART_APOLLO5B)
#include "apollo5b.h"
#endif
#define NEMADC_BASEADDR       DC_BASE
#endif

// IRQ number
#ifndef NEMADC_IRQ
#define NEMADC_IRQ           ((IRQn_Type)29U)
#endif

static uintptr_t nemadc_regs = 0;

volatile int irq_count = 0;

//
// declaration of display controller interrupt callback function.
//

nema_dc_interrupt_callback  nemadc_te_cb = NULL;
nema_dc_interrupt_callback  nemadc_vsync_cb = NULL;
static void* vsync_arg;
static bool bNeedLaunchInTe = false;
//
// The structure for MiP interface
//
static MiP_display_config_t sMiPConfig = {0};
static float fFormatPeriod = 0;
//*****************************************************************************
//
//! @brief DC's TE interrupt callback initialize function
//!
//! @param  fnTECallback                - DC TE interrupt callback function
//!
//! this function used to initialize display controller te interrupt
//! callback function.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_set_te_interrupt_callback(nema_dc_interrupt_callback fnTECallback)
{
    nemadc_te_cb = fnTECallback;
}

//*****************************************************************************
//
//! @brief DC's vsync interrupt callback initialize function
//!
//! @param  fnVsyncCallback - DC Vsync interrupt callback function
//! @param  arg             - DC Vsync interrupt callback argument
//!
//! this function used to initialize display controller vsync interrupt
//! callback function.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_set_vsync_interrupt_callback(nema_dc_interrupt_callback fnVsyncCallback, void* arg)
{
    nemadc_vsync_cb = fnVsyncCallback;
    vsync_arg = arg;
}

//*****************************************************************************
//
//! @brief wait NemaDC to become idle
//!
//! @param ui32Mask   - Mask for the status change.
//! @param ui32Value  - Target value for the status change.
//!
//! This function will delay for approximately the given number of microseconds
//! while checking for a status change, exiting when either the given time has
//! expired or the status change is detected.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//*****************************************************************************
static uint32_t
wait_dbi_idle(uint32_t ui32Mask, uint32_t ui32Value)
{
    uint32_t ui32usMaxDelay = 100000;
    uint32_t ui32Status;
    //
    // wait NemaDC to become idle
    //
    return am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->STATUS, ui32Mask, ui32Value);
}
//*****************************************************************************
//
//! @brief wait jdi idle
//!
//! @return AM_HAL_STATUS_SUCCESS.
//
//*****************************************************************************
static uint32_t
wait_mip_idle(void)
{
    am_hal_delay_us(fFormatPeriod * sMiPConfig.VCK_GCK_closing_pulses * sMiPConfig.VCK_GCK_width);
    nemadc_set_mode(0);
    nemadc_set_mip_panel_parameters(&sMiPConfig);
    return AM_HAL_STATUS_SUCCESS;
}
//*****************************************************************************
//
//! @brief Configure NemaDC.
//!
//! @param  psDCConfig     - NemaDC configuration structure.
//!
//! This function configures NemaDC display interface, output color format,
//! clock settings, TE setting and timing.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_configure(nemadc_initial_config_t *psDCConfig)
{
    uint32_t cfg = 0;
    if(psDCConfig->eInterface == DISP_INTERFACE_DBI || psDCConfig->eInterface == DISP_INTERFACE_DBIDSI)
    {
        //
        // Program NemaDC MIPI interface
        //
        if (psDCConfig->eInterface == DISP_INTERFACE_DBIDSI)
        {
            if(CLKGEN->DISPCLKCTRL_b.DISPCLKSEL == CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC192)
            {
                //
                // Set the primary divider ratio to 2 to make sure the pixel clock isn't greater than 96MHz and bypass predivider
                //
                nemadc_clkdiv(2, 1, 4, 0);
            }
            else
            {
                //
                // Set the primary divider ratio to 0 or 1,it's bypass the primary divider.the swap feature is invalid in this situation.
                // After this configuration, both pixel_clk and format_clk's frequencies are from pll_clk through predivider if its value is equal to 0 or 1;
                // they are from pll_clk directly when its value is 0 or 1.
                //
                nemadc_clkdiv(1, 1, 4, 0);
            }
            cfg = MIPICFG_DBI_EN | MIPICFG_RESX | MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN | MIPICFG_EN_STALL | psDCConfig->ui32PixelFormat;
            //
            // Setting the DBIB_CLK clock frequency is the half of the format clock, that is 96MHz.(This is the limitation of the DSI host)
            //
            nemadc_reg_write(NEMADC_REG_FORMAT_CTRL2, 0x2U << 30);
        }
        else
        {
            int i32PreDivider;
            float fPLLCLKFreq;
            //
            // Calculated the present clock source frequency.
            //
            fPLLCLKFreq = 3 * (2 << CLKGEN->DISPCLKCTRL_b.DISPCLKSEL);
            i32PreDivider = nema_ceil(fPLLCLKFreq / psDCConfig->fCLKMaxFreq);

            //
            // The value of the predivider should be less than 128 on Apollo5B.
            //
            if(i32PreDivider > 127)
            {
                return;
            }
            nemadc_clkdiv( 1, i32PreDivider, 4, 0);

            cfg = MIPICFG_DBI_EN | MIPICFG_RESX | psDCConfig->ui32PixelFormat;
        }
        if (psDCConfig->bTEEnable)
        {
            cfg |= MIPICFG_DIS_TE;
        }
        nemadc_MIPI_CFG_out(cfg);
        //
        // Program NemaDC to transfer a resx*resy region
        //
        psDCConfig->ui32FrontPorchX = 1;
        psDCConfig->ui32BlankingX = 10;
        psDCConfig->ui32BackPorchX = 1;
        psDCConfig->ui32FrontPorchY = 1;
        psDCConfig->ui32BlankingY = 1;
        psDCConfig->ui32BackPorchY = 1;
    }
    else if ((psDCConfig->eInterface == DISP_INTERFACE_QSPI) ||
             (psDCConfig->eInterface == DISP_INTERFACE_DSPI) ||
             (psDCConfig->eInterface == DISP_INTERFACE_SPI4))
    {
        //
        // Bypass primary divider and predivider
        //
        nemadc_clkdiv(1, 1, 4, 0);
        cfg = MIPICFG_SPI_CSX_V | MIPICFG_DBI_EN | MIPICFG_RESX | MIPICFG_SPI4 | psDCConfig->ui32PixelFormat;
        if (psDCConfig->eInterface == DISP_INTERFACE_QSPI)
        {
            cfg |= MIPICFG_QSPI;
        }
        else if (psDCConfig->eInterface == DISP_INTERFACE_DSPI)
        {
            cfg |= MIPICFG_DSPI;
        }
        
        if(psDCConfig->bTEEnable)
        {
            cfg |= MIPICFG_DIS_TE;
        }
        nemadc_MIPI_CFG_out(cfg);
        // Program NemaDC to transfer a resx*resy region
        psDCConfig->ui32FrontPorchX = 1;
        psDCConfig->ui32BlankingX = 1;
        psDCConfig->ui32BackPorchX = 1;
        psDCConfig->ui32FrontPorchY = 1;
        psDCConfig->ui32BlankingY = 1;
        psDCConfig->ui32BackPorchY = 1;
    }
    else if (psDCConfig->eInterface == DISP_INTERFACE_JDI)
    {
        int i32PreDivider,i32PrimaryDivider;
        float fPLLCLKFreq;
#ifdef APOLLO5_FPGA
        fPLLCLKFreq = APOLLO5_FPGA;
#else
        fPLLCLKFreq = 3 * (2 << CLKGEN->DISPCLKCTRL_b.DISPCLKSEL);
#endif
        //
        // Calculate the Optimal solution primary divider and predivider for the JDI interface.
        //
        //
        // To reach a higher performance,we should make sure the frequencies of BCK(its frequency is a quarter of format_clk)
        // and GCK less than its maximum supported of the panel.
        //
        if (fPLLCLKFreq / (4 * psDCConfig->fHCKBCKMaxFreq) > psDCConfig->fHCKBCKMaxFreq / psDCConfig->fVCKGCKFFMaxFreq)
        {
            //
            // Calculate the primary divider,to make BCK frequency less than panel limitation. 
            //
            i32PrimaryDivider = nema_ceil(fPLLCLKFreq / (4 * psDCConfig->fHCKBCKMaxFreq));
            //
            // The minimum of primary divider is 2 to enable the divider swap feature.
            //
            if (i32PrimaryDivider < 2)
            {
                i32PrimaryDivider = 2;
            }
            //
            // Set the predivider to 1, to bapass it.
            //
            i32PreDivider = 1;
        }
        else
        {
            //
            // Calculate primary divider to assure the frequency of GCK less than its maximum supported.
            //
            i32PrimaryDivider = (int)(psDCConfig->fHCKBCKMaxFreq / psDCConfig->fVCKGCKFFMaxFreq);
            //
            // The minimum of primary divider is 2 to enable the divider swap feature.
            //
            if (i32PrimaryDivider < 2)
            {
                i32PrimaryDivider = 2;
            }
            //
            // Calculate predivider to assure the frequency of BCK less than its maximum supported.
            //
            i32PreDivider = nema_ceil(fPLLCLKFreq / (4 * psDCConfig->fHCKBCKMaxFreq * i32PrimaryDivider));
        }

        nemadc_clkdiv(i32PrimaryDivider, i32PreDivider, 4, 0);
        
        sMiPConfig.resx                    = psDCConfig->ui16ResX;
        sMiPConfig.resy                    = psDCConfig->ui16ResY;
        sMiPConfig.XRST_INTB_delay         = psDCConfig->ui32XRSTINTBDelay;
        sMiPConfig.XRST_INTB_width         = psDCConfig->ui32XRSTINTBWidth;
        sMiPConfig.VST_GSP_delay           = psDCConfig->ui32VSTGSPDelay;
        sMiPConfig.VST_GSP_width           = psDCConfig->ui32VSTGSPWidth;
        sMiPConfig.VCK_GCK_delay           = psDCConfig->ui32VCKGCKDelay;
        sMiPConfig.VCK_GCK_width           = psDCConfig->ui32VCKGCKWidth;
        sMiPConfig.VCK_GCK_closing_pulses  = psDCConfig->ui32VCKGCKClosingPulses;
        sMiPConfig.HST_BSP_delay           = psDCConfig->ui32HSTBSPDelay;
        sMiPConfig.HST_BSP_width           = psDCConfig->ui32HSTBSPWidth;
        sMiPConfig.HCK_BCK_data_start      = psDCConfig->ui32HCKBCKDataStart;
        sMiPConfig.ENB_GEN_delay           = psDCConfig->ui32ENBGENDelay;
        sMiPConfig.ENB_GEN_width           = psDCConfig->ui32ENBGENWidth;
        nemadc_set_mip_panel_parameters(&sMiPConfig);
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_swap | NemaDC_clkctrl_cg_clk_en);
        fFormatPeriod = 1.0 / (psDCConfig->fHCKBCKMaxFreq * 4);
        return;
    }
    else if (psDCConfig->eInterface == DISP_INTERFACE_DPI)
    {
        int i32PreDivider;
        float fPLLCLKFreq;
        fPLLCLKFreq = 3 * (2 << CLKGEN->DISPCLKCTRL_b.DISPCLKSEL);
        i32PreDivider = nema_ceil(fPLLCLKFreq / psDCConfig->fCLKMaxFreq);

        //
        // The value of the predivider should be less than 32 on Apollo5A.
        //
        if(i32PreDivider > 31)
        {
            return;
        }
        nemadc_clkdiv( 1, i32PreDivider, 4, 0);

        //
        // Configure DPI(RGB) interface color coding, Hsync and Vsync polarity.
        //
        nemadc_reg_write(NEMADC_REG_MODE, NEMADC_NEG_V | NEMADC_NEG_H | psDCConfig->ui32PixelFormat);
    }

    nemadc_timing(psDCConfig->ui16ResX, 
                  psDCConfig->ui32FrontPorchX, 
                  psDCConfig->ui32BlankingX, 
                  psDCConfig->ui32BackPorchX,
                  psDCConfig->ui16ResY, 
                  psDCConfig->ui32FrontPorchY, 
                  psDCConfig->ui32BlankingY, 
                  psDCConfig->ui32BackPorchY);
    //
    // Enable clock divider.
    //
    if (psDCConfig->eInterface == DISP_INTERFACE_DPI)
    {
        //
        // Invert clock polarity
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_inv | NemaDC_clkctrl_cg_clk_en);
    }
    else
    {
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
    }
}

//*****************************************************************************
//
//! @brief Prepared operations before sending frame
//!
//! @param  bAutoLaunch    - true:launch transfer in DC TE interrupt implicitly.
//!
//! This function configures clock gating, sends MIPI_write_memory_start
//! command before sending frame. If DBIDSI interface is selected, this function
//! also configures HS/LP mode and data/command type of DSI.
//! Note: bLaunchInTE taks effect in the DC TE interrupt handler, which means
//! if GPIO TE is used or TE signal is ignored, setting this parameter to true or false
//! makes no difference, user still need to call nemadc_transfer_frame_launch manually.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_transfer_frame_prepare(bool bAutoLaunch)
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    uint32_t ui32MemWrCmd = MIPI_write_memory_start;
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            am_hal_dsi_pre_rw_cmd(true);
            
            nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));//HS
            //
            // disable clock gating
            //
            nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0xFFFFFFF0U | NemaDC_clkctrl_cg_clk_en);
    
            wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U);
            //
            // Set data/commands command type
            //
            nemadc_dsi_ct(NemaDC_dt_DCS_long_write, // Unused parameter
                          NemaDC_dt_DCS_long_write, // Unused parameter
                          NemaDC_dcs_datacmd);

            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            //
            // Send DCS write_memory_start command
            //
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
            wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U);
        }
        else
        {
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
            nemadc_MIPI_out(MIPI_DBIB_CMD | NemaDC_wcmd24 | (ui32MemWrCmd << 8));
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        //
        // Enable clock divider
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
        if ((ui32Cfg & MIPICFG_QSPI) != 0)
        {
            //
            // QSPI interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg | MIPICFG_SPI_HOLD);
            nemadc_MIPI_out( MIPI_DBIB_CMD | MIPI_MASK_QSPI | CMD1_DATA4);
            nemadc_MIPI_out( MIPI_DBIB_CMD | MIPI_MASK_QSPI | MIPI_CMD24 |
                            (ui32MemWrCmd << CMD_OFFSET));
        }
        else if((ui32Cfg & MIPICFG_DSPI) != 0)
        {
            //
            // DSPI interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg & (~MIPICFG_DSPI));
            // Start MIPI Panel Memory Write
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
            nemadc_MIPI_CFG_out(((ui32Cfg & (~MIPICFG_SPI4)) | MIPICFG_SPI3) | MIPICFG_SPIDC_DQSPI | MIPICFG_SPI_HOLD);
        }
        else
        {
            //
            // SPI4 interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg | MIPICFG_SPI_HOLD);
            // Start MIPI Panel Memory Write
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
        }
    }
    //
    // turn DC TE interrupt
    //
    if (ui32Cfg & MIPICFG_DIS_TE)
    {
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 3);
    }

    bNeedLaunchInTe = bAutoLaunch;
}

//*****************************************************************************
//
//! @brief Launch frame transfer
//!
//! This function enables DC frame end interrupt and launches frame transfer.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_transfer_frame_launch()
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Enable frame end interrupt for DBI/DSI interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_OUTP_OFF);
    }
    else if (((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX)) != 0))
    {
        //
        // Enable spi frame end interrupt for SPI4/DSPI/QSPI interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 5);
        //
        // Send One Frame
        //
        nemadc_set_mode(NEMADC_ONE_FRAME);
    }
    else if (0x3FF & nemadc_reg_read(NEMADC_REG_FORMAT_CTRL3))
    {
        //
        // Enable frame end interrupt for JDI interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_MIP_IF | NEMADC_SCANDOUBLE);
    }
    else
    {
        //
        // Enable frame end interrupt for DPI(RGB) interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);

        nemadc_set_mode(NEMADC_ONE_FRAME | nemadc_reg_read(NEMADC_REG_MODE));
    }
}

//*****************************************************************************
//
//! @brief Required operations after completing frame transfer
//!
//! This function sets NemaDC back to normal mode after completing frame
//! transfer. This function should be called in frame end interrupt.
//!
//! @return None.
//
//****************************************************************************
static void
nemadc_transfer_frame_end()
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_SPI_HOLD));
            nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en); // enable clock gating
            nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1); // LP
        }
        else
        {
            nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_FRC_CSX_0));
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        nemadc_MIPI_CFG_out((ui32Cfg | MIPICFG_SPI4) & (~(MIPICFG_SPI3 | MIPICFG_SPIDC_DQSPI | MIPICFG_SPI_HOLD)));
        //
        // enable clock gating
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0);
    }
    else if (0x3FF & nemadc_reg_read(NEMADC_REG_FORMAT_CTRL3))
    {
        //
        // Clear mode register
        //
    }
    else
    {
        //
        // Frame end operation for DPI(RGB) interface.
        //
    }

}

//*****************************************************************************
//
//! @brief  DBI write
//!
//! @param  ui8Cmd        - command
//! @param  pui8Para      - pointer of parameters to be sent
//! @param  ui8ParaLen    - length of parameters to be sent
//!
//! This function sends DBI commands to panel.
//!
//! @return AM_HAL_STATUS_SUCCESS.
//
//****************************************************************************
static uint32_t
dbi_write(uint8_t ui8Cmd, uint8_t *pui8Para, uint8_t ui8ParaLen)
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    //
    // the maximum length of write is 8 bytes.(1 command + 7 parameters)
    //
    if (ui8ParaLen > 7)
    {
        ui8ParaLen = 7;
    }
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    nemadc_MIPI_out(NemaDC_DBI_cmd | NemaDC_wcmd24 | (ui8Cmd << 8));
    for (uint8_t ui8Index = 0; ui8Index < ui8ParaLen; ui8Index++)
    {
        nemadc_MIPI_out(pui8Para[ui8Index]);
    }
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    nemadc_MIPI_CFG_out(ui32Cfg);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  DBI read
//!
//! @param  ui8Cmd        - command
//! @param  ui8ParaLen    - length of parameters to be read
//! @param  ui32Received  - received data.
//!
//! This function sends DBI commands to panel.
//!
//! @return AM_HAL_STATUS_SUCCESS.
//
//****************************************************************************
static uint32_t
dbi_read(uint8_t ui8Cmd, uint8_t ui8ParaLen, uint32_t *ui32Received)
{
    uint32_t ui32ReadValue,ui32Mask = 0;

    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Force Chip Select line low.
    //
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    //
    // the maximum length of this kind of read is 4 bytes.
    //
    if (ui8ParaLen > 4)
    {
        ui8ParaLen = 4;
    }

    //
    // Send read command
    //
    nemadc_MIPI_cmd(NemaDC_wcmd24 | (ui8Cmd << 8));
    //
    // wait NemaDC to become idle
    //
    while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);
    //
    // trigger dbi-read operation for 8bits,first 8bits is invalid data
    //
    nemadc_reg_write(NEMADC_REG_DBIB_RDAT, (0x0 << 30));

    while (ui8ParaLen-- != 0)
    {
        //
        // trigger dbi-read operation for 8bits
        //
        nemadc_reg_write(NEMADC_REG_DBIB_RDAT, (0x0 << 30));
        ui32Mask <<= 8;
        ui32Mask |= 0xFF;
    }

    //
    // wait NemaDC to become idle
    //
    while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);
    nemadc_MIPI_CFG_out(ui32Cfg);

    *ui32Received = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & ui32Mask;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief DBI-B interface read the frame buffer
//!
//! @param ui8ReceiveBuffer     - pointer of receive buffer
//! @param ui32ParaLen          - the read count
//!
//! @note During a read cycle the host processor reads data from the display module  
//! via the interface. The Type B interface utilizes D/CX, RDX and WRX signals as  
//! well as all eight (D[7:0]) information signals.RDX is driven from high to low 
//! then allowed to be pulled back to high during the read cycle. The display module 
//! provides information to the host processor during the read cycle while the host 
//! processor reads the display module information on the rising edge of RDX. D/CX 
//! is driven high during the read cycle.
//!
//! @return pointer.
//
//*****************************************************************************
void
dbi_frame_read(uint8_t *ui8ReceiveBuffer, uint32_t ui32ParaLen)
{

    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Force Chip Select line low.
    //
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    //
    // Send command to read memory.
    //
    nemadc_MIPI_cmd(NemaDC_wcmd24 | (MIPI_read_memory_start << 8));

    while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);

    for (uint32_t index = 0; index < ui32ParaLen; index++)
    {
        //
        // trigger dbi-read operation for 8bits
        //
        nemadc_reg_write(NEMADC_REG_DBIB_RDAT, (0x0 << 30));

        while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);

        ui8ReceiveBuffer[index] = (uint8_t)(nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & 0xff);
    }

    nemadc_MIPI_CFG_out(ui32Cfg);
}

//*****************************************************************************
//
//! @brief  Send DSI DCS(Display Command Set) write commands
//!
//! @param  ui8Cmd        - command
//! @param  pui8Para      - pointer of parameters to be sent
//! @param  ui8ParaLen    - length of parameters to be sent
//! @param  bHS           - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI DCS commands.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_dcs_write(uint8_t ui8Cmd, uint8_t* pui8Para, uint8_t ui8ParaLen, bool bHS)
{
    uint32_t ui32Cfg = 0;

    if (bHS == true) // high speed mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct((uint32_t)0, // Unused parameter
                  (uint32_t)0, // Unused parameter
                   NemaDC_dcs_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // enable command/parameters packing
    //
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
    //
    // Download command & parameter to DBI i/f
    //
    nemadc_MIPI_out(NemaDC_DBI_cmd | ui8Cmd);
    for (uint8_t ui8Index = 0; ui8Index < ui8ParaLen; ui8Index++)
    {
        nemadc_MIPI_out(pui8Para[ui8Index]);
    }
    //
    // Send command-parameter packet.
    //
    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Send DSI generic write command.
//!
//! @param  pui8Para        - pointer of parameters to be sent
//! @param  ui8ParaLen      - length of parameters to be sent,it should be greater than zero.
//! @param  bHS             - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI generic commands.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_generic_write(uint8_t* pui8Para, uint8_t ui8ParaLen, bool bHS)
{
    uint32_t ui32Cfg = 0;
    nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x6)); // set vc_no to 00.

    if (bHS == true) //high speed mode
    {
        //
        // Transmit in HS mode
        //
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct((uint32_t)0,  // Unused parameter
                  (uint32_t)0,  // Unused parameter
                   NemaDC_ge_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // enable command/parameters packing
    //
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);

    if ( ui8ParaLen < 9 )
    {
        //
        // Disable the overlong write feature when its length is less than 9 bytes.
        //
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x08));
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
    }
    else
    {
        //
        // Enable the overlong write feature when its length isn't less than 9 bytes.
        //
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x08);
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    }

    //
    // Download command & parameter to DBI i/f
    //
    if ( ui8ParaLen == 0 )
    {
        nemadc_MIPI_out(NemaDC_DBI_cmd | 0x00);
    }
    else
    {
        for (uint8_t ui8Index = 0; ui8Index < ui8ParaLen; ui8Index++)
        {
            if ( ui8ParaLen < 3 )
            {
                nemadc_MIPI_out(NemaDC_DBI_cmd | pui8Para[ui8Index]);
            }
            else
            {
                nemadc_MIPI_out(pui8Para[ui8Index]);
            }
        }
    }
    //
    // Send command-parameter packet.
    //
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Send SPI4/DSPI/QSPI/DSI command via Display Controller(DC)
//!
//! @param  ui8Command      - command
//! @param  p_ui8Para       - pointer of parameters to be sent
//! @param  ui8ParaLen      - length of parameters to be sent
//! @param  bDCS            - DCS command or generic command
//! @param  bHS             - high speed mode or low power mode (escape mode)
//!
//! This function sends commands to display drive IC.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
uint32_t
nemadc_mipi_cmd_write(uint8_t ui8Command,
                      uint8_t* p_ui8Para,
                      uint8_t ui8ParaLen,
                      bool bDCS,
                      bool bHS)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            am_hal_dsi_pre_rw_cmd(bHS);

            if (bDCS)
            {
                ui32Status = dsi_dcs_write(ui8Command, p_ui8Para, ui8ParaLen, bHS);
            }
            else
            {
                ui32Status = dsi_generic_write(p_ui8Para, ui8ParaLen, bHS);
            }
        }
        else
        {
            ui32Status = dbi_write(ui8Command, p_ui8Para, ui8ParaLen);
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        //
        // SPI4,DSPI and QSPI interfaces.
        //
        uint32_t ui32Cmd = 0;
        //
        // Enable Clock
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }

        if ((ui32Cfg & MIPICFG_QSPI) != 0)
        {
            //
            // QSPI interface.
            //
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            ui32Cmd = MIPI_DBIB_CMD | MIPI_MASK_QSPI;
            nemadc_MIPI_out(ui32Cmd | CMD1_DATA1);
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD24 | (ui8Command << CMD_OFFSET));
            //
            // maximum value of ui8ParaLen isn't greater than 18.
            //
            if (ui8ParaLen > 18)
            {
                ui8ParaLen = 18;
            }
        }
        else
        {
            if ((ui32Cfg & MIPICFG_DSPI) != 0)
            {
                //
                // DSPI interface
                //
                nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_DSPI));
            }
            else
            {
                //
                // SPI4 interface
                // 
                nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            }
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui8Command);
            //
            // maximum value of ui8ParaLen isn't greater than 21.
            //
            if (ui8ParaLen > 21)
            {
                ui8ParaLen = 21;
            }
        }

        for (uint8_t i = 0; i < ui8ParaLen/3*3; i+= 3 )
        {
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD24 | p_ui8Para[i] << 16 | p_ui8Para[i+1] << 8 | p_ui8Para[i+2]);
        }

        if (ui8ParaLen % 3 == 2)
        {
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD16 | p_ui8Para[ui8ParaLen-2] << 8 | p_ui8Para[ui8ParaLen-1]);
        }
        else if (ui8ParaLen % 3 == 1)
        {
            nemadc_MIPI_out(ui32Cmd | p_ui8Para[ui8ParaLen-1]);
        }
        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        nemadc_MIPI_CFG_out(ui32Cfg);
    }
    return ui32Status;
}

//*****************************************************************************
//
//! @brief  mask valid bitfields
//!
//! @param  ui8DataLen    - length of data to be read
//!
//! @return uint32_t.
//
//****************************************************************************
static inline uint32_t
mask_valid(uint8_t ui8DataLen)
{
    uint32_t mask = 0;
    switch ( ui8DataLen )
    {
        case 1 :
            mask = 0xff;
            break;
        case 2 :
            mask = 0xffff;
            break;
        case 3 :
            mask = 0xffffff;
            break;
        case 4 :
            mask = 0xffffffff;
            break;
        default :
            mask = 0xff;
            break;
    }
    return mask;
}

//*****************************************************************************
//
//! @brief  Send DSI DCS(Display Command Set) read commands
//!
//! @param  ui8Cmd        - command
//! @param  ui8DataLen    - length of data to be read,it should be greater than zero.
//! @param  ui32Received  - received data.
//! @param  bHS           - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI DCS read commands.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_dcs_read(uint8_t ui8Cmd, uint8_t ui8DataLen, uint32_t* ui32Received, bool bHS)
{
    uint32_t ui32Cfg = 0;
    //
    // set return packet size (bytes)
    //
    am_hal_dsi_set_return_size(ui8DataLen, bHS);

    if (bHS == true) // high speed mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct(NemaDC_dt_DCS_read_param_no,
                  NemaDC_dt_DCS_read_param_no,
                  NemaDC_dcs_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    if(ui8DataLen == 1)
    {
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID | MIPICFG_SPI_HOLD);

        nemadc_MIPI_cmd(NemaDC_DBI_read | ui8Cmd);
        //
        // Send command-parameter packet.
        //
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID);
    }
    else
    {
        //
        // Send command-parameter packet.
        //
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID);

        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        // Force Chip select Low.
        // This will keep CS low from command phase to read data phase.
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID | MIPICFG_FRC_CSX_0);

        nemadc_MIPI_cmd(NemaDC_DBI_read | ui8Cmd);

        for (uint8_t i = 0; i < ui8DataLen - 1; i++)
        {
            //
            // Return directly if timeout
            //
            if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
            {
                return AM_HAL_STATUS_TIMEOUT;
            }
            nemadc_reg_write(NEMADC_REG_INTERFACE_RDAT, 0);
        }
    }
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    //
    // Release Chip select.
    // This will release CS since read operation has finished Applicable only for DBIB read case.
    //
    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    *ui32Received = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & mask_valid(ui8DataLen);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Send DSI generic read command.
//!
//! @param  p_ui8Para           - pointer of parameters to be sent
//! @param  ui8ParaLen          - length of parameters to be sent
//! @param  ui8DataLen          - length of data to be read,it should be greater than zero.
//! @param  ui32Received        - received data.
//! @param  bHS                 - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI generic read command.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_generic_read(uint8_t *p_ui8Para, uint8_t ui8ParaLen, uint8_t ui8DataLen, uint32_t* ui32Received, bool bHS)
{
    uint32_t ui32Cfg = 0;
    //
    // set return packet size (bytes)
    //
    am_hal_dsi_set_return_size(ui8DataLen, bHS);

    nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x6)); // set vc_no to 00.

    if (bHS == true) // high speed mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct((uint32_t)0,  // Unused parameter
                  (uint32_t)0,  // Unused parameter
                   NemaDC_ge_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // send command
    //
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID | MIPICFG_FRC_CSX_0);
    if (ui8ParaLen == 0) // Generic read, no parameter
    {
        nemadc_MIPI_cmd(NemaDC_DBI_read);
    }
    else
    {
        for (uint8_t i = 0; i < ui8ParaLen; i++)
        {
            nemadc_MIPI_cmd(NemaDC_DBI_read | p_ui8Para[i]);
        }
    }

    if (ui8DataLen == 1) 
    {
        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        nemadc_reg_write(NEMADC_REG_DBIB_RDAT, 0);
    }
    else
    {
        for (uint8_t i = 0; i < ui8DataLen - 1; i++)
        {
            //
            // Return directly if timeout
            //
            if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
            {
                return AM_HAL_STATUS_TIMEOUT;
            }
            nemadc_reg_write(NEMADC_REG_DBIB_RDAT, 0);
        }
    }

    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // return read parameters
    //
    *ui32Received = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & mask_valid(ui8DataLen);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Send SPI4/DSPI/QSPI/DSI read command via Display Controller(DC)
//!
//! @param  ui8Command          - command
//! @param  p_ui8Para           - pointer of parameters to be sent
//! @param  ui8ParaLen          - length of parameters to be sent
//! @param  p_ui32Data          - pointer of data to be read
//! @param  ui8DataLen          - length of data to be read (number of bytes)
//! @param  bDCS                - DCS command or generic command
//! @param  bHS                 - high speed mode or low power mode (escape mode)
//!
//! This function sends read commands to display drive IC.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
uint32_t
nemadc_mipi_cmd_read(uint8_t ui8Command,
                     uint8_t* p_ui8Para,
                     uint8_t ui8ParaLen,
                     uint32_t* p_ui32Data,
                     uint8_t ui8DataLen,
                     bool bDCS,
                     bool bHS)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            //
            // Enable DSI read,otherwise enable TE interruption
            //
            nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x20));

            am_hal_dsi_pre_rw_cmd(bHS);

            if (bDCS)
            {
                ui32Status = dsi_dcs_read(ui8Command, ui8DataLen, p_ui32Data, bHS);
            }
            else
            {
                ui32Status = dsi_generic_read(p_ui8Para, ui8ParaLen, ui8DataLen, p_ui32Data, bHS);
            }
            //
            // Enable TE interruption,otherwise enable DSI read.
            //
            nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x20);

        }
        else
        {
            ui32Status = dbi_read(ui8Command, ui8DataLen, p_ui32Data);
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {   
        //
        // QSPI/DSPI/SPI4 interface.
        //
        uint32_t ui32Cmd = 0;
        //
        // Enable Clock
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
        if ((ui32Cfg & MIPICFG_QSPI) != 0)
        {
            //
            // QSPI interface.
            //
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            ui32Cmd = MIPI_DBIB_CMD | MIPI_MASK_QSPI;
            nemadc_MIPI_out(ui32Cmd | CMD1_RDAT1);
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD16 | ui8Command);
        }
        else
        {
            if ((ui32Cfg & MIPICFG_DSPI) != 0)
            {
                //
                // DSPI interface
                //
                nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_DSPI));
            }
            else
            {
                //
                // SPI4 interface
                // 
                nemadc_MIPI_CFG_out(ui32Cfg);
            }
            ui32Cmd = MIPI_DBIB_CMD | ui8Command;
        }

        if (ui8DataLen == 1)
        {
            nemadc_MIPI_out(ui32Cmd | NemaDC_DBI_read);
        }
        else if (ui8DataLen == 2)
        {
            nemadc_MIPI_out(ui32Cmd | NemaDC_DBI_read | NemaDC_rcmd16);
        }
        else if (ui8DataLen == 3)
        {
            nemadc_MIPI_out(ui32Cmd | NemaDC_DBI_read | NemaDC_rcmd24);
        }
        else
        {
            //
            //Read 33-bit, the first high bit is ignored.
            //
            nemadc_reg_write(NEMADC_REG_FORMAT_CTRL,(32 << 16));
            nemadc_MIPI_out(ui32Cmd | NemaDC_DBI_read | NemaDC_rcmd32);
        }

        if ((ui32Cfg & MIPICFG_QSPI) != 0)
        {
            nemadc_MIPI_CFG_out(ui32Cfg & ~MIPICFG_SPI_HOLD);
        }

        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, DC_STATUS_dbi_rd_wr_on))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }

        *p_ui32Data = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & mask_valid(ui8DataLen);
        nemadc_MIPI_CFG_out(ui32Cfg);

    }
    return ui32Status;
}


//*****************************************************************************
//
//! @brief DC's Vsync interrupt handle function
//!
//! @param  pvUnused            - invalid  parameter
//!
//! this function used to clear Vsync interrupt status bit.and execute callback.
//!
//! @return None.
//
//*****************************************************************************
static void
prvVsyncInterruptHandler(void *pvUnused)
{
    ++irq_count;

    /* Clear the interrupt */
    nemadc_reg_write(NEMADC_REG_INTERRUPT, nemadc_reg_read(NEMADC_REG_INTERRUPT) & (~(3UL << 4)));

    //
    // DC workarounds after transfer is finsihed
    //
    nemadc_transfer_frame_end();

    //
    // vsync interrupt callback function
    //
    if (nemadc_vsync_cb != NULL)
    {
        nemadc_vsync_cb(vsync_arg, 0);
    }
    else
    {
#  if WAIT_IRQ_BINARY_SEMAPHORE
        xSemaphoreGiveFromISR(xSemaphore_vsync, NULL);
#  endif
    }
}
//*****************************************************************************
//
//! @brief DC's TE interrupt handle function
//!
//! @param  pvUnused            - invalid  parameter
//!
//! this function used to clear TE interrupt status bit.and execute callback.
//!
//! @return None.
//
//*****************************************************************************
static void
prvTEInterruptHandler(void *pvUnused)
{
    if (bNeedLaunchInTe)
    {
        nemadc_transfer_frame_launch();
        bNeedLaunchInTe = false;
    }
    ++irq_count;

    /* Clear the interrupt */
    nemadc_reg_write(NEMADC_REG_INTERRUPT, nemadc_reg_read(NEMADC_REG_INTERRUPT) & (~(1U << 3)));

    //
    // TE interrupt callback function
    //
    if (nemadc_te_cb != NULL)
    {
        nemadc_te_cb(NULL, 0);
    }
    else
    {
#if WAIT_IRQ_BINARY_SEMAPHORE
        xSemaphoreGiveFromISR(xSemaphore_TE, NULL);
#endif
    }
}

//*****************************************************************************
//
//! @brief Display Controller interrupt service function.
//!
//! this function will be called automatically when DC's interrupt(s) arrived.
//!
//! @return None.
//
//*****************************************************************************
void
am_disp_isr()
{
#ifdef SYSTEM_VIEW
        traceISR_ENTER();
#endif
    //
    // spi frame end interrupt or frame end interrupt
    //
    if ((nemadc_reg_read(NEMADC_REG_INTERRUPT) & (3UL << 4)) != 0)
    {
        prvVsyncInterruptHandler(NULL);
    }
    if ((nemadc_reg_read(NEMADC_REG_INTERRUPT) & (1UL << 3)) != 0)
    {
        prvTEInterruptHandler(NULL);
    }

#ifdef SYSTEM_VIEW
    traceISR_EXIT();
#endif
}

//*****************************************************************************
//
//! @brief Display Controller initialize function.
//!
//! @return None.
//
//*****************************************************************************
int32_t
nemadc_sys_init(void)
{
    //
    // Enable the display controller clock
    //
    CLKGEN->CLKCTRL_b.DISPCTRLCLKEN = CLKGEN_CLKCTRL_DISPCTRLCLKEN_ENABLE;
    // xil_printf( "nemadc_sys_init()\r\n" );
    nemadc_regs = (uintptr_t)NEMADC_BASEADDR;

    /* Clear the interrupt */
    nemadc_reg_write(NEMADC_REG_INTERRUPT, 0);

    //
    // Enable TE interruption
    //
    nemadc_reg_write(NEMADC_REG_GPIO, 0x20);

    /* Install Interrupt Handler */
    NVIC_SetPriority(NEMADC_IRQ, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(NEMADC_IRQ);

#if WAIT_IRQ_BINARY_SEMAPHORE
    if ((nemadc_vsync_cb == NULL) && (xSemaphore_vsync == NULL))
    {
        xSemaphore_vsync = xSemaphoreCreateBinary();
    }
    if (xSemaphore_TE == NULL)
    {
        xSemaphore_TE = xSemaphoreCreateBinary();
    }
#endif

    return 0;
}

//*****************************************************************************
//
//! @brief Wait for Vsync interrupt.
//!
//! this function invokes to wait Vsync interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_wait_vsync(void)
{
    /* Wait for the interrupt */
#if WAIT_IRQ_POLL == 1
    uint32_t ui32usMaxDelay = 3000000; // 1 sec
    uint32_t ui32Status;
    //
    // waiting TE interrupt bit reset,if it not zero.
    //
    ui32Status = am_hal_delay_us_status_change(50000, (uint32_t)&DC->INTERRUPT, 1UL << 3, 0);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return;
    }
    //irq_handler sets NEMADC_REG_INTERRUPT to 0. Poll until this happens

    //
    // bit 4 is frame end interrupt(JDI,DPI,DBI) or bit 5 is spi frame end interrupt(SPI4,DSPI,QSPI)
    //
    ui32Status = am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->INTERRUPT, 3UL << 4, 0);

    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return;
    }
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DSI or not.
    //
    if ((ui32Cfg & MIPICFG_DBI_EN) && (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN)))
    {
        //
        // wait lane 0 return stop state
        //
        ui32Status = am_hal_dsi_wait_stop_state(0);
        if (ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            return;
        }
    }
    else if (0x3FF & nemadc_reg_read(NEMADC_REG_FORMAT_CTRL3))
    {
        wait_mip_idle();
    }
#endif // WAIT_IRQ_POLL

#if WAIT_IRQ_BINARY_SEMAPHORE == 1
    xSemaphoreTake( xSemaphore_vsync, portMAX_DELAY );
#endif
}

//*****************************************************************************
//
//! @brief Wait for TE interrupt.
//!
//! this function invokes to Wait TE interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_wait_te(void)
{
    /* Wait for the interrupt */
#if WAIT_IRQ_POLL == 1
    uint32_t ui32usMaxDelay = 100000;
    uint32_t ui32Status;
    //irq_handler sets NEMADC_REG_INTERRUPT to 0. Poll until this happens
    ui32Status = am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->INTERRUPT, 1UL << 3, 0);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return;
    }
#endif // WAIT_IRQ_POLL

#if WAIT_IRQ_BINARY_SEMAPHORE == 1
    xSemaphoreTake( xSemaphore_TE, portMAX_DELAY );
#endif // WAIT_IRQ_BINARY_SEMAPHORE
}

//*****************************************************************************
//
//! @brief Read NemaDC Hardware Register
//!
//! @param  reg            - Register address
//!
//! this function invokes to Read DC register.
//!
//! @return value          - Register Value
//
//*****************************************************************************
uint32_t
nemadc_reg_read(uint32_t reg)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(nemadc_regs + reg);
    return *ptr;
}

//*****************************************************************************
//
//! @brief Write NemaDC Hardware Register
//!
//! @param  reg            - Register address
//! @param  value            - Register Value
//!
//! this function invokes to write DC register.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_reg_write(uint32_t reg, uint32_t value)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(nemadc_regs + reg);
    *ptr = value;
}

#if 0
//*****************************************************************************
//
//! @brief nemadc_get_vsync
//!
//!
//! @return 32-bit am_hal_status_e status
//! @return AM_HAL_STATUS_IN_USE means NemaDC is still in processing
//! @return AM_HAL_STATUS_SUCCESS means NemaDC is complete all operation
//
//*****************************************************************************
am_hal_status_e
nemadc_get_vsync(void)
{
    if (nemadc_reg_read(NEMADC_REG_INTERRUPT) & 1<<5)
    {
        return AM_HAL_STATUS_IN_USE;
    }
    else
    {
        return AM_HAL_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
//! @brief nemadc_get_te
//!
//!
//! @return 32-bit am_hal_status_e status
//! @return AM_HAL_STATUS_IN_USE means TE is not arrived
//! @return AM_HAL_STATUS_SUCCESS means TE is arrived
//
//*****************************************************************************
am_hal_status_e
nemadc_get_te(void)
{
    if (nemadc_reg_read(NEMADC_REG_INTERRUPT) & 1<<3)
    {
        return AM_HAL_STATUS_IN_USE;
    }
    else
    {
        return AM_HAL_STATUS_SUCCESS;
    }
}
#endif
