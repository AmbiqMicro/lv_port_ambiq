//*****************************************************************************
//
//! @file am_hal_sysctrl.c
//!
//! @brief Functions for interfacing with the M4F system control registers
//!
//! @addtogroup sysctrl5 SYSCTRL - System Control
//! @ingroup apollo5b_hal
//! @{
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
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_hal_sysctrl_ton_config.h"
#include "am_hal_sysctrl_clk_mux_reset.h"

//*****************************************************************************
//
// Defines
//
//*****************************************************************************
//! @cond SYSCTRL_TON_CONFIG
#define AM_SIMOBUCK_SCHEME_HIGH_EFFICIENCY 0
//! @endcond SYSCTRL_TON_CONFIG

//*****************************************************************************
//
//  Globals
//
//*****************************************************************************

extern void buck_ldo_update_override(bool bEnable);

//! @cond SYSCTRL_TON_CONFIG
//
// TON Configuration Tables
//
#if (AM_SIMOBUCK_SCHEME_HIGH_EFFICIENCY == 0)
static const am_hal_sysctrl_ton_levels_t ui8TonActiveConfig[SYSCTRL_GPU_TON_POWER_STATE_MAX][SYSCTRL_CPU_TON_POWER_STATE_MAX] =
{
    {SYSCTRL_TON_LEVEL_LOW, SYSCTRL_TON_LEVEL_LOW,  SYSCTRL_TON_LEVEL_HIGH},
    {SYSCTRL_TON_LEVEL_LOW, SYSCTRL_TON_LEVEL_LOW,  SYSCTRL_TON_LEVEL_HIGH},
    {SYSCTRL_TON_LEVEL_LOW, SYSCTRL_TON_LEVEL_HIGH, SYSCTRL_TON_LEVEL_HIGH},
};
#else
static const am_hal_sysctrl_ton_levels_t ui8TonActiveConfig[SYSCTRL_GPU_TON_POWER_STATE_MAX][SYSCTRL_CPU_TON_POWER_STATE_MAX] =
 {
    {SYSCTRL_TON_LEVEL_LOW, SYSCTRL_TON_LEVEL_LOW,  SYSCTRL_TON_LEVEL_LOW },
    {SYSCTRL_TON_LEVEL_LOW, SYSCTRL_TON_LEVEL_LOW,  SYSCTRL_TON_LEVEL_LOW },
    {SYSCTRL_TON_LEVEL_LOW, SYSCTRL_TON_LEVEL_LOW,  SYSCTRL_TON_LEVEL_HIGH},
};
#endif

bool g_bFrcBuckAct  = false;

//! @cond SYSCTRL_CLK_MUX_RESET
static uint8_t g_ui8ClkmuxrstClkNeeded[SYSCTRL_CLKMUXRST_CLK_MAX] = {0};
//! @endcond SYSCTRL_CLK_MUX_RESET

//*****************************************************************************
//
//! @brief Control the buck state in deepsleep
//!
//! @param bFrcBuckAct - True for forcing buck active in deepsleep
//!                    - False for not forcing buck active in deepsleep
//!
//! If you want to manually control the buck state in deepsleep mode,
//! am_hal_sysctrl_force_buck_active_in_deepsleep must
//! be called for setting buck state before calling
//! am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP).
//
//*****************************************************************************
void
am_hal_sysctrl_force_buck_active_in_deepsleep(bool bFrcBuckAct)
{
    g_bFrcBuckAct = bFrcBuckAct;
}

//
// Simobuck Ton atcive trim values
//
static uint8_t g_ui8VddcActLowTonTrim    = 0x0D;
static uint8_t g_ui8VddcActHighTonTrim   = 0x0D;
static uint8_t g_ui8VddcLvActLowTonTrim  = 0x0B;
static uint8_t g_ui8VddcLvActHighTonTrim = 0x0B;
static uint8_t g_ui8VddfActLowTonTrim    = 0x12;
static uint8_t g_ui8VddfActHighTonTrim   = 0x19;

//! @endcond SYSCTRL_TON_CONFIG

//*****************************************************************************
//
// Place the core into sleep or deepsleep.
//
// This function puts the MCU to sleep or deepsleep depending on bSleepDeep.
//
// Valid values for bSleepDeep are:
//     AM_HAL_SYSCTRL_SLEEP_NORMAL
//     AM_HAL_SYSCTRL_SLEEP_DEEP
//
//*****************************************************************************
void
am_hal_sysctrl_sleep(bool bSleepDeep)
{
    bool bBuckIntoLPinDS = false, bSimobuckAct = false;
    am_hal_pwrctrl_pwrmodctl_cpdlp_t sActCpdlpConfig;

    //
    // Disable interrupts and save the previous interrupt state.
    //
    AM_CRITICAL_BEGIN

    //
    // Get the current CPDLPSTATE configuration in active mode
    //
    am_hal_pwrctrl_pwrmodctl_cpdlp_get(&sActCpdlpConfig);

    //
    // Get current mode.
    //
    bSimobuckAct = ( PWRCTRL->VRSTATUS_b.SIMOBUCKST == PWRCTRL_VRSTATUS_SIMOBUCKST_ACT );

    //
    // If the user selected DEEPSLEEP and OTP is off, setup the system to enter DEEP SLEEP.
    // CPU cannot go to deepsleep if OTP is still powered on
    //
    if ((bSleepDeep == AM_HAL_SYSCTRL_SLEEP_DEEP)
        && (!PWRCTRL->DEVPWRSTATUS_b.PWRSTOTP))
    {
        //
        // Set the CPDLPSTATE configuration in deepsleep mode
        //
        am_hal_pwrctrl_pwrmodctl_cpdlp_t sDSCpdlpConfig =
        {
            .eRlpConfig = sActCpdlpConfig.eRlpConfig,
            .eElpConfig = AM_HAL_PWRCTRL_ELP_RET,
            .eClpConfig = AM_HAL_PWRCTRL_CLP_RET
        };
        am_hal_pwrctrl_pwrmodctl_cpdlp_config(sDSCpdlpConfig);
#if NO_TEMPSENSE_IN_DEEPSLEEP
        //
        // Restore VDDC and VDDF if temperature sensing is disabled in deepsleep.
        //
        vddc_vddf_setting_restore();
#endif
        //
        // Check if SIMOBUCK needs to stay in Active mode in DeepSleep
        //
        if ( bSimobuckAct )
        {
            //
            // Check if SIMOBUCK would go into LP mode in DeepSleep
            //
            if ( !(PWRCTRL->AUDSSPWRSTATUS &
                    (PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDADC_Msk |
                    PWRCTRL_AUDSSPWRSTATUS_PWRSTI2S1_Msk   |
                    PWRCTRL_AUDSSPWRSTATUS_PWRSTI2S0_Msk   |
                    PWRCTRL_AUDSSPWRSTATUS_PWRSTPDM0_Msk)) &&
                !(PWRCTRL->DEVPWRSTATUS &
                    (PWRCTRL_DEVPWRSTATUS_PWRSTDBG_Msk      |
                    PWRCTRL_DEVPWRSTATUS_PWRSTUSBPHY_Msk   |
                    PWRCTRL_DEVPWRSTATUS_PWRSTUSB_Msk      |
                    PWRCTRL_DEVPWRSTATUS_PWRSTSDIO1_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTSDIO0_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTCRYPTO_Msk   |
                    PWRCTRL_DEVPWRSTATUS_PWRSTDISPPHY_Msk  |
                    PWRCTRL_DEVPWRSTATUS_PWRSTDISP_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTGFX_Msk      |
                    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI3_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI2_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI1_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTMSPI0_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTADC_Msk      |
                    PWRCTRL_DEVPWRSTATUS_PWRSTUART3_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTUART2_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTUART1_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTUART0_Msk    |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM7_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM6_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM5_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM4_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM3_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM2_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM1_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOM0_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOSFD0_Msk   |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOSFD1_Msk   |
                    PWRCTRL_DEVPWRSTATUS_PWRSTIOS0_Msk     |
                    PWRCTRL_DEVPWRSTATUS_PWRSTOTP_Msk ))      &&
                //
                // Check and confirm Simobuck/LDO is forced active if PLL is enabled
                //
                ( MCUCTRL->PLLCTL0_b.SYSPLLPDB != MCUCTRL_PLLCTL0_SYSPLLPDB_ENABLE ))
            {
                if (!g_bFrcBuckAct)
                {
                    //
                    // This implies upon deepsleep, buck can transition into LP mode
                    //
                    bBuckIntoLPinDS = true;
                    //
                    // Remove overrides to allow buck to go in LP mode
                    //
                    buck_ldo_update_override(false);

#if AM_HAL_PWRCTRL_SIMOLP_AUTOSWITCH
                    if (g_ui32TrimVer >= 2)
                    {
                        //
                        // Toggle overrides to make sure buck goes to LP regardless of the state from last time
                        //
                        PWRCTRL->TONCNTRCTRL |= (PWRCTRL_TONCNTRCTRL_LPMODESWOVR_Msk | PWRCTRL_TONCNTRCTRL_ENABLELPOVR_Msk);
                    }
#endif
                }
            }
        }

        //
        // Prepare the core for deepsleep (write 1 to the DEEPSLEEP bit).
        //
        SCB->SCR |= _VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
#if !AM_HAL_STALL_CPU_HPWAKE
        //
        // If in HP mode, we need to wait till HFRC2 is ready and CPU is fully back in HP mode, before attempting deepsleep
        //
        if (PWRCTRL->MCUPERFREQ_b.MCUPERFREQ == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
        {
            while ( PWRCTRL->MCUPERFREQ_b.MCUPERFSTATUS != AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE )
            {
                am_hal_delay_us(1);
            }
        }
#endif // !AM_HAL_STALL_CPU_HPWAKE
    }
    else
    {
        //
        // Set the CPDLPSTATE configuration in normal sleep mode
        //
        am_hal_pwrctrl_pwrmodctl_cpdlp_t sNSCpdlpConfig;
        if (PWRCTRL->CPUPWRCTRL_b.SLEEPMODE) // ARM sleep
        {
            sNSCpdlpConfig.eRlpConfig = sActCpdlpConfig.eRlpConfig;
            sNSCpdlpConfig.eElpConfig = AM_HAL_PWRCTRL_ELP_ON_CLK_OFF;
            sNSCpdlpConfig.eClpConfig = AM_HAL_PWRCTRL_CLP_ON_CLK_OFF;
        }
        else // Ambiq sleep
        {
            sNSCpdlpConfig.eRlpConfig = sActCpdlpConfig.eRlpConfig;
            sNSCpdlpConfig.eElpConfig = AM_HAL_PWRCTRL_ELP_ON_CLK_OFF; // or can leave at 0x0 as we will turn the clocks off at the source
            sNSCpdlpConfig.eClpConfig = AM_HAL_PWRCTRL_CLP_ON_CLK_OFF; // or can leave at 0x0 as we will turn the clocks off at the source
        }
        am_hal_pwrctrl_pwrmodctl_cpdlp_config(sNSCpdlpConfig);

        //
        // Prepare the core for normal sleep (write 0 to the DEEPSLEEP bit).
        //
        SCB->SCR &= ~_VAL2FLD(SCB_SCR_SLEEPDEEP, 1);
    }

    //
    // Before executing WFI, flush APB writes.
    //
    am_hal_sysctrl_sysbus_write_flush();

    //
    // Execute the sleep instruction.
    //
    __WFI();

    //
    // Upon wake, execute the Instruction Sync Barrier instruction.
    //
    __ISB();

#if AM_HAL_STALL_CPU_HPWAKE
    //
    // If in HP mode, we need to wait till HFRC2 is ready and CPU is fully back in HP mode
    //
    if ((bSleepDeep == AM_HAL_SYSCTRL_SLEEP_DEEP) && (PWRCTRL->MCUPERFREQ_b.MCUPERFREQ == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE))
    {
        while ( PWRCTRL->MCUPERFREQ_b.MCUPERFSTATUS != AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE )
        {
            am_hal_delay_us(1);
        }
    }
#endif // AM_HAL_STALL_CPU_HPWAKE

    if ( bBuckIntoLPinDS )
    {
        //
        // Re-enable overrides
        //
        buck_ldo_update_override(true);
    }

#if AM_HAL_PWRCTRL_SIMOLP_AUTOSWITCH
    if ((g_ui32TrimVer >= 2) && (PWRCTRL->TONCNTRCTRL & PWRCTRL_TONCNTRCTRL_LPMODESWOVR_Msk) && (PWRCTRL->TONCNTRCTRL & PWRCTRL_TONCNTRCTRL_ENABLELPOVR_Msk))
    {
        PWRCTRL->TONCNTRCTRL &= ~(PWRCTRL_TONCNTRCTRL_LPMODESWOVR_Msk | PWRCTRL_TONCNTRCTRL_ENABLELPOVR_Msk);
    }
#endif

    //
    // Restore the CPDLPSTATE
    //
    am_hal_pwrctrl_pwrmodctl_cpdlp_config(sActCpdlpConfig);

    //
    // Restore the interrupt state.
    //
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Enable the floating point module.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_enable(void)
{
    //
    //! Enable the EPU. See section 6.3 of the CM55 TRM.

    //
    SCB->CPACR |= (0xF << 20);
    __DSB();
    __ISB();
}

//*****************************************************************************
//
// Disable the floating point module.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_disable(void)
{
    //
    // Disable access to the FPU in both privileged and user modes.
    // NOTE: Write 0s to all reserved fields in this register.
    //
    SCB->CPACR |= ~(0xF << 20);
    __DSB();
    __ISB();
}

//*****************************************************************************
//
// Enable stacking of FPU registers on exception entry.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_stacking_enable(bool bLazy)
{
    uint32_t ui32fpccr;

    //
    // Set the requested FPU stacking mode in ISRs.
    //
    AM_CRITICAL_BEGIN
#define SYSCTRL_FPCCR_LAZY  (FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk)
    ui32fpccr  = FPU->FPCCR;
    ui32fpccr &= ~SYSCTRL_FPCCR_LAZY;
    ui32fpccr |= (bLazy ? SYSCTRL_FPCCR_LAZY : FPU_FPCCR_ASPEN_Msk);
    FPU->FPCCR = ui32fpccr;
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Disable FPU register stacking on exception entry.
//
//*****************************************************************************
void
am_hal_sysctrl_fpu_stacking_disable(void)
{
    //
    // Completely disable FPU context save on entry to ISRs.
    //
    AM_CRITICAL_BEGIN
    FPU->FPCCR &= ~SYSCTRL_FPCCR_LAZY;
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Issue a system wide reset using the AIRCR bit in the M4 system ctrl.
//
//*****************************************************************************
void
am_hal_sysctrl_aircr_reset(void)
{
    //
    // Set the system reset bit in the AIRCR register
    //
    __NVIC_SystemReset();
}

//! @cond SYSCTRL_TON_CONFIG
//*****************************************************************************
//
// Initialize Ton config handling during power up.
//
//*****************************************************************************
void am_hal_sysctrl_ton_config_init()
{
    //
    // Read Ton-Active-Trim values from registers during initialization
    //
    if (g_ui32TrimVer >= 1)
    {
        g_ui8VddcActLowTonTrim    = MCUCTRL->SIMOBUCK2_b.VDDCACTLOWTONTRIM;
        g_ui8VddcActHighTonTrim   = MCUCTRL->SIMOBUCK2_b.VDDCACTHIGHTONTRIM;
        g_ui8VddfActLowTonTrim    = MCUCTRL->SIMOBUCK7_b.VDDFACTLOWTONTRIM;
        g_ui8VddfActHighTonTrim   = MCUCTRL->SIMOBUCK6_b.VDDFACTHIGHTONTRIM;
        g_ui8VddcLvActLowTonTrim  = MCUCTRL->SIMOBUCK4_b.VDDCLVACTLOWTONTRIM;
        g_ui8VddcLvActHighTonTrim = MCUCTRL->SIMOBUCK4_b.VDDCLVACTHIGHTONTRIM;
    }
    else
    {
        g_ui8VddcActLowTonTrim    = SYSCTRL_VDDCACTLOWTONTRIM;
        g_ui8VddcActHighTonTrim   = SYSCTRL_VDDCACTHIGHTONTRIM;
        g_ui8VddfActLowTonTrim    = SYSCTRL_VDDFACTLOWTONTRIM;
        g_ui8VddfActHighTonTrim   = SYSCTRL_VDDFACTHIGHTONTRIM;
        g_ui8VddcLvActLowTonTrim  = SYSCTRL_VDDCLVACTLOWTONTRIM;
        g_ui8VddcLvActHighTonTrim = SYSCTRL_VDDCLVACTHIGHTONTRIM;
    }
}

//*****************************************************************************
//
// Update SIMOBUCK Ton Values when CPU is active according to GPU state
//
//*****************************************************************************
void am_hal_sysctrl_ton_config_update(bool bGpuOn, am_hal_pwrctrl_gpu_mode_e eGpuSt)
{
    am_hal_sysctrl_gpu_ton_power_state eGpuTonSt = SYSCTRL_GPU_TON_POWER_STATE_OFF;

    //
    // Turn off SIMOBUCK before configuring TON
    //
    bool bSIMOBUCKInitialized = PWRCTRL->VRCTRL_b.SIMOBUCKEN;
    if ( bSIMOBUCKInitialized )
    {
        buck_ldo_update_override(false);
        am_hal_delay_us(5);
        PWRCTRL->VRCTRL_b.SIMOBUCKEN = 0;
    }

    //
    // Convert GPU state to corresponding GPU Ton state
    //
    if (bGpuOn)
    {
        eGpuTonSt = (eGpuSt == AM_HAL_PWRCTRL_GPU_MODE_LOW_POWER) ? SYSCTRL_GPU_TON_POWER_STATE_LP:
                                                                    SYSCTRL_GPU_TON_POWER_STATE_HP;
    }
    //
    // Check whether the Ton settings should be at higher/lower values for CPU
    // LP and HP mode, for the GPU state specified, from the power table.
    //
    bool bTonLPisLow = (ui8TonActiveConfig[eGpuTonSt][SYSCTRL_CPU_TON_POWER_STATE_LP] == SYSCTRL_TON_LEVEL_LOW);
    bool bTonHPisLow = (ui8TonActiveConfig[eGpuTonSt][SYSCTRL_CPU_TON_POWER_STATE_HP] == SYSCTRL_TON_LEVEL_LOW);

    //
    // Enable SIMOBUCK HP trims
    //
    MCUCTRL->SIMOBUCK1_b.SIMOBUCKHPTRIMEN = MCUCTRL_SIMOBUCK1_SIMOBUCKHPTRIMEN_SIMOBUCK_HP_ON;

    //
    // Update VDDC and VDDF TON trim values for CPU LP and HP mode when
    // SIMOBUCK is active
    //
    MCUCTRL->SIMOBUCK2_b.VDDCACTLOWTONTRIM    = bTonLPisLow ? g_ui8VddcActLowTonTrim :
                                                              g_ui8VddcActHighTonTrim;
    MCUCTRL->SIMOBUCK2_b.VDDCACTHIGHTONTRIM   = bTonHPisLow ? g_ui8VddcActLowTonTrim :
                                                              g_ui8VddcActHighTonTrim;
    MCUCTRL->SIMOBUCK7_b.VDDFACTLOWTONTRIM    = bTonLPisLow ? g_ui8VddfActLowTonTrim :
                                                              g_ui8VddfActHighTonTrim;
    MCUCTRL->SIMOBUCK6_b.VDDFACTHIGHTONTRIM   = bTonHPisLow ? g_ui8VddfActLowTonTrim :
                                                              g_ui8VddfActHighTonTrim;
    MCUCTRL->SIMOBUCK4_b.VDDCLVACTLOWTONTRIM  = bTonLPisLow ? g_ui8VddcLvActLowTonTrim :
                                                              g_ui8VddcLvActHighTonTrim;
    MCUCTRL->SIMOBUCK4_b.VDDCLVACTHIGHTONTRIM = bTonHPisLow ? g_ui8VddcLvActLowTonTrim :
                                                              g_ui8VddcLvActHighTonTrim;

    //
    // Turn on SIMOBUCK aftger configuring TON
    //
    if ( bSIMOBUCKInitialized )
    {
        PWRCTRL->VRCTRL_b.SIMOBUCKEN = 1;
        am_hal_delay_us(5);
        buck_ldo_update_override(true);
    }
}
//! @endcond SYSCTRL_TON_CONFIG

//! @cond SYSCTRL_CLK_MUX_RESET
//*****************************************************************************
//
// Update clocks needed for clock muxes reset operation
//
//*****************************************************************************
void am_hal_sysctrl_clkmuxrst_audadc_clkgen_off_update(bool bClkgenOff)
{
    AM_CRITICAL_BEGIN
    SNVR2REG->SNVR2_b.AUDADC_CLKGEN_OFF = bClkgenOff;
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Update clocks needed for clock muxes reset operation
//
//*****************************************************************************
void am_hal_sysctrl_clkmuxrst_pll_fref_update(MCUCTRL_PLLCTL0_FREFSEL_Enum eFref)
{
    AM_CRITICAL_BEGIN
    SNVR2REG->SNVR2_b.PLL_FREFSEL = (uint8_t) eFref;
    AM_CRITICAL_END
}

//*****************************************************************************
//
// Update clocks needed for clock muxes reset operation
//
//*****************************************************************************
void am_hal_sysctrl_clkmuxrst_clkneeded_update(am_hal_sysctrl_clkmuxrst_clk_e eClk, uint8_t ui8ClkSrcBm)
{
    uint8_t ui8Idx;
    uint8_t ui8ClockNeeded = 0;

    AM_CRITICAL_BEGIN

    //
    // Update clock needed array
    //
    g_ui8ClkmuxrstClkNeeded[(uint8_t)eClk] = ui8ClkSrcBm;

    //
    // Compute and save clock needed
    //
    for (ui8Idx = 0; ui8Idx < SYSCTRL_CLKMUXRST_CLK_MAX; ui8Idx++)
    {
        ui8ClockNeeded |= g_ui8ClkmuxrstClkNeeded[ui8Idx];
    }
    SNVR2REG->SNVR2_b.HFRC_DED_NEEDED = (ui8ClockNeeded & SYSCTRL_CLKMUXRST_CLK_HFRC_DED) ? 1 : 0;
    SNVR2REG->SNVR2_b.HFRC2_NEEDED = (ui8ClockNeeded & SYSCTRL_CLKMUXRST_CLK_HFRC2) ? 1 : 0;
    SNVR2REG->SNVR2_b.XTAL_NEEDED = (ui8ClockNeeded & SYSCTRL_CLKMUXRST_CLK_XTAL) ? 1 : 0;
    SNVR2REG->SNVR2_b.EXTCLK_NEEDED = (ui8ClockNeeded & SYSCTRL_CLKMUXRST_CLK_EXTCLK) ? 1 : 0;
    SNVR2REG->SNVR2_b.PLL_NEEDED = (ui8ClockNeeded & SYSCTRL_CLKMUXRST_CLK_PLL) ? 1 : 0;

    AM_CRITICAL_END
}

//*****************************************************************************
//
// Handle clock muxes reset during low_power_init
//
//*****************************************************************************
void am_hal_sysctrl_clkmuxrst_low_power_init()
{
    //
    // Execute clock mux reset if this is not a POA reset and SNVR2 signature
    // matches
    //
    if (!RSTGEN->STAT_b.POASTAT && SNVR2REG->SNVR2_b.SIGNATURE == SYSCTRL_CLKMUXRST_SIGNATURE)
    {
        // If Clock Recovery is needed
        if (SNVR2REG->SNVR2_b.AUDADC_CLKGEN_OFF || SNVR2REG->SNVR2_b.HFRC_DED_NEEDED ||
            SNVR2REG->SNVR2_b.HFRC2_NEEDED || SNVR2REG->SNVR2_b.XTAL_NEEDED ||
            SNVR2REG->SNVR2_b.EXTCLK_NEEDED || SNVR2REG->SNVR2_b.PLL_NEEDED)
        {
            am_hal_gpio_pincfg_t sGpio15Cfg;

            // ----------------------------------------------------------------
            // Stage 1: Enable Clocks Needed
            // ----------------------------------------------------------------
            // Enable HFRC_DED if AUDADC clock gen mux was OFF or if HFRC_DED is
            // marked needed
            if (SNVR2REG->SNVR2_b.AUDADC_CLKGEN_OFF || SNVR2REG->SNVR2_b.HFRC_DED_NEEDED)
            {
                *(volatile uint32_t*)0x400200C0 |= 0x00000001;
            }

            // Switch to HFRC LL MUX to HFRC if AUDADC clock gen mux was OFF,
            if (SNVR2REG->SNVR2_b.AUDADC_CLKGEN_OFF)
            {
                MCUCTRL->PLLMUXCTL_b.AUDADCPLLCLKSEL = MCUCTRL_PLLMUXCTL_AUDADCPLLCLKSEL_HFRC;
                am_hal_sysctrl_sysbus_write_flush();
                am_hal_delay_us(1);
            }

            // If HFRC2 is marked needed, switch to HFRC LL Mux
            if (SNVR2REG->SNVR2_b.HFRC2_NEEDED)
            {
                CLKGEN->MISC_b.FRCHFRC2 = CLKGEN_MISC_FRCHFRC2_FRC;
                // Wait for HFRC2 Clock to be ready
                am_hal_delay_us_status_check(200,
                                             (uint32_t)&CLKGEN->CLOCKENSTAT,
                                             CLKGEN_CLOCKENSTAT_HFRC2READY_Msk,
                                             CLKGEN_CLOCKENSTAT_HFRC2READY_Msk,
                                             true);
                am_hal_delay_us(5);
            }

            // If XTAL is marked needed, kick start XTAL
            if (SNVR2REG->SNVR2_b.XTAL_NEEDED ||
                (SNVR2REG->SNVR2_b.PLL_NEEDED && SNVR2REG->SNVR2_b.PLL_FREFSEL == MCUCTRL_PLLCTL0_FREFSEL_XTAL32MHz))
            {
                am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
                am_hal_delay_us(1500);
            }

            // if EXTCLK is marked needed, backup GPIO 15 config and switch
            // function to ExtRefClk
            if (SNVR2REG->SNVR2_b.EXTCLK_NEEDED ||
                (SNVR2REG->SNVR2_b.PLL_NEEDED && SNVR2REG->SNVR2_b.PLL_FREFSEL == MCUCTRL_PLLCTL0_FREFSEL_EXTREFCLK))
            {
                am_hal_gpio_pincfg_t sExtRefClkCfg;
                sExtRefClkCfg.GP.cfg_b.uFuncSel = AM_HAL_PIN_15_REFCLK_EXT;
                am_hal_gpio_pinconfig_get(15, &sGpio15Cfg);
                am_hal_gpio_pinconfig(15, sExtRefClkCfg);
            }

            // if PLL is marked needed, start PLL in bypass mode
            if (SNVR2REG->SNVR2_b.PLL_NEEDED)
            {
                SYSPLLn(0)->PLLCTL0_b.SYSPLLVDDFPDNB = MCUCTRL_PLLCTL0_SYSPLLVDDFPDNB_ENABLE;
                SYSPLLn(0)->PLLCTL0_b.SYSPLLVDDHPDNB = MCUCTRL_PLLCTL0_SYSPLLVDDHPDNB_ENABLE;
                am_hal_sysctrl_sysbus_write_flush();
                am_hal_delay_us(5);
                SYSPLLn(0)->PLLCTL0_b.FOUTPOSTDIVPD  = MCUCTRL_PLLCTL0_FOUTPOSTDIVPD_ACTIVE;
                SYSPLLn(0)->PLLCTL0_b.FOUT4PHASEPD   = MCUCTRL_PLLCTL0_FOUT4PHASEPD_ACTIVE;
                SYSPLLn(0)->PLLCTL0_b.FREFSEL        = SNVR2REG->SNVR2_b.PLL_FREFSEL;
                SYSPLLn(0)->PLLCTL0_b.BYPASS         = 1;
                SYSPLLn(0)->PLLCTL0_b.SYSPLLPDB      = MCUCTRL_PLLCTL0_SYSPLLPDB_ENABLE;
            }

            // ----------------------------------------------------------------
            // Stage 2: Clock muxes reset
            // ----------------------------------------------------------------
            // Power on PDM, I2S, AUDADC, and USB
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_PDM0);
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_I2S0);
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_I2S1);
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_AUDADC);
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_USB);
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_USBPHY);
            am_hal_sysctrl_sysbus_write_flush();
            am_hal_delay_us(5);

            // Make sure the clock path form CLKGEN out to LL mux input is not
            // gated.
            PDMn(0)->CTRL_b.CLKEN = 1;
            I2Sn(0)->CLKCFG_b.MCLKEN = 1;
            I2Sn(1)->CLKCFG_b.MCLKEN = 1;
            USBn(0)->CLKCTRL_b.PHYREFCLKDIS = 0;

            // Generate APB Write Pulse to trigger AUDADC clock gen switching
            AUDADCn(0)->CFG_b.CLKSEL = AUDADC_CFG_CLKSEL_HFRC_48MHz;
            am_hal_sysctrl_bus_write_flush();
            am_hal_delay_us(1);

            // Switch HFRC LL MUX back to CLK_GEN if AUDADC clock gen mux was
            // OFF
            if ( SNVR2REG->SNVR2_b.AUDADC_CLKGEN_OFF )
            {
                MCUCTRL->PLLMUXCTL_b.AUDADCPLLCLKSEL = MCUCTRL_PLLMUXCTL_AUDADCPLLCLKSEL_CLKGEN;
                am_hal_sysctrl_sysbus_write_flush();
            }

            // Wait for LL Mux to be completed
            am_hal_delay_us(20);

            //Disable clock we have previously enabled
            PDMn(0)->CTRL_b.CLKEN = 1;
            I2Sn(0)->CLKCFG_b.MCLKEN = 1;
            I2Sn(1)->CLKCFG_b.MCLKEN = 1;
            USBn(0)->CLKCTRL_b.PHYREFCLKDIS = 0;

            // Power off PDM, I2S, AUDADC, and USB
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_PDM0);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_I2S0);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_I2S1);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_AUDADC);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_USB);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_USBPHY);

            // ----------------------------------------------------------------
            // Stage 3: Revert Clock Settings
            // ----------------------------------------------------------------
            // if PLL was enabled, power it off
            if (SNVR2REG->SNVR2_b.PLL_NEEDED)
            {
                SYSPLLn(0)->PLLCTL0_b.SYSPLLPDB      = MCUCTRL_PLLCTL0_SYSPLLPDB_DISABLE;
                SYSPLLn(0)->PLLCTL0_b.BYPASS         = 0;
                SYSPLLn(0)->PLLCTL0_b.FREFSEL        = MCUCTRL_PLLCTL0_FREFSEL_XTAL32MHz;
                SYSPLLn(0)->PLLCTL0_b.FOUT4PHASEPD   = MCUCTRL_PLLCTL0_FOUT4PHASEPD_POWERDOWN;
                SYSPLLn(0)->PLLCTL0_b.FOUTPOSTDIVPD  = MCUCTRL_PLLCTL0_FOUTPOSTDIVPD_POWERDOWN;
                SYSPLLn(0)->PLLCTL0_b.SYSPLLVDDFPDNB = MCUCTRL_PLLCTL0_SYSPLLVDDFPDNB_DISABLE;
                SYSPLLn(0)->PLLCTL0_b.SYSPLLVDDHPDNB = MCUCTRL_PLLCTL0_SYSPLLVDDHPDNB_DISABLE;
            }

            // if EXTCLK was enabled, revert pin config
            if (SNVR2REG->SNVR2_b.EXTCLK_NEEDED ||
                (SNVR2REG->SNVR2_b.PLL_NEEDED && SNVR2REG->SNVR2_b.PLL_FREFSEL == MCUCTRL_PLLCTL0_FREFSEL_EXTREFCLK))
            {
                am_hal_gpio_pinconfig(15, sGpio15Cfg);
            }

            // If XTAL was started, power XTAL off
            if (SNVR2REG->SNVR2_b.XTAL_NEEDED ||
                (SNVR2REG->SNVR2_b.PLL_NEEDED && SNVR2REG->SNVR2_b.PLL_FREFSEL == MCUCTRL_PLLCTL0_FREFSEL_XTAL32MHz))
            {
                am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE, false);
            }

            // If HFRC2 was forced enabled, release the force
            if (SNVR2REG->SNVR2_b.HFRC2_NEEDED)
            {
                CLKGEN->MISC_b.FRCHFRC2 = CLKGEN_MISC_FRCHFRC2_NOFRC;
            }

            // If HFRC_DED was forced enabled, release the force
            if (SNVR2REG->SNVR2_b.AUDADC_CLKGEN_OFF || SNVR2REG->SNVR2_b.HFRC_DED_NEEDED)
            {
                *(volatile uint32_t*)0x400200C0 &= ~((uint32_t)0x00000001);
            }
        }
    }

    // Reinitialize clock mux memory
    SNVR2REG->SNVR2 = 0;
    SNVR2REG->SNVR2_b.SIGNATURE = SYSCTRL_CLKMUXRST_SIGNATURE;
}
//! @endcond SYSCTRL_CLK_MUX_RESET


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
