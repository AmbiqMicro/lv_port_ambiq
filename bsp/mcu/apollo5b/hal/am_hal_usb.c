//*****************************************************************************
//
//! @file am_hal_usb.c
//!
//! @brief Functions for USB module
//!
//! @addtogroup usb USB Functionality
//! @ingroup apollo5b_hal
//! @{

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "am_mcu_apollo.h"


//*****************************************************************
//
//! @name USB State
//! @{
//
//*****************************************************************
#define AM_HAL_MAGIC_USB 0xEA9E06

#define AM_HAL_USB_CHK_HANDLE(h)                                         \
((h) &&                                                                  \
((am_hal_handle_prefix_t *)(h))->s.bInit &&                              \
(((am_hal_handle_prefix_t *)(h))->s.magic == AM_HAL_MAGIC_USB))

#define AM_HAL_USB_ENTER_CRITICAL NVIC_DisableIRQ(USB0_IRQn)
#define AM_HAL_USB_EXIT_CRITICAL  NVIC_EnableIRQ(USB0_IRQn)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name Endpoint Masks
//!
//! maximum endpoint number is six (EP0 ~ EP5)
//! @{
//
//*****************************************************************************
#define AM_HAL_USB_EP_MASK       0x3F
#define AM_HAL_USB_CHK_EP(n)     ((n & ~AM_HAL_USB_EP_MASK) > 0)
#define AM_HAL_USB_CHK_EP_NUM(n) ((n & 0x7F) > 5)
#define AM_HAL_USB_CHK_USB(n)    (n & ~0xF)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name Endpoint Numbers
//!
//! Five endpoint number are available except EP0
//! @{
//
//*****************************************************************************
#define AM_HAL_USB_EP0_NUMBER    0x0
#define AM_HAL_USB_EP1_NUMBER    0x1
#define AM_HAL_USB_EP2_NUMBER    0x2
#define AM_HAL_USB_EP3_NUMBER    0x3
#define AM_HAL_USB_EP4_NUMBER    0x4
#define AM_HAL_USB_EP5_NUMBER    0x5
#define AM_HAL_USB_EP_MAX_NUMBER AM_HAL_USB_EP5_NUMBER
//*****************************************************************************
//! @}
//*****************************************************************************

#ifdef AM_HAL_USB_TEST_MODE_ENABLED
//
//! USB test mode defines
//
#define USB_REQ_SET_FEATURE         (3)
#define USB_REQ_FEATURE_TEST_MODE   (2)
#define USB_REQ_TEST_MODE_J         (1)
#define USB_REQ_TEST_MODE_K         (2)
#define USB_REQ_TEST_MODE_SE0NAK    (3)
#define USB_REQ_TEST_MODE_TESTPKT   (4)
#endif

//
//! USB workaround and feature macros
//

// Enable EP_OUT Timeout detection for DMA Mode 1. This is used to handle cases
// where host is sending data in multiple of MAX_PAYLOAD_SIZE without ZLP.
#define ENABLE_ADMA_OUT_XFER_END_DETECTION
#define AM_HAL_USB_TIMEOUT 2

#define SWAP_WORD(a)  (a << 24) | ((a << 8) & 0xff0000) | ((a >> 8) & 0xff00) | (a >> 24)

#undef AM_HAL_USB_FEATURE_ZERO_LENGTH_PACKET

//*****************************************************************************
//
//! Endpoint State
//
//*****************************************************************************
typedef enum
{
    AM_HAL_USB_EP0_STATE_IDLE,
    AM_HAL_USB_EP0_STATE_SETUP,
    AM_HAL_USB_EP0_STATE_DATA_RX,
    AM_HAL_USB_EP0_STATE_DATA_TX,
    AM_HAL_USB_EP0_STATE_STATUS_RX,
    AM_HAL_USB_EP0_STATE_STATUS_TX,
    AM_HAL_USB_EP0_STATE_NUM
}
am_hal_usb_ep0_state_e;

//*****************************************************************************
//
//! Endpoint Transfer Info
//
//*****************************************************************************
typedef struct
{
    uint8_t *buf;
    uint32_t len;
    uint32_t remaining;
    struct
    {
        uint8_t busy:1;
        uint8_t zlp:1;
        uint8_t dir:1;
        uint8_t lastpkt:1;
    } flags;

    #ifdef ENABLE_ADMA_OUT_XFER_END_DETECTION
    // Timeout counter for ADMA OUT_EP xfer
    uint32_t pLastBufAddr;
    uint32_t pStartBufAddr;
    uint32_t timeout_counter;
    #endif
}
am_hal_usb_ep_xfer_t;

//*****************************************************************************
//
//! USB Register State
//
//*****************************************************************************
typedef struct
{
    bool bValid;
    uint32_t regCFG0;
    uint32_t regCFG1;
    uint32_t regCFG2;
    struct
    {
        uint32_t regIDX0;
        uint32_t regIDX1;
        uint32_t regIDX2;
    } regEndPoints[AM_HAL_USB_EP_MAX_NUMBER];
}
am_hal_usb_register_state_t;

//*****************************************************************************
//
//! USB SOF Interrupt requests
//
//*****************************************************************************
typedef union
{
    uint8_t sofIntrReq;
    struct
    {
        uint8_t stack   : 1;
        uint8_t out_ep  : AM_HAL_USB_EP_MAX_NUMBER;
    }sofIntrReq_b;
}
am_hal_usb_sof_request_t;

//*****************************************************************************
//
//! USB Endpoint Double Buffer Enable
//
//*****************************************************************************
typedef union
{
    uint8_t en;
    struct
    {
        uint8_t ep1: 1;
        uint8_t ep2: 1;
        uint8_t ep3: 1;
        uint8_t ep4: 1;
        uint8_t ep5: 1;
    }en_b;
}
am_hal_usb_dbugger_enable_t;

typedef struct
{
    am_hal_usb_dbugger_enable_t in_ep;
    am_hal_usb_dbugger_enable_t out_ep;
}
am_hal_usb_dbuffer_config_t;

//*****************************************************************************
//
//! USB state data structure
//
//*****************************************************************************
typedef struct
{
    am_hal_handle_prefix_t prefix;
    am_hal_usb_register_state_t sRegState;

    uint32_t ui32Allocated;

    am_hal_usb_dev_speed_e eDevSpeed;
    am_hal_usb_dev_state_e eDevState;
    am_hal_usb_ep0_state_e eEP0State;

    uint16_t ep0_maxpacket;
    uint16_t epin_maxpackets[AM_HAL_USB_EP_MAX_NUMBER];
    uint16_t epout_maxpackets[AM_HAL_USB_EP_MAX_NUMBER];

    am_hal_usb_ep_xfer_t ep0_xfer;
    am_hal_usb_ep_xfer_t ep_xfers[AM_HAL_USB_EP_MAX_NUMBER][2];

#ifdef AM_HAL_USB_TEST_MODE_ENABLED
    bool bInTestMode;
#endif

    am_hal_usb_xfer_mode_e eOutXferMode;
    am_hal_usb_xfer_mode_e eInXferMode;

    uint32_t ui32Module;

    //! Device event callback functions
    am_hal_usb_dev_evt_callback dev_evt_callback;
    //! EP0 setup received callback
    am_hal_usb_ep0_setup_received_callback ep0_setup_callback;
    //! Endpoint transfer complete callback
    am_hal_usb_ep_xfer_complete_callback ep_xfer_complete_callback;

    bool bPendingInEndData;
    bool bPendingOutEndData;

    //! SOF interrupt requested
    am_hal_usb_sof_request_t sofReq;

    //! Double Buffer Setting for endpoints
    am_hal_usb_dbuffer_config_t sDBufferConfig;

    //! Keep pending OUT EP interrupt for later retrieval
    uint32_t ui32PendingIntrOut;
}
am_hal_usb_state_t;

static am_hal_usb_state_t g_am_hal_usb_states[AM_REG_USB_NUM_MODULES] = {0};

//*****************************************************************************
//
// USB DMA Mode 0 queue management variables.
//
//*****************************************************************************
typedef struct
{
    uint32_t                ui32BufAddr;
    uint32_t                ui32Count;
    uint8_t                 ui8EpNum;
    am_hal_usb_xfer_dir_e   eDir;
}
am_hal_usb_dma0_queue_t;

// Maximum queue size needed for USB DMA Mode 0
#define MAX_DMA0_QUEUE_SIZE (AM_HAL_USB_EP_MAX_NUMBER * 2)
static am_hal_queue_t g_sUSBDMA0Queue;
static am_hal_usb_dma0_queue_t g_psUSBDMA0QueueMem[MAX_DMA0_QUEUE_SIZE];
static bool g_bUSBDMA0Busy = false;



//*****************************************************************************
//
// CFG0 registers
//
//*****************************************************************************
//
//! @name FADDR register macros
//! @{
//
//*****************************************************************************
#define FADDR_FuncAddr(pUSB)                   *((volatile uint8_t *)&(pUSB->CFG0)) & USB_CFG0_FuncAddr_Msk >> USB_CFG0_FuncAddr_Pos
#define FADDR_FuncAddr_Set(pUSB, addr)         do { \
                                                   *((volatile uint8_t *)&(pUSB->CFG0)) &= ~USB_CFG0_FuncAddr_Msk; \
                                                   *((volatile uint8_t *)&(pUSB->CFG0)) |= (addr) << USB_CFG0_FuncAddr_Pos; \
                                               } while (0)

#define FADDR_Update(pUSB)                     (*((volatile uint8_t *)&(pUSB->CFG0)) & USB_CFG0_Update_Msk)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name POWER register macros
//! @{
//
//*****************************************************************************
#define POWER_IsoUpdate_Set(pUSB)              *((volatile uint8_t *)&(pUSB->CFG0) + 1) |=  (USB_CFG0_ISOUpdate_Msk >> 8)
#define POWER_IsoUpdate_Clear(pUSB)            *((volatile uint8_t *)&(pUSB->CFG0) + 1) &= ~(USB_CFG0_ISOUpdate_Msk >> 8)
#define POWER_AMSPECIFIC_Set(pUSB)             *((volatile uint8_t *)&(pUSB->CFG0) + 1) |=  (USB_CFG0_AMSPECIFIC_Msk >> 8)
#define POWER_AMSPECIFIC_Clear(pUSB)           *((volatile uint8_t *)&(pUSB->CFG0) + 1) &= ~(USB_CFG0_AMSPECIFIC_Msk >> 8)
#define POWER_HSEnab_Set(pUSB)                 *((volatile uint8_t *)&(pUSB->CFG0) + 1) |=  (USB_CFG0_HSEnab_Msk >> 8)
#define POWER_HSEnab_Clear(pUSB)               *((volatile uint8_t *)&(pUSB->CFG0) + 1) &= ~(USB_CFG0_HSEnab_Msk >> 8)
#define POWER_HSMode(pUSB)                     *((volatile uint8_t *)&(pUSB->CFG0) + 1) &   (USB_CFG0_HSMode_Msk >> 8)
#define POWER_Reset(pUSB)                      *((volatile uint8_t *)&(pUSB->CFG0) + 1) &   (USB_CFG0_Reset_Msk >> 8)
#define POWER_Resume_Set(pUSB)                 *((volatile uint8_t *)&(pUSB->CFG0) + 1) |=  (USB_CFG0_Resume_Msk >> 8)
#define POWER_Resume_Clear(pUSB)               *((volatile uint8_t *)&(pUSB->CFG0) + 1) &= ~(USB_CFG0_Resume_Msk >> 8)
#define POWER_SuspendMode(pUSB)                *((volatile uint8_t *)&(pUSB->CFG0) + 1) &   (USB_CFG0_Suspen_Msk >> 8)
#define POWER_EnableSuspendM_Set(pUSB)         *((volatile uint8_t *)&(pUSB->CFG0) + 1) |=  (USB_CFG0_Enabl_Msk >> 8)

#define INTRIN_Get(pUSB)                       (*((volatile uint16_t *)&(pUSB->CFG0) + 1) & AM_HAL_USB_EP_MASK)
#define INTRIN_Clear(pUSB)                     INTRIN_Get(pUSB)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
// CFG1 registers
//
//*****************************************************************************

//*****************************************************************************
//
//! @name INTROUT register
//! @{
//
//*****************************************************************************
#define INTROUT_Get(pUSB)                      (*((volatile uint8_t *)&(pUSB->CFG1)) & AM_HAL_USB_EP_MASK)
#define INTROUT_Clear(pUSB)                    INTROUT_Get(pUSB)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name INTRINE register
//! @{
//
//*****************************************************************************
#define INTRINE_Enable(pUSB, v)                *((volatile uint16_t *)&(pUSB->CFG1) + 1) |= ((v) & AM_HAL_USB_EP_MASK)
#define INTRINE_Disable(pUSB, v)               *((volatile uint16_t *)&(pUSB->CFG1) + 1) &= (~(v) & AM_HAL_USB_EP_MASK)
#define INTRINE_Get(pUSB)                      (*((volatile uint16_t *)&(pUSB->CFG1) + 1) & AM_HAL_USB_EP_MASK)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
// CFG2 registers
//
//*****************************************************************************

//*****************************************************************************
//
//! @name INTROUTE register
//! @{
//
//*****************************************************************************
#define INTROUTE_Enable(pUSB, v)               *((volatile uint8_t *)&(pUSB->CFG2)) |= ((v) & AM_HAL_USB_EP_MASK)
#define INTROUTE_Disable(pUSB, v)              *((volatile uint8_t *)&(pUSB->CFG2)) &= (~(v) & AM_HAL_USB_EP_MASK)
#define INTROUTE_Get(pUSB)                     (*((volatile uint8_t *)&(pUSB->CFG2)) & AM_HAL_USB_EP_MASK)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name INTRUSB register
//! @{
//
//*****************************************************************************
#define INTRUSB_Get(pUSB)                      (*((volatile uint8_t *)&(pUSB->CFG2) + 2) & 0xF)
#define INTRUSB_Clear(pUSB)                    INTRUSB_Get(pUSB)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name INTRUSBE register
//! @{
//
//*****************************************************************************
#define INTRUSBE_SOF_Enable(pUSB)              *((volatile uint8_t *)&(pUSB->CFG2) + 3) |=  ((uint8_t)(USB_CFG2_SOFE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_SOF_Disable(pUSB)             *((volatile uint8_t *)&(pUSB->CFG2) + 3) &= ~((uint8_t)(USB_CFG2_SOFE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_Reset_Enable(pUSB)            *((volatile uint8_t *)&(pUSB->CFG2) + 3) |=  ((uint8_t)(USB_CFG2_ResetE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_Reset_Disable(pUSB)           *((volatile uint8_t *)&(pUSB->CFG2) + 3) &= ~((uint8_t)(USB_CFG2_ResetE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_Resume_Enable(pUSB)           *((volatile uint8_t *)&(pUSB->CFG2) + 3) |=  ((uint8_t)(USB_CFG2_ResumeE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_Resume_Disable(pUSB)          *((volatile uint8_t *)&(pUSB->CFG2) + 3) &= ~((uint8_t)(USB_CFG2_ResumeE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_Suspend_Enable(pUSB)          *((volatile uint8_t *)&(pUSB->CFG2) + 3) |=  ((uint8_t)(USB_CFG2_SuspendE_Msk >> USB_CFG2_SuspendE_Pos))
#define INTRUSBE_Suspend_Disable(pUSB)         *((volatile uint8_t *)&(pUSB->CFG2) + 3) &= ~((uint8_t)(USB_CFG2_SuspendE_Msk >> USB_CFG2_SuspendE_Pos))

#define INTRUSBE_Enable(pUSB, v)               *((volatile uint8_t *)&(pUSB->CFG2) + 3) |= ((v) & 0xF)
#define INTRUSBE_Disable(pUSB, v)              *((volatile uint8_t *)&(pUSB->CFG2) + 3) &= (~(v) & 0xF)

#define INTRUSBE_Get(pUSB)                     (*((volatile uint8_t *)&(pUSB->CFG2) + 3) & 0xF)
//*****************************************************************************
//! @}
//*****************************************************************************

// CFG3 registers

//
//! Frame Number register
//
#define FRAME_NUM(pUSB)                        (pUSB->CFG3 & USB_CFG3_FRMNUM_Msk) >> USB_CFG3_FRMNUM_Pos //FIXME

//
//! Endpoint index register
//
#define EP_INDEX_Set(pUSB, idx)                 pUSB->CFG3 &= ~USB_CFG3_ENDPOINT_Msk; \
                                                pUSB->CFG3 |= (idx) << USB_CFG3_ENDPOINT_Pos

//*****************************************************************************
//
//! @name Test Mode register
//! @{
//
//*****************************************************************************
#define TESTMODE_TestSE0NAK_Set(pUSB)           pUSB->CFG3 |= USB_CFG3_TestSE0NAK_Msk
#define TESTMODE_TestJ_Set(pUSB)                pUSB->CFG3 |= USB_CFG3_TestJ_Msk
#define TESTMODE_TestK_Set(pUSB)                pUSB->CFG3 |= USB_CFG3_TestK_Msk
#define TESTMODE_TestPacket_Set(pUSB)           pUSB->CFG3 |= USB_CFG3_TestPacket_Msk
#define TESTMODE_ForceHS_Set(pUSB)              pUSB->CFG3 |= USB_CFG3_ForceHS_Msk
#define TESTMODE_ForceFS_Set(pUSB)              pUSB->CFG3 |= USB_CFG3_ForceFS_Msk
//*****************************************************************************
//! @}
//*****************************************************************************

//
// IDX0 registers
//

//*****************************************************************************
//
//! @name EP0 CSR0 register
//! @{
//
//*****************************************************************************
#define CSR0_ServicedSetupEnd_Set(pUSB)         pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_IncompTxServiceSetupEnd_Msk
#define CSR0_ServicedOutPktRdy_Set(pUSB)        pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_ClrDataTogServicedOutPktRdy_Msk
#define CSR0_ServicedOutPktRdy(pUSB)            pUSB->IDX0 & USB_IDX0_ClrDataTogServicedOutPktRdy_Msk
#define CSR0_SendStall_Set(pUSB)                pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_SentStallSendStall_Msk
#define CSR0_SetupEnd(pUSB)                    (pUSB->IDX0 &   USB_IDX0_SendStallSetupEnd_Msk)

#define CSR0_DataEnd_Set(pUSB)                  pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_FlushFIFODataEnd_Msk
#define CSR0_SentStall(pUSB)                   (pUSB->IDX0 &   USB_IDX0_UnderRunSentStall_Msk)
#define CSR0_SentStall_Clear(pUSB)              pUSB->IDX0 &= ~(USB_IDX0_UnderRunSentStall_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define CSR0_InPktRdy(pUSB)                    (pUSB->IDX0 &   USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define CSR0_InPktRdy_Set(pUSB)                 pUSB->IDX0 = (pUSB->IDX0 & ~USB_IDX0_InPktRdyOutPktRdy_Msk) | USB_IDX0_FIFONotEmptyInPktRdy_Msk
#define CSR0_OutPktRdy(pUSB)                   (pUSB->IDX0 &   USB_IDX0_InPktRdyOutPktRdy_Msk)
#define CSR0_OutPktRdy_Set(pUSB)                pUSB->IDX0 = (pUSB->IDX0 & ~USB_IDX0_FIFONotEmptyInPktRdy_Msk) | USB_IDX0_InPktRdyOutPktRdy_Msk
#define CSR0_ServicedOutPktRdyAndDataEnd_Set(pUSB)    pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | (USB_IDX0_ClrDataTogServicedOutPktRdy_Msk | USB_IDX0_FlushFIFODataEnd_Msk)
#define CSR0_InPktRdyAndDataEnd_Set(pUSB)             pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | (USB_IDX0_FIFONotEmptyInPktRdy_Msk | USB_IDX0_FlushFIFODataEnd_Msk)
#define CSR0_ServicedOutPktRdyAndSendStall_Set(pUSB)  pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | (USB_IDX0_ClrDataTogServicedOutPktRdy_Msk | USB_IDX0_SentStallSendStall_Msk)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name INMAXP register
//! @{
//
//*****************************************************************************
#define INMAXP_MaxPayload(pUSB)                (pUSB->IDX0 &   USB_IDX0_MAXPAYLOAD_Msk) >> USB_IDX0_MAXPAYLOAD_Pos
#define INMAXP_MaxPayload_Set(pUSB, maxp)       pUSB->IDX0 &= ~(USB_IDX0_MAXPAYLOAD_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk); \
                                                pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | (maxp) << USB_IDX0_MAXPAYLOAD_Pos
#define INMAXP_PktSplitOption(pUSB)            (pUSB->IDX0 &   USB_IDX0_PKTSPLITOPTION_Msk) >> USB_IDX0_PKTSPLITOPTION_Pos
#define INMAXP_PktSplitOption_Set(pUSB, split)  pUSB->IDX0 &= ~(USB_IDX0_PKTSPLITOPTION_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk); \
                                                pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | (split) << USB_IDX0_PKTSPLITOPTION_Pos
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name INCSRL register
//! @{
//
//*****************************************************************************
#define INCSRL_IncompTx(pUSB)                  (pUSB->IDX0 &   USB_IDX0_IncompTxServiceSetupEnd_Msk)
#define INCSRL_IncompTx_Clear(pUSB)             pUSB->IDX0 &= ~(USB_IDX0_IncompTxServiceSetupEnd_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRL_ClrDataTog_Set(pUSB)             pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_ClrDataTogServicedOutPktRdy_Msk
#define INCSRL_SentStall(pUSB)                 (pUSB->IDX0 &   USB_IDX0_SentStallSendStall_Msk)
#define INCSRL_SentStall_Clear(pUSB)            pUSB->IDX0 &= ~(USB_IDX0_SentStallSendStall_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRL_SendStall_Set(pUSB)              pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_SendStallSetupEnd_Msk
#define INCSRL_SendStall_Clear(pUSB)            pUSB->IDX0 &= ~(USB_IDX0_SendStallSetupEnd_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRL_FlushFIFO_Set(pUSB)              pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_FlushFIFODataEnd_Msk
#define INCSRL_UnderRun(pUSB)                  (pUSB->IDX0 &   USB_IDX0_UnderRunSentStall_Msk)
#define INCSRL_UnderRun_Clear(pUSB)             pUSB->IDX0 &= ~(USB_IDX0_UnderRunSentStall_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRL_FIFONotEmpty(pUSB)              (pUSB->IDX0 &   USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRL_FIFONotEmpty_Clear(pUSB)         pUSB->IDX0 &= ~(USB_IDX0_FIFONotEmptyInPktRdy_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRL_InPktRdy(pUSB)                  (pUSB->IDX0 &   USB_IDX0_InPktRdyOutPktRdy_Msk)
#define INCSRL_InPktRdy_Set(pUSB)               pUSB->IDX0 = (pUSB->IDX0 & ~USB_IDX0_FIFONotEmptyInPktRdy_Msk) | USB_IDX0_InPktRdyOutPktRdy_Msk
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name INCSRU register
//! @{
//
//*****************************************************************************
#define INCSRU_AutoSet(pUSB)                   (pUSB->IDX0 &   USB_IDX0_AutoSet_Msk)
#define INCSRU_AutoSet_Set(pUSB)                pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) |  USB_IDX0_AutoSet_Msk
#define INCSRU_AutoSet_Clear(pUSB)              pUSB->IDX0 &= ~(USB_IDX0_AutoSet_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRU_ISO_Set(pUSB)                    pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_ISO_Msk
#define INCSRU_ISO_Clear(pUSB)                  pUSB->IDX0 &= ~(USB_IDX0_ISO_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRU_Mode_Set(pUSB)                   pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_Mode_Msk
#define INCSRU_Mode_Clear(pUSB)                 pUSB->IDX0 &= ~(USB_IDX0_Mode_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRU_DMAReqEnab_Set(pUSB)             pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_DMAReqEnab_Msk
#define INCSRU_DMAReqEnab_Clear(pUSB)           pUSB->IDX0 &= ~(USB_IDX0_DMAReqEnab_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRU_FrcDataTog_Set(pUSB)             pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_FrcDataTog_Msk
#define INCSRU_FrcDataTog_Clear(pUSB)           pUSB->IDX0 &= ~(USB_IDX0_FrcDataTog_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)

#define INCSRU_DMAReqMode_Set(pUSB)             pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_DMAReqMode_Msk
#define INCSRU_DMAReqMode_Clear(pUSB)           pUSB->IDX0 &= ~(USB_IDX0_DMAReqMode_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
#define INCSRU_DPktBufDis_Set(pUSB)             pUSB->IDX0 = (pUSB->IDX0 & ~(USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)) | USB_IDX0_DPktBufDis_Msk
#define INCSRU_DPktBufDis_Clear(pUSB)           pUSB->IDX0 &= ~(USB_IDX0_DPktBufDis_Msk | USB_IDX0_InPktRdyOutPktRdy_Msk | USB_IDX0_FIFONotEmptyInPktRdy_Msk)
//*****************************************************************************
//! @}
//*****************************************************************************

//
// IDX1 registers
//

#define USB_IDX1_SET_MASK_WITH_PROTECT(msk)     pUSB->IDX1 |= ((msk) | USB_IDX1_OutPktRdy_Msk)
#define USB_IDX1_CLR_MASK_WITH_PROTECT(msk)     pUSB->IDX1  = ((pUSB->IDX1 & (~msk)) | USB_IDX1_OutPktRdy_Msk)

//*****************************************************************************
//
//! @name OUTMAXP register
//! @{
//
//*****************************************************************************
#define OUTMAXP_MaxPayload(pUSB)               (pUSB->IDX1 & USB_IDX1_MAXPAYLOAD_Msk) >> USB_IDX1_MAXPAYLOAD_Pos
#define OUTMAXP_MaxPayload_Set(pUSB, maxp)      USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_MAXPAYLOAD_Msk); \
                                                USB_IDX1_SET_MASK_WITH_PROTECT((maxp) << USB_IDX1_MAXPAYLOAD_Pos)

#define OUTMAXP_PktSplitOption(pUSB)           (pUSB->IDX1 & USB_IDX1_PKTSPLITOPTION_Msk) >> USB_IDX1_PKTSPLITOPTION_Pos
#define OUTMAXP_PktSplitOption_Set(pUSB, split) USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_PKTSPLITOPTION_Msk); \
                                                USB_IDX1_SET_MASK_WITH_PROTECT((split) << USB_IDX1_PKTSPLITOPTION_Pos)
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name OUTCSRL register
//! @{
//
//*****************************************************************************
#define OUTCSRL_ClrDataTog_Set(pUSB)           USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_ClrDataTog_Msk)
#define OUTCSRL_SentStall(pUSB)                (pUSB->IDX1 &   USB_IDX1_SentStall_Msk)
#define OUTCSRL_SentStall_Clear(pUSB)          USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_SentStall_Msk)
#define OUTCSRL_SendStall_Set(pUSB)            USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_SendStall_Msk)
#define OUTCSRL_SendStall_Clear(pUSB)          USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_SendStall_Msk)
#define OUTCSRL_FlushFIFO_Set(pUSB)            USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_FlushFIFO_Msk)
#define OUTCSRL_DataError(pUSB)                (pUSB->IDX1 &   USB_IDX1_DataError_Msk)
#define OUTCSRL_OverRun(pUSB)                  (pUSB->IDX1 &   USB_IDX1_OverRun_Msk)
#define OUTCSRL_OverRun_Clear(pUSB)            USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_OverRun_Msk)
#define OUTCSRL_FIFOFull(pUSB)                 (pUSB->IDX1 &   USB_IDX1_FIFOFull_Msk)
#define OUTCSRL_OutPktRdy(pUSB)                (pUSB->IDX1 &   USB_IDX1_OutPktRdy_Msk)
#define OUTCSRL_OutPktRdy_Clear(pUSB)          pUSB->IDX1 &= ~USB_IDX1_OutPktRdy_Msk
//*****************************************************************************
//! @}
//*****************************************************************************

//*****************************************************************************
//
//! @name OUTCSRU register
//! @{
//
//*****************************************************************************
#define OUTCSRU_AutoClear_Set(pUSB)            USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_AutoClear_Msk)
#define OUTCSRU_AutoClear_Clear(pUSB)          USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_AutoClear_Msk)
#define OUTCSRU_ISO_Set(pUSB)                  USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_ISO_Msk)
#define OUTCSRU_ISO_Clear(pUSB)                USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_ISO_Msk)
#define OUTCSRU_DMAReqEnab_Set(pUSB)           USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_DMAReqEnab_Msk)
#define OUTCSRU_DMAReqEnab_Clear(pUSB)         USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_DMAReqEnab_Msk)
#define OUTCSRU_DisNye_Set(pUSB)               USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_DisNye_Msk)
#define OUTCSRU_DisNye_Clear(pUSB)             USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_DisNye_Msk)
#define OUTCSRU_PIDErr(pUSB)                   (pUSB->IDX1 &   USB_IDX1_DisNye_Msk)
#define OUTCSRU_DMAReqMode_Set(pUSB)           USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_DMAReqMode_Msk)
#define OUTCSRU_DMAReqMode_Clear(pUSB)         USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_DMAReqMode_Msk)
#define OUTCSRU_DPktBufDis(pUSB)               (pUSB->IDX1 &   USB_IDX1_DPktBufDis_Msk)
#define OUTCSRU_DPktBufDis_Set(pUSB)           USB_IDX1_SET_MASK_WITH_PROTECT(USB_IDX1_DPktBufDis_Msk)
#define OUTCSRU_DPktBufDis_Clear(pUSB)         USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_DPktBufDis_Msk)
#define OUTCSRU_IncompRx(pUSB)                 (pUSB->IDX1 &   USB_IDX1_IncompRx_Msk)
#define OUTCSRU_IncompRx_Clear(pUSB)           USB_IDX1_CLR_MASK_WITH_PROTECT(USB_IDX1_IncompRx_Msk)
//*****************************************************************************
//! @}
//*****************************************************************************

//
// IDX2 register
//

#define OUTCOUNT(pUSB)                         (pUSB->IDX2 & USB_IDX2_ENDPTOUTCOUNT_Msk) >> USB_IDX2_ENDPTOUTCOUNT_Pos

//! EP0 count register
#define COUNT0(pUSB)                           (pUSB->IDX2 & USB_IDX2_ENDPTOUTCOUNT_Msk) >> USB_IDX2_ENDPTOUTCOUNT_Pos

//*****************************************************************************
//
//! @name DMA register
//! @{
//
//*****************************************************************************
#define DMAINTSTAT_Get(pUSB)                    (pUSB->DMAINTSTAT & (USB_DMAINTSTAT_DMACMP_Msk | USB_DMAINTSTAT_DMAERR_Msk) )
#define ADMACMPINTSTAT_Get(pUSB)                (pUSB->ADMACMPINTSTAT)
#define DMAINTSTAT_Clear(pUSB, mask)            (pUSB->DMAINTCLR = mask)

#define DMACTRL_DMAEN_Enable(pUSB)              (pUSB->DMACTRL  |= USB_DMACTRL_DMAEN_Msk)
#define DMACTRL_DMAEN_Disable(pUSB)             (pUSB->DMACTRL  &= ~USB_DMACTRL_DMAEN_Msk)

#define EARLY_DMA_IN_ENABLE(pUSB)              *((volatile uint8_t *)&(pUSB->INFO) + 3) |=  ((uint8_t)(USB_INFO_EDMAIN_Msk >> USB_INFO_EDMAOUT_Pos))
#define EARLY_DMA_IN_DISABLE(pUSB)             *((volatile uint8_t *)&(pUSB->INFO) + 3) &=  ~((uint8_t)(USB_INFO_EDMAIN_Msk >> USB_INFO_EDMAOUT_Pos))
#define EARLY_DMA_OUT_ENABLE(pUSB)             *((volatile uint8_t *)&(pUSB->INFO) + 3) |=  ((uint8_t)(USB_INFO_EDMAOUT_Msk >> USB_INFO_EDMAOUT_Pos))
#define EARLY_DMA_OUT_DISABLE(pUSB)            *((volatile uint8_t *)&(pUSB->INFO) + 3) &=  ~((uint8_t)(USB_INFO_EDMAOUT_Msk >> USB_INFO_EDMAOUT_Pos))



//*****************************************************************************
//
//! @name Dynamic FIFO size
//! @{
//
//*****************************************************************************
#define EP_FIFO_SZ_8                            0x0
#define EP_FIFO_SZ_16                           0x1
#define EP_FIFO_SZ_32                           0x2
#define EP_FIFO_SZ_64                           0x3
#define EP_FIFO_SZ_128                          0x4
#define EP_FIFO_SZ_256                          0x5
#define EP_FIFO_SZ_512                          0x6
#define EP_FIFO_SZ_1024                         0x7
#define EP_FIFO_SZ_2048                         0x8
#define EP_FIFO_SZ_4096                         0x9
//*****************************************************************************
//! @}
//*****************************************************************************

// Double packet buffering
#define FIFO_DOUBLE_PKTBUF                      0x1
#define FIFO_SINGLE_PKTBUF                      0x0

#define InFIFOsz(pUSB)                         (pUSB->IDX2 & USB_IDX2_INFIFOSZ_Msk) >> USB_IDX2_INFIFOSZ_Pos
#define InFIFOsz_Set(pUSB, dp, sz)              pUSB->IDX2 &= ~USB_IDX2_INFIFOSZ_Msk; \
                                                pUSB->IDX2 |= ((dp) << 4 | (sz)) << USB_IDX2_INFIFOSZ_Pos

#define OutFIFOsz(pUSB)                        (pUSB->IDX2 & USB_IDX2_OUTFIFOSZ_Msk) >> USB_IDX2_OUTFIFOSZ_Pos
#define OutFIFOsz_Set(pUSB, dp, sz)             pUSB->IDX2 &= ~USB_IDX2_OUTFIFOSZ_Msk; \
                                                pUSB->IDX2 |= ((dp) << 4 | (sz)) << USB_IDX2_OUTFIFOSZ_Pos

#define InFIFOadd(pUSB)                        (pUSB->FIFOADD & USB_FIFOADD_INFIFOADD_Msk) >> USB_FIFOADD_INFIFOADD_Pos
#define InFIFOadd_Set(pUSB, addr)               pUSB->FIFOADD &= ~USB_FIFOADD_INFIFOADD_Msk; \
                                                pUSB->FIFOADD |= (addr) << USB_FIFOADD_INFIFOADD_Pos;

#define OutFIFOadd(pUSB)                       (pUSB->FIFOADD & USB_FIFOADD_OUTFIFOADD_Msk) >> USB_FIFOADD_OUTFIFOADD_Pos
#define OutFIFOadd_Set(pUSB, addr)              pUSB->FIFOADD &= ~USB_FIFOADD_OUTFIFOADD_Msk; \
                                                pUSB->FIFOADD |= (addr) << USB_FIFOADD_OUTFIFOADD_Pos

// Endpoint FIFO registers
#define FIFOx_ADDR(pUSB, x)                    ((volatile uint32_t *)(&(pUSB->FIFO0) + x))

// HWVers register
#define HWVERS_RC(pUSB)                        (pUSB->HWVERS & USB_HWVERS_RC_Msk)   >> USB_HWVERS_RC_Pos
#define HWVERS_xx(pUSB)                        (pUSB->HWVERS & USB_HWVERS_xx_Msk)   >> USB_HWVERS_xx_Pos
#define HWVERS_yyy(pUSB)                       (pUSB->HWVERS & USB_HWVERS_yyy_Msk)  >> USB_HWVERS_yyy_Pos
#define HWVERS(pUSB)                            pUSB->HWVERS

// EPINFO register
#define EPINFO_OutEndPoints(pUSB)              (pUSB->INFO & USB_INFO_OutEndPoints_Msk) >> USB_INFO_OutEndPoints_Pos
#define EPINFO_InEndPoints(pUSB)               (pUSB->INFO & USB_INFO_InEndPoints_Msk) >> USB_INFO_InEndPoints_Pos

// RAMINFO register
#define RAMINFO_RamBits(pUSB)                  (pUSB->INFO & USB_INFO_RamBits_Msk)      >> USB_INFO_RamBits_Pos

//
//! Endpoint direction type
//
typedef enum
{
    AM_HAL_USB_EP_DIR_OUT     = 0,
    AM_HAL_USB_EP_DIR_IN      = 1,
    AM_HAL_USB_EP_DIR_IN_MASK = 0x80
}
am_hal_usb_ep_dir_e;

#define AM_HAL_USB_EP_XFER_MASK      0x3

//
//! Endpoint transfer type
//
typedef enum
{
    AM_HAL_USB_EP_XFER_CONTROL = 0 ,
    AM_HAL_USB_EP_XFER_ISOCHRONOUS ,
    AM_HAL_USB_EP_XFER_BULK        ,
    AM_HAL_USB_EP_XFER_INTERRUPT
}
am_hal_usb_ep_xfer_type_e;

//*****************************************************************************
//
// Static functions
//
//*****************************************************************************
static void am_hal_usb_xfer_complete(am_hal_usb_state_t *pState, am_hal_usb_ep_xfer_t *pXfer, uint8_t ui8EpAddr, uint32_t ui32XferLen, am_hal_usb_xfer_code_e eXferCode, void *param);
static void am_hal_usb_out_ep_dma1_adma_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum);
static inline uint32_t am_hal_usb_intr_out_status_clear(am_hal_usb_state_t *pState, uint8_t ui8EpNum);

//
// Endpoint address and attribute operation functions
//
//*****************************************************************************
//
// Get Endpoint Direction
//
//*****************************************************************************
static inline am_hal_usb_ep_dir_e
am_hal_usb_ep_dir(uint8_t addr)
{
    return (addr & AM_HAL_USB_EP_DIR_IN_MASK) ? AM_HAL_USB_EP_DIR_IN : AM_HAL_USB_EP_DIR_OUT;
}

//*****************************************************************************
//
// Get Endpoint Number
//
//*****************************************************************************
static inline uint8_t
am_hal_usb_ep_number(uint8_t addr)
{
    return (uint8_t)(addr & (~AM_HAL_USB_EP_DIR_IN_MASK));
}

//*****************************************************************************
//
// Get Endpoint Address Type
//
//*****************************************************************************
static inline uint8_t
am_hal_usb_ep_addr(uint8_t num, uint8_t dir)
{
    return (uint8_t)(num | (dir ?  AM_HAL_USB_EP_DIR_IN_MASK : 0));
}

//*****************************************************************************
//
// Get Endpoint Transfer Type
//
//*****************************************************************************
static inline am_hal_usb_ep_xfer_type_e
am_hal_usb_ep_xfer_type(uint8_t attr)
{
    return (am_hal_usb_ep_xfer_type_e)(attr & AM_HAL_USB_EP_XFER_MASK);
}

//*****************************************************************************
//
// check and update SOF interrupt setting
//
//*****************************************************************************
static inline void
am_hal_usb_update_sof_intr_enable(am_hal_usb_state_t *pState, USB_Type *pUSB)
{
    if ( pState->sofReq.sofIntrReq == 0 )
    {
        INTRUSBE_SOF_Disable(pUSB);
    }
    else
    {
        INTRUSBE_SOF_Enable(pUSB);
    }
}

//*****************************************************************************
//
// Initialization function.
//
//*****************************************************************************
uint32_t
am_hal_usb_initialize(uint32_t ui32Module, void **ppHandle)
{
    //
    // Check that the request module is in range.
    //
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32Module >= AM_REG_USB_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    //
    // Check for valid arguments.
    //
    if ( !ppHandle )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Check if the handle is unallocated.
    //
    if ( g_am_hal_usb_states[ui32Module].prefix.s.bInit )
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

    //
    // Initialize the handle.
    //
    g_am_hal_usb_states[ui32Module].prefix.s.bInit   = true;
    g_am_hal_usb_states[ui32Module].prefix.s.magic   = AM_HAL_MAGIC_USB;
    g_am_hal_usb_states[ui32Module].ui32Module       = ui32Module;
    g_am_hal_usb_states[ui32Module].sRegState.bValid = false;

    //
    // Set PIO mode by default.
    //
    g_am_hal_usb_states[ui32Module].eOutXferMode = AM_HAL_USB_OUT_PIO_MODE;
    g_am_hal_usb_states[ui32Module].eInXferMode = AM_HAL_USB_IN_PIO_MODE;

    //
    // Initialize USB DMA0 Command Queue
    //
    am_hal_queue_init(&g_sUSBDMA0Queue, g_psUSBDMA0QueueMem,
                      sizeof(am_hal_usb_dma0_queue_t), sizeof(g_psUSBDMA0QueueMem));

    //
    // Return the handle.
    //
    *ppHandle = (void *)&g_am_hal_usb_states[ui32Module];

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
} // am_hal_usb_initialize()

//*****************************************************************************
//
// De-Initialization function.
//
//*****************************************************************************
uint32_t
am_hal_usb_deinitialize(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

    //
    // Check the handle.
    //
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( !AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    //
    // Reset the handle.
    //
    pState->prefix.s.bInit = false;
    pState->prefix.s.magic = 0;
    pState->ui32Module     = 0;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Power control functions.
//
//*****************************************************************************
uint32_t
am_hal_usb_power_control(void *pHandle,
                         am_hal_sysctrl_power_state_e ePowerState,
                         bool bRetainState)
{


#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if ( !AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    uint8_t i;
    uint32_t ui32Status;
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *) pHandle;
    uint32_t ui32Module = pState->ui32Module;


#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( ui32Module >= AM_REG_USB_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    USB_Type *pUSB = USBn(ui32Module);

    //
    // Decode the requested power state and update SCARD operation accordingly.
    //
    switch (ePowerState)
    {
        case AM_HAL_SYSCTRL_WAKE:
            //
            // Make sure we don't try to restore an invalid state.
            //
            if ( bRetainState && !pState->sRegState.bValid )
            {
                return AM_HAL_STATUS_INVALID_OPERATION;
            }

            //
            // Enable power control.
            //
            if ((ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_USB)) != AM_HAL_STATUS_SUCCESS)
            {
                return ui32Status;
            }

            if ((ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_USBPHY)) != AM_HAL_STATUS_SUCCESS)
            {
                return ui32Status;
            }

            //
            // Add the USB SRAM trim settings
            //
            pUSB->SRAMCTRL = _VAL2FLD(USB_SRAMCTRL_WABL, 1)     |
                             _VAL2FLD(USB_SRAMCTRL_WABLM, 1)    |
                             _VAL2FLD(USB_SRAMCTRL_RAWL, 1)     |
                             _VAL2FLD(USB_SRAMCTRL_RAWLM, 2)    |
                             _VAL2FLD(USB_SRAMCTRL_EMAW, 0)     |
                             _VAL2FLD(USB_SRAMCTRL_EMAS, 0)     |
                             _VAL2FLD(USB_SRAMCTRL_EMA, 3)      |
                             _VAL2FLD(USB_SRAMCTRL_RET1N, 1);

            if ( bRetainState )
            {
                AM_HAL_USB_ENTER_CRITICAL;

                //
                // Restore the CFG register
                //
                pUSB->CFG0 = pState->sRegState.regCFG0;
                pUSB->CFG1 = pState->sRegState.regCFG1;
                pUSB->CFG2 = pState->sRegState.regCFG2;

                for (i = AM_HAL_USB_EP1_NUMBER; i <= AM_HAL_USB_EP5_NUMBER; i++)
                {
                    EP_INDEX_Set(pUSB, i);
                    pUSB->IDX0 = pState->sRegState.regEndPoints[i-1].regIDX0;
                    pUSB->IDX1 = pState->sRegState.regEndPoints[i-1].regIDX1;
                    pUSB->IDX2 = pState->sRegState.regEndPoints[i-1].regIDX2;
                }

                pState->sRegState.bValid = false;

                AM_HAL_USB_EXIT_CRITICAL;
            }
            break;

        case AM_HAL_SYSCTRL_NORMALSLEEP:
        case AM_HAL_SYSCTRL_DEEPSLEEP:
            if ( bRetainState )
            {
                AM_HAL_USB_ENTER_CRITICAL;

                pState->sRegState.regCFG0 = pUSB->CFG0;
                pState->sRegState.regCFG1 = pUSB->CFG1;
                pState->sRegState.regCFG2 = pUSB->CFG2;

                for (i = AM_HAL_USB_EP1_NUMBER; i <= AM_HAL_USB_EP5_NUMBER; i++)
                {
                    EP_INDEX_Set(pUSB, i);
                    pState->sRegState.regEndPoints[i - 1].regIDX0 = pUSB->IDX0;
                    pState->sRegState.regEndPoints[i - 1].regIDX1 = pUSB->IDX1;
                    pState->sRegState.regEndPoints[i - 1].regIDX2 = pUSB->IDX2;
                }

                pState->sRegState.bValid = true;

                AM_HAL_USB_EXIT_CRITICAL;
            }

            //
            // Clear all interrupts before sleeping as having a pending SCARD
            // interrupt burns power.
            //

            am_hal_usb_intr_usb_clear(pState);
            am_hal_usb_intr_ep_in_clear(pState);
            am_hal_usb_intr_ep_out_clear(pState);

            //
            // Disable power control.
            //
            if ((ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_USB)) == AM_HAL_STATUS_SUCCESS)
            {
                ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_USBPHY);
            }
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    }

    //
    // Return the status.
    //
    return ui32Status;
}

//*****************************************************************************
//
// USB clock control functions
//
//*****************************************************************************
uint32_t
am_hal_usb_phy_clock_enable(void *pHandle, bool bEnable, am_hal_usb_dev_speed_e eSpeed)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check to make sure this is a valid handle.
    //
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    am_hal_usb_state_t *pState = (am_hal_usb_state_t *) pHandle;
    am_hal_syspll_mux_select_e eCurLLMuxSel;
    am_hal_syspll_mux_select_e eTargetLLMuxSel;
    USB_CLKCTRL_PHYREFCLKSEL_Enum eTargetUsbClkGenSel;

    //
    // Derive Final Setting from selection
    //
    if (bEnable)
    {
        if (eSpeed == AM_HAL_USB_SPEED_HIGH)
        {
            eTargetLLMuxSel = AM_HAL_SYSPLL_LLMUX_USB_SRC_PLL;
            eTargetUsbClkGenSel = USB_CLKCTRL_PHYREFCLKSEL_OFF;
        }
        else
        {
            eTargetLLMuxSel = AM_HAL_SYSPLL_LLMUX_USB_SRC_CLKGEN;
            eTargetUsbClkGenSel = USB_CLKCTRL_PHYREFCLKSEL_HFRC_24MHz;
        }
    }
    else
    {
        eTargetLLMuxSel = AM_HAL_SYSPLL_LLMUX_USB_SRC_CLKGEN;
        eTargetUsbClkGenSel = USB_CLKCTRL_PHYREFCLKSEL_OFF;
    }

    //
    // Switch last level clock mux
    //
    am_hal_syspll_mux_get(AM_HAL_SYSPLL_LLMUX_USB, &eCurLLMuxSel);
    if (eCurLLMuxSel != eTargetLLMuxSel)
    {
        //
        // Make sure USB PHYREFCLK from from clk_gen source is available before
        // configuring PLL mux
        //
        if (eTargetUsbClkGenSel == USB_CLKCTRL_PHYREFCLKSEL_OFF)
        {
            if (USBn(pState->ui32Module)->CLKCTRL_b.PHYREFCLKSEL == USB_CLKCTRL_PHYREFCLKSEL_OFF)
            {
                USBn(pState->ui32Module)->CLKCTRL_b.PHYREFCLKSEL = USB_CLKCTRL_PHYREFCLKSEL_HFRC_24MHz;
            }
        }
        else
        {
            USBn(pState->ui32Module)->CLKCTRL_b.PHYREFCLKSEL = eTargetUsbClkGenSel;
        }
        USBn(pState->ui32Module)->CLKCTRL_b.PHYREFCLKDIS = 0;

        //
        // Switch PLL mux select and wait for switching to complete
        //
        am_hal_syspll_mux_select(eTargetLLMuxSel, 1);
    }

    //
    // Switch USB PHYREFCLK clk_gen source
    //
    USBn(pState->ui32Module)->CLKCTRL_b.PHYREFCLKSEL = eTargetUsbClkGenSel;

    //
    // Return success status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// start the remote wakeup
//
//*****************************************************************************
uint32_t
am_hal_usb_start_remote_wakeup(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    // Wrong USB device state
    if (pState->eDevState != AM_HAL_USB_DEV_STATE_SUSPENDED)
    {
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
    else
    {
        INTRUSBE_Resume_Enable(pUSB);
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// stop the remote wakeup
//
//*****************************************************************************
uint32_t
am_hal_usb_end_remote_wakeup(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    //
    // Should clear this bit after 10 ms (a maximum of 15 ms) to end Resume signaling
    //
    INTRUSBE_Resume_Disable(pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// set the USB device address
//
//*****************************************************************************
uint32_t
am_hal_usb_set_addr(void *pHandle, uint8_t ui8DevAddr)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
        if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
        {
            return AM_HAL_STATUS_INVALID_HANDLE;
        }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    FADDR_FuncAddr_Set(pUSB, ui8DevAddr);

    return AM_HAL_STATUS_SUCCESS;
}

//
// Upper layer function like USB power control module
// will use this API to set USB device state
// from 'SUSPENDING' to 'SUSPENDED' or
// from 'RESUMING' to 'RESUMED'
//
//*****************************************************************************
//
// set the USB device state
//
//*****************************************************************************
uint32_t
am_hal_usb_set_dev_state(void *pHandle, am_hal_usb_dev_state_e eDevState)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    AM_CRITICAL_BEGIN
    pState->eDevState = eDevState;
    AM_CRITICAL_END

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// get the current USB speed
//
//*****************************************************************************
am_hal_usb_dev_speed_e
am_hal_get_usb_dev_speed(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_USB_SPEED_UNKNOWN;
    }
#endif

    return pState->eDevSpeed;
}

//*****************************************************************************
//
// set the USB speed
//
//*****************************************************************************
uint32_t
am_hal_usb_set_dev_speed(void *pHandle, am_hal_usb_dev_speed_e eSpeed)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) || eSpeed == AM_HAL_USB_SPEED_UNKNOWN)
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    if (eSpeed == AM_HAL_USB_SPEED_FULL || eSpeed == AM_HAL_USB_SPEED_LOW)
    {
        POWER_HSEnab_Clear(pUSB);
    }
    else
    {
        POWER_HSEnab_Set(pUSB);
    }
    pState->eDevSpeed = eSpeed;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// soft connect to the USB host
//
//*****************************************************************************
uint32_t
am_hal_usb_attach(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    // Do soft connection to the USB host
    POWER_AMSPECIFIC_Set(pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// soft disconnect to the USB host
//
//*****************************************************************************
uint32_t
am_hal_usb_detach(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    // Do soft disconnection from the USB Host
    POWER_AMSPECIFIC_Clear(pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// get the USB frame number
//
//*****************************************************************************
uint32_t
am_hal_get_frame_number(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    return FRAME_NUM(pUSB);
}

#ifdef AM_HAL_USB_GET_HW_INFO_ENABLED

//*****************************************************************************
//
// get the hardware information
//
//*****************************************************************************
uint32_t
am_hal_usb_get_hw_infor(void *pHandle, am_hal_usb_hw_info *sHWInfo)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    USB_Type *pUSB = USBn(pState->ui32Module);

    sHWInfo->ui8Major       = HWVERS_xx(pUSB);
    sHWInfo->ui16Minor      = HWVERS_yyy(pUSB);
    sHWInfo->ui8OutEpNum    = EPINFO_OutEndPoints(pUSB);
    sHWInfo->ui8InEpNum     = EPINFO_InEndPoints(pUSB);
    sHWInfo->ui8RamBits     = RAMINFO_RamBits(pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

#endif

//*****************************************************************************
//
// Config USB DMA mode0
//
//*****************************************************************************
static inline uint32_t
am_hal_usb_dma_mode0_configure(USB_Type *pUSB, uint8_t ui8EpNum, am_hal_usb_xfer_dir_e eDir, uint32_t ui32BufAddr, uint32_t ui32Count)
{
    //
    // Mark USB DMA0 Engine as busy
    //
    g_bUSBDMA0Busy = true;

    //
    // Disable DMA0
    //
    DMACTRL_DMAEN_Disable(pUSB);

    //
    // DMA Mode 0 configuration.
    //
    pUSB->DMATARGADDR           = ui32BufAddr;
    pUSB->DMACTRL_b.DMAEP       = ui8EpNum ;
    pUSB->DMACTRL_b.DMADIR      = eDir;
    pUSB->DMACTRL_b.DMAPRI      = AM_HAL_USB_DMA_PRI_LOW;
    pUSB->DMACTRL_b.TOTCOUNT    = ui32Count;

    //
    // Enable DMA interrupts.
    //
    pUSB->DMAINTEN = USB_DMAINTEN_DMACMP_Msk | USB_DMAINTEN_DMAERR_Msk;

    //
    // Enable DMA0.
    //
    DMACTRL_DMAEN_Enable(pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Config USB ADMA mode
//
//*****************************************************************************
static inline uint32_t
am_hal_usb_adma_config(USB_Type *pUSB, uint32_t ui32DmaReqLine, uint32_t ui32DmaReqSize, uint32_t ui32TotalCount,
                       am_hal_usb_dma_pri_e ePriority, am_hal_usb_xfer_dir_e eDirection, uint8_t ui8EpNum, uint32_t ui32TarAddr)
{
    //
    // Check Total Count and DMA request size
    //
    if ( (ui32TotalCount > USB_ADMATOTCOUNT0_ADMATOTCOUNT0_Msk) || (ui32DmaReqSize > USB_ADMAREQSIZE0_ADMAREQSIZE0_Msk) )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Select ADMA Mode
    //
    pUSB->ADMACTRL |= 0x1;

    //
    // Set ADMA request line direction
    //
    if ( eDirection == AM_HAL_USB_IN_DIR )
    {
        pUSB->ADMADIR |= 0x1 << ui32DmaReqLine;
    }
    else
    {
        pUSB->ADMADIR &= ~(0x1 << ui32DmaReqLine);
    }

    //
    // Set ADMA request line priority
    //
    if ( ePriority == AM_HAL_USB_DMA_PRI_HIGH )
    {
        pUSB->ADMAPRI |= 0x1 << ui32DmaReqLine;
    }
    else
    {
        pUSB->ADMAPRI &= ~(0x1 << ui32DmaReqLine);
    }

    //
    // Enable ADMA request line Interrupts
    //
    pUSB->ADMACMPINTEN |= 0x1 << ui32DmaReqLine;
    pUSB->ADMAERRINTEN |= 0x1 << ui32DmaReqLine;


    switch (ui32DmaReqLine)
    {
        case 0:
        {
            pUSB->ADMATARGADDR0 = ui32TarAddr;
            pUSB->ADMAEP0      |= ui8EpNum & USB_ADMAEP0_ADMAEP0_Msk;
            pUSB->ADMAREQSIZE0 |= ui32DmaReqSize & USB_ADMAREQSIZE0_ADMAREQSIZE0_Msk;
            pUSB->ADMATOTCOUNT0 = ui32TotalCount & USB_ADMATOTCOUNT0_ADMATOTCOUNT0_Msk;
            break;
        }
        case 1 :
        {
            pUSB->ADMATARGADDR1 = ui32TarAddr;
            pUSB->ADMAEP1      |= ui8EpNum & USB_ADMAEP1_ADMAEP1_Msk;
            pUSB->ADMAREQSIZE1 |= ui32DmaReqSize & USB_ADMAREQSIZE1_ADMAREQSIZE1_Msk;
            pUSB->ADMATOTCOUNT1 = ui32TotalCount & USB_ADMATOTCOUNT1_ADMATOTCOUNT1_Msk;
            break;
        }
        case 2 :
        {
            pUSB->ADMATARGADDR2 = ui32TarAddr;
            pUSB->ADMAEP2      |= ui8EpNum & USB_ADMAEP2_ADMAEP2_Msk;
            pUSB->ADMAREQSIZE2 |= ui32DmaReqSize & USB_ADMAREQSIZE2_ADMAREQSIZE2_Msk;
            pUSB->ADMATOTCOUNT2 = ui32TotalCount & USB_ADMATOTCOUNT2_ADMATOTCOUNT2_Msk;
            break;
        }
        case 3 :
        {
            pUSB->ADMATARGADDR3 = ui32TarAddr;
            pUSB->ADMAEP3      |= ui8EpNum & USB_ADMAEP3_ADMAEP3_Msk;
            pUSB->ADMAREQSIZE3 |= ui32DmaReqSize & USB_ADMAREQSIZE3_ADMAREQSIZE3_Msk;
            pUSB->ADMATOTCOUNT3 = ui32TotalCount & USB_ADMATOTCOUNT3_ADMATOTCOUNT3_Msk;
            break;
        }
        case 4 :
        {
            pUSB->ADMATARGADDR4 = ui32TarAddr;
            pUSB->ADMAEP4      |= ui8EpNum & USB_ADMAEP4_ADMAEP4_Msk;
            pUSB->ADMAREQSIZE4 |= ui32DmaReqSize & USB_ADMAREQSIZE4_ADMAREQSIZE4_Msk;
            pUSB->ADMATOTCOUNT4 = ui32TotalCount & USB_ADMATOTCOUNT4_ADMATOTCOUNT4_Msk;
            break;
        }
        case 5:
        {
            pUSB->ADMATARGADDR5 = ui32TarAddr;
            pUSB->ADMAEP5      |= ui8EpNum & USB_ADMAEP5_ADMAEP5_Msk;
            pUSB->ADMAREQSIZE5 |= ui32DmaReqSize & USB_ADMAREQSIZE5_ADMAREQSIZE5_Msk;
            pUSB->ADMATOTCOUNT5 = ui32TotalCount & USB_ADMATOTCOUNT5_ADMATOTCOUNT5_Msk;
            break;
        }
        case 6 :
        {
            pUSB->ADMATARGADDR6 = ui32TarAddr;
            pUSB->ADMAEP6      |= ui8EpNum & USB_ADMAEP6_ADMAEP6_Msk;
            pUSB->ADMAREQSIZE6 |= ui32DmaReqSize & USB_ADMAREQSIZE6_ADMAREQSIZE6_Msk;
            pUSB->ADMATOTCOUNT6 = ui32TotalCount & USB_ADMATOTCOUNT6_ADMATOTCOUNT6_Msk;
            break;
        }
        case 7 :
        {
            pUSB->ADMATARGADDR7 = ui32TarAddr;
            pUSB->ADMAEP7      |= ui8EpNum & USB_ADMAEP7_ADMAEP7_Msk;
            pUSB->ADMAREQSIZE7 |= ui32DmaReqSize & USB_ADMAREQSIZE7_ADMAREQSIZE7_Msk;
            pUSB->ADMATOTCOUNT7 = ui32TotalCount & USB_ADMATOTCOUNT7_ADMATOTCOUNT7_Msk;
            break;
        }
        case 8 :
        {
            pUSB->ADMATARGADDR8 = ui32TarAddr;
            pUSB->ADMAEP8      |= ui8EpNum & USB_ADMAEP8_ADMAEP8_Msk;
            pUSB->ADMAREQSIZE8 |= ui32DmaReqSize & USB_ADMAREQSIZE8_ADMAREQSIZE8_Msk;
            pUSB->ADMATOTCOUNT8 = ui32TotalCount & USB_ADMATOTCOUNT8_ADMATOTCOUNT8_Msk;
            break;
        }
        case 9 :
        {
            pUSB->ADMATARGADDR9 = ui32TarAddr;
            pUSB->ADMAEP9      |= ui8EpNum & USB_ADMAEP9_ADMAEP9_Msk;
            pUSB->ADMAREQSIZE9 |= ui32DmaReqSize & USB_ADMAREQSIZE9_ADMAREQSIZE9_Msk;
            pUSB->ADMATOTCOUNT9 = ui32TotalCount & USB_ADMATOTCOUNT9_ADMATOTCOUNT9_Msk;
            break;
        }
        default:
            return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Enable ADMA for USB DMA request line
    //
    pUSB->ADMAEN |= (0x1 << ui32DmaReqLine);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Unload FIFO to buffer in PIO mode
//
//*****************************************************************************
static inline void
am_hal_usb_fifo_unloading(USB_Type *pUSB, uint8_t ui8EpNum, uint8_t *pucBuf, uint32_t ui32Count)
{
    uint32_t Read32bitCount;
    uint32_t Read32bitRemain;

    Read32bitCount   = ui32Count / sizeof(uint32_t);
    Read32bitRemain  = ui32Count - Read32bitCount * sizeof(uint32_t);

    for (int i = 0; i < Read32bitCount; i++)
    {
        *((uint32_t *)pucBuf + i) = *FIFOx_ADDR(pUSB, ui8EpNum);
    }

    if (Read32bitRemain)
    {
        volatile uint8_t *pui8FIFO = ((volatile uint8_t *)FIFOx_ADDR(pUSB, ui8EpNum));

        for (int i = 0; i < Read32bitRemain; i++)
        {
            pucBuf[Read32bitCount*sizeof(uint32_t) + i] = *pui8FIFO;
        }
    }

}

//*****************************************************************************
//
// Unload FIFO to buffer in DMA Mode 1 (ADMA Mode)
//
//*****************************************************************************
static inline uint32_t
am_hal_usb_fifo_unloading_dma1(USB_Type *pUSB, am_hal_usb_state_t *pState, uint8_t ui8EpNum, uint8_t *pucBuf, uint32_t ui32Count)
{
    uint32_t ui32Status;
    uint32_t ui32ReqLineNum;
    uint32_t ui32DmaReqSize;
    am_hal_usb_ep_xfer_t *pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_OUT];

    if (ui32Count == 0)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    if (ui8EpNum < 1 || ui8EpNum > 5)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // DMA Mode 1 configuration.
    //
    EP_INDEX_Set(pUSB, ui8EpNum);

    //
    // OUT Packet is ready to be processed
    //
    if (OUTCSRL_OutPktRdy(pUSB))
    {
        // Only handle manually if it is short packet. Else, let ADMA handle it.
        uint32_t ui32OutCnt = OUTCOUNT(pUSB);
        if (ui32OutCnt == 0)
        {
            am_hal_usb_intr_out_status_clear(pState, ui8EpNum);
            OUTCSRL_OutPktRdy_Clear(pUSB);
        }
        else if (ui32OutCnt < OUTMAXP_MaxPayload(pUSB))
        {
            uint32_t ui32LenUnload = (ui32OutCnt <= pXfer->len) ? ui32OutCnt : pXfer->len;
            am_hal_usb_fifo_unloading(pUSB, ui8EpNum, pucBuf, ui32LenUnload);
            am_hal_usb_intr_out_status_clear(pState, ui8EpNum);
            OUTCSRL_OutPktRdy_Clear(pUSB);
            am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum, ui32LenUnload, USB_XFER_DONE, NULL);
            return AM_HAL_STATUS_SUCCESS;
        }
    }

    if ( ui32Count > OUTMAXP_MaxPayload(pUSB) )
    {
        OUTCSRU_AutoClear_Set(pUSB);
        if ((pState->sDBufferConfig.out_ep.en >> (ui8EpNum - 1)) & 0x01)
        {
            OUTCSRU_DPktBufDis_Clear(pUSB);
        }
    }
    else
    {
        OUTCSRU_AutoClear_Clear(pUSB);
        if ((pState->sDBufferConfig.out_ep.en >> (ui8EpNum - 1)) & 0x01)
        {
            OUTCSRU_DPktBufDis_Set(pUSB);
        }
    }
    OUTCSRU_DMAReqEnab_Set(pUSB);
    OUTCSRU_DMAReqMode_Set(pUSB);

    ui32ReqLineNum = ui8EpNum + 4;
    ui32DmaReqSize = OUTMAXP_MaxPayload(pUSB);

    ui32Status = am_hal_usb_adma_config(pUSB, ui32ReqLineNum, ui32DmaReqSize, ui32Count, AM_HAL_USB_DMA_PRI_LOW, AM_HAL_USB_OUT_DIR, ui8EpNum, (uint32_t)pucBuf);
    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Status;
    }

    #ifdef ENABLE_ADMA_OUT_XFER_END_DETECTION
    //
    // Start timeout mechanism for ADMA
    //
    pXfer->pLastBufAddr                 = (uint32_t)pucBuf;
    pXfer->pStartBufAddr                = (uint32_t)pucBuf;
    pXfer->timeout_counter              = AM_HAL_USB_TIMEOUT;
    pState->sofReq.sofIntrReq_b.out_ep |= (1 << (ui8EpNum - 1));
    am_hal_usb_update_sof_intr_enable(pState, pUSB);
    #endif

    //
    // Enable DMA.
    //
    DMACTRL_DMAEN_Enable(pUSB);

    //
    // enable out endpoint interrupt for this EP.
    //
    INTROUTE_Enable(pUSB, 0x1 << ui8EpNum);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Unload FIFO to buffer in DMA Mode 0
//
//*****************************************************************************
static inline uint32_t
am_hal_usb_dma_unloading_fifo(USB_Type *pUSB, uint8_t ui8EpNum, uint8_t *pucBuf, uint32_t ui32Count)
{
    uint32_t ui32Status;

    if (ui32Count == 0)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    am_hal_usb_dma0_queue_t dma0_entry =
    {
        .ui32BufAddr    = (uint32_t) pucBuf,
        .ui32Count      = ui32Count,
        .ui8EpNum       = ui8EpNum,
        .eDir           = AM_HAL_USB_OUT_DIR
    };

    AM_HAL_USB_ENTER_CRITICAL;

    if ( g_bUSBDMA0Busy )
    {
        bool bSuccess = am_hal_queue_item_add(&g_sUSBDMA0Queue, &dma0_entry, 1);
        ui32Status = bSuccess? AM_HAL_STATUS_SUCCESS: AM_HAL_STATUS_FAIL;
    }
    else
    {
        ui32Status = am_hal_usb_dma_mode0_configure(pUSB, ui8EpNum, AM_HAL_USB_OUT_DIR, (uint32_t)pucBuf, ui32Count);
    }

    AM_HAL_USB_EXIT_CRITICAL;

    return ui32Status;
}

//*****************************************************************************
//
// Load the FIFO with Data in PIO mode
//
//*****************************************************************************
static inline void
am_hal_usb_fifo_loading(USB_Type *pUSB, uint8_t ui8EpNum, uint8_t *pucBuf, uint32_t ui32Count)
{
    uint32_t Write32bitCount;
    uint32_t Write32bitRemain;

    Write32bitCount   = ui32Count / sizeof(uint32_t);
    Write32bitRemain  = ui32Count - Write32bitCount * sizeof(uint32_t);

    for (int i = 0; i < Write32bitCount; i++)
    {
        *FIFOx_ADDR(pUSB, ui8EpNum) = *((uint32_t *)pucBuf + i);
    }

    if (Write32bitRemain)
    {
        volatile uint8_t *pui8FIFO = ((volatile uint8_t *)FIFOx_ADDR(pUSB, ui8EpNum));

        for (int i = 0; i < Write32bitRemain; i++)
        {
            *pui8FIFO = pucBuf[Write32bitCount*sizeof(uint32_t) + i];
        }
    }
}

//*****************************************************************************
//
// Load the FIFO with Data in DMA Mode 1 (ADMA Mode)
//
//*****************************************************************************
static inline uint32_t
am_hal_usb_fifo_loading_dma1(USB_Type *pUSB, uint8_t ui8EpNum, uint8_t *pucBuf, uint32_t ui32Count)
{
    uint32_t ui32Status;
    uint32_t ui32ReqLineNum;
    uint32_t ui32DmaReqSize;

    if (ui32Count == 0)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    if (ui8EpNum < 1 || ui8EpNum > 5)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // DMA Mode 1 configuration.
    //
    EP_INDEX_Set(pUSB, ui8EpNum);

    INCSRU_AutoSet_Set(pUSB);
    INCSRU_DMAReqEnab_Set(pUSB);
    INCSRU_DMAReqMode_Set(pUSB);

    ui32ReqLineNum = ui8EpNum - 1;
    ui32DmaReqSize = INMAXP_MaxPayload(pUSB);

    ui32Status = am_hal_usb_adma_config(pUSB, ui32ReqLineNum, ui32DmaReqSize, ui32Count, AM_HAL_USB_DMA_PRI_LOW, AM_HAL_USB_IN_DIR , ui8EpNum, (uint32_t)pucBuf);
    if ( ui32Status != AM_HAL_STATUS_SUCCESS )
    {
        return ui32Status;
    }

    //
    // Enable DMA.
    //
    DMACTRL_DMAEN_Enable(pUSB);

    return AM_HAL_STATUS_SUCCESS;
}


//*****************************************************************************
//
// Load the FIFO with Data in DMA Mode 0
//
//*****************************************************************************
static inline uint32_t
am_hal_usb_dma_loading_fifo(USB_Type *pUSB, uint8_t ui8EpNum, uint8_t *pucBuf, uint32_t ui32Count)
{
    uint32_t ui32Status;

    if (ui32Count == 0)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    am_hal_usb_dma0_queue_t dma0_entry =
    {
        .ui32BufAddr    = (uint32_t)pucBuf,
        .ui32Count      = ui32Count,
        .ui8EpNum       = ui8EpNum,
        .eDir           = AM_HAL_USB_IN_DIR
    };

    AM_HAL_USB_ENTER_CRITICAL;

    if ( g_bUSBDMA0Busy )
    {
        bool bSuccess = am_hal_queue_item_add(&g_sUSBDMA0Queue, &dma0_entry, 1);
        ui32Status = bSuccess? AM_HAL_STATUS_SUCCESS: AM_HAL_STATUS_FAIL;
    }
    else
    {
        am_hal_usb_dma_mode0_configure(pUSB, ui8EpNum, AM_HAL_USB_IN_DIR, (uint32_t)pucBuf, ui32Count);
    }

    AM_HAL_USB_EXIT_CRITICAL;

    return ui32Status;
}


#ifdef AM_HAL_USB_TEST_MODE_ENABLED

static const uint8_t
test_packet[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
    0xEE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0xFC, 0x7E, 0xBF, 0xDF,
    0xEF, 0xF7, 0xFB, 0xFD, 0x7E
};

//*****************************************************************************
//
// set the USB test mode flag
//
//*****************************************************************************
uint32_t
am_hal_usb_enter_test_mode(const void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    pState->bInTestMode = true;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// do the USB test
//
//*****************************************************************************
uint32_t
am_hal_usb_test_mode(const void *pHandle, const am_hal_usb_test_mode_e eTestMode)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    // bInTestMode is setting when host send
    // SetFeature(TEST_TMODE)
    if (pState->bInTestMode)
    {
        bool bTestModeEntered = true;
        switch ( eTestMode )
        {
            case AM_HAL_USB_TEST_SE0_NAK:
                TESTMODE_TestSE0NAK_Set(pUSB);
                break;
            case AM_HAL_USB_TEST_J:
                TESTMODE_TestJ_Set(pUSB);
                break;
            case AM_HAL_USB_TEST_K:
                TESTMODE_TestK_Set(pUSB);
                break;
            case AM_HAL_USB_TEST_PACKET:
                EP_INDEX_Set(pUSB, 0x0);
                am_hal_usb_fifo_loading(pUSB, 0x0, (uint8_t *)test_packet, sizeof(test_packet));
                CSR0_InPktRdy_Set(pUSB);
                TESTMODE_TestPacket_Set(pUSB);
                break;
            default:
                bTestModeEntered = false;
                break;
        }
        if (bTestModeEntered)
        {
            while(1);
        }
    }
    return AM_HAL_STATUS_SUCCESS;
}

#endif

//*****************************************************************************
//
// enable the SOF interrupt
//
//*****************************************************************************
uint32_t
am_hal_usb_enable_sof_intr(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);
    pState->sofReq.sofIntrReq_b.stack = true;
    am_hal_usb_update_sof_intr_enable(pState, pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// disable the SOF interrupt
//
//*****************************************************************************
uint32_t
am_hal_usb_disable_sof_intr(void *pHandle)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);
    pState->sofReq.sofIntrReq_b.stack = false;
    am_hal_usb_update_sof_intr_enable(pState, pUSB);

    return AM_HAL_STATUS_SUCCESS;
}

//
// Endpoints related operation functions
//

//*****************************************************************************
//
// Reset USB Transfer
//
//*****************************************************************************
static inline void
am_hal_usb_xfer_reset(am_hal_usb_ep_xfer_t *pXfer)
{
    memset((void *)pXfer, 0x0, sizeof(*pXfer));
}

//*****************************************************************************
//
// Reset EP0 State
//
//*****************************************************************************
static inline void
am_hal_usb_ep0_state_reset(am_hal_usb_state_t *pState)
{
    pState->eEP0State = AM_HAL_USB_EP0_STATE_IDLE;
    am_hal_usb_xfer_reset(&pState->ep0_xfer);
}

//*****************************************************************************
//
// Complete the USB Transfer
//
//*****************************************************************************
static void
am_hal_usb_xfer_complete(am_hal_usb_state_t *pState, am_hal_usb_ep_xfer_t *pXfer,
                         uint8_t ui8EpAddr, uint32_t ui32XferLen,
                         am_hal_usb_xfer_code_e eXferCode, void *param)
{
    /* The transaction we're about to issue the callback for is still marked 'busy'.
     * This isn't normally a problem in thread models, where the ISR callback just
     * sets a bit for later. Pure-ISR USB implementations will generally want to
     * immediately reissue endpoint transactions./
     *
     * So, reset the transfer first, then issue the callback.
     */

    if (pState && pXfer)
    {
        #ifdef ENABLE_ADMA_OUT_XFER_END_DETECTION
        //
        // If ADMA timeout mechanism is activated. Deactivate it accordingly
        //
        if (pXfer->pStartBufAddr != 0)
        {
            pState->sofReq.sofIntrReq_b.out_ep &= ~(1 << (am_hal_usb_ep_number(ui8EpAddr)-1));
            am_hal_usb_update_sof_intr_enable(pState, USBn(pState->ui32Module));
        }
        #endif

        //
        // Reset Xfer struct and issue xfer complete callback
        //
        am_hal_usb_xfer_reset(pXfer);
        pState->ep_xfer_complete_callback(ui8EpAddr, ui32XferLen, eXferCode, param);
    }
}

//*****************************************************************************
//
// stall the endpoint
//
//*****************************************************************************
uint32_t
am_hal_usb_ep_stall(void *pHandle, uint8_t ui8EpAddr)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP_NUM(ui8EpAddr))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    uint8_t ui8EpNum, ui8EpDir;
    am_hal_usb_state_t *pState;
    USB_Type *pUSB;

    ui8EpNum = am_hal_usb_ep_number(ui8EpAddr);
    ui8EpDir = am_hal_usb_ep_dir(ui8EpAddr);

    pState = (am_hal_usb_state_t *)pHandle;
    pUSB = USBn(pState->ui32Module);

    AM_HAL_USB_ENTER_CRITICAL;
    // Select the EP registers by INDEX
    EP_INDEX_Set(pUSB, ui8EpNum);

    if (ui8EpNum == AM_HAL_USB_EP0_NUMBER)
    {
        // EP0 stall setting
        // It happens when upper layer USB stack can't proccess some
        // Request, now it should be 'AM_HAL_USB_EP0_STATE_SETUP'
        // An EP0 interrupt will generate, SentStall bit should be clear in
        // ep0 interrupt handler
        CSR0_ServicedOutPktRdyAndSendStall_Set(pUSB);
    }
    else
    {
        // Non zero endpoint stall setting
        // In next EP interrupt handling function to clear 'SentStall' bit
        // keep this 'SendStall' bit set until upper layer stack need to
        // clear it.
        switch ( ui8EpDir )
        {
            case AM_HAL_USB_EP_DIR_IN:
                INCSRL_SendStall_Set(pUSB);
                break;
            case AM_HAL_USB_EP_DIR_OUT:
                OUTCSRL_SendStall_Set(pUSB);
                break;
        }
    }

    AM_HAL_USB_EXIT_CRITICAL;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// clear the endpoint stall
//
//*****************************************************************************
uint32_t
am_hal_usb_ep_clear_stall(void *pHandle, uint8_t ui8EpAddr)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle) )
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP_NUM(ui8EpAddr))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    uint8_t ui8EpNum, ui8EpDir;
    am_hal_usb_state_t *pState;
    USB_Type *pUSB;

    ui8EpNum = am_hal_usb_ep_number(ui8EpAddr);
    ui8EpDir = am_hal_usb_ep_dir(ui8EpAddr);

    pState   = (am_hal_usb_state_t *)pHandle;
    pUSB     = USBn(pState->ui32Module);

    // EP0 stall clearing, do nothing
    if (ui8EpNum == AM_HAL_USB_EP0_NUMBER)
    {
        return AM_HAL_STATUS_SUCCESS;
    }

    AM_HAL_USB_ENTER_CRITICAL;
    EP_INDEX_Set(pUSB, ui8EpNum);
    if (ui8EpDir == AM_HAL_USB_EP_DIR_IN)
    {
        INCSRL_SendStall_Clear(pUSB);

        // See 8.1.3 IN endpoint error handling
        INCSRL_ClrDataTog_Set(pUSB);
    }
    else
    {
        OUTCSRL_SendStall_Clear(pUSB);
        // See 8.2.3 Out endpoint error handling
        OUTCSRL_ClrDataTog_Set(pUSB);
    }
    AM_HAL_USB_EXIT_CRITICAL;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Return Size Mapping from Index
//
//*****************************************************************************
static uint32_t
am_hal_usb_fifo_size(uint8_t ui8FifoSZ)
{
    static uint32_t ui32SizeMapping[] =
    {
        8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
    };
    return ui32SizeMapping[ui8FifoSZ];
}

//*****************************************************************************
//
// Return FIFO Size by Packet Size
//
//*****************************************************************************
static uint8_t
am_hal_usb_fifo_size_by_maxpacket(uint16_t ui16PktSize)
{
    uint8_t exp2 = 0;
    uint16_t tmp = ui16PktSize / 8;

    tmp = tmp >> 1;
    while (tmp)
    {
        exp2++;
        tmp = tmp >> 1;
    }
    return exp2;
}

//*****************************************************************************
//
// Return FICO Endpoint Sddress
//
//*****************************************************************************
static uint32_t
am_hal_usb_ep_fifo_addr(uint32_t *ui32Allocated, uint16_t ui16PktSize, bool bEnableDoublePacket)
{
    // first 64 bytes is allocated to EP0
    uint32_t tmp = *ui32Allocated;
    if ( bEnableDoublePacket )
    {
        *ui32Allocated += 2 * ui16PktSize / 8;
    }
    else
    {
        *ui32Allocated += ui16PktSize / 8;
    }

    return tmp;
}

//*****************************************************************************
//
// Reset FIFO Endpoint
//
//*****************************************************************************
static inline void
am_hal_usb_ep_fifo_reset(uint32_t *ui32Allocated)
{
    *ui32Allocated = 8;
}

//*****************************************************************************
//
// initialize the endpoint
//
//*****************************************************************************
uint32_t
am_hal_usb_ep_init(void *pHandle, uint8_t ui8EpAddr, uint8_t ui8EpAttr, uint16_t ui16MaxPacket)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP_NUM(ui8EpAddr))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
    uint8_t sz;
    uint8_t ui8EpNum, ui8EpDir;
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    // Normally USB stack endpoint descriptor should define
    // 64  bytes max packet for full speed
    // 512 bytes max packet for high speed
    bool hspeed = POWER_HSMode(pUSB);

    if ((ui16MaxPacket > am_hal_usb_fifo_size(EP_FIFO_SZ_64)) && !hspeed)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif

    ui8EpNum = am_hal_usb_ep_number(ui8EpAddr);
    ui8EpDir = am_hal_usb_ep_dir(ui8EpAddr);

    EP_INDEX_Set(pUSB, ui8EpNum);

    if (ui8EpNum == 0x0)
    {
        INTRINE_Enable(pUSB, 0x1 << 0x0);
        pState->ep0_maxpacket = ui16MaxPacket;
        return AM_HAL_STATUS_SUCCESS;
    }

    switch ( ui8EpDir )
    {
        case AM_HAL_USB_EP_DIR_IN:
            INMAXP_MaxPayload_Set(pUSB, ui16MaxPacket);
            pState->epin_maxpackets[ui8EpNum - 1] = ui16MaxPacket;

            if (am_hal_usb_ep_xfer_type(ui8EpAttr) == AM_HAL_USB_EP_XFER_ISOCHRONOUS)
            {
                INCSRU_ISO_Set(pUSB);
            }
            else
            {
                INCSRU_ISO_Clear(pUSB);
            }

            INCSRL_ClrDataTog_Set(pUSB);

            if (INCSRL_FIFONotEmpty(pUSB))
            {
                INCSRL_FlushFIFO_Set(pUSB);
            }

#ifdef AM_HAL_USB_FEATURE_SHARING_FIFO_WITH_OUT_EP
            // Only for FIFO is used for both IN and OUT transaction
            // IN EP share the same FIFO with OUT EP
            // Not recommend to use this feature.
            INCSRU_Mode_Set(pUSB);
#endif
            // EP IN FIFO setting
            sz = am_hal_usb_fifo_size_by_maxpacket(ui16MaxPacket);
            if ( (pState->sDBufferConfig.in_ep.en >> (ui8EpNum - 1)) & 0x01 )
            {
                INCSRU_DPktBufDis_Clear(pUSB);
                InFIFOsz_Set(pUSB, FIFO_DOUBLE_PKTBUF, sz);
                InFIFOadd_Set(pUSB, am_hal_usb_ep_fifo_addr(&pState->ui32Allocated, ui16MaxPacket, true));
            }
            else
            {
                INCSRU_DPktBufDis_Set(pUSB);
                InFIFOsz_Set(pUSB, FIFO_SINGLE_PKTBUF, sz);
                InFIFOadd_Set(pUSB, am_hal_usb_ep_fifo_addr(&pState->ui32Allocated, ui16MaxPacket, false));
            }
            break;
        case AM_HAL_USB_EP_DIR_OUT:
            OUTMAXP_MaxPayload_Set(pUSB, ui16MaxPacket);
            pState->epout_maxpackets[ui8EpNum - 1] = ui16MaxPacket;

            if (am_hal_usb_ep_xfer_type(ui8EpAttr) == AM_HAL_USB_EP_XFER_ISOCHRONOUS)
            {
                OUTCSRU_ISO_Set(pUSB);
            }
            else
            {
                // Enable bulk protocol
                OUTCSRU_ISO_Clear(pUSB);

#ifdef AM_HAL_USB_FEATURE_NO_NYET
                // Disable NYET for test purpose (only effective for highspeed)
                OUTCSRU_DisNye_Set(pUSB);
#endif
            }

            OUTCSRL_ClrDataTog_Set(pUSB);

            if (OUTCSRL_OutPktRdy(pUSB))
            {
                OUTCSRL_FlushFIFO_Set(pUSB);
            }

            // EP OUT FIFO setting
            sz = am_hal_usb_fifo_size_by_maxpacket(ui16MaxPacket);
            if ((pState->sDBufferConfig.out_ep.en >> (ui8EpNum - 1)) & 0x01)
            {
                OUTCSRU_DPktBufDis_Clear(pUSB);
                OutFIFOsz_Set(pUSB, FIFO_DOUBLE_PKTBUF, sz);
                OutFIFOadd_Set(pUSB, am_hal_usb_ep_fifo_addr(&pState->ui32Allocated, ui16MaxPacket, true));
            }
            else
            {
                OUTCSRU_DPktBufDis_Set(pUSB);
                OutFIFOsz_Set(pUSB, FIFO_SINGLE_PKTBUF, sz);
                OutFIFOadd_Set(pUSB, am_hal_usb_ep_fifo_addr(&pState->ui32Allocated, ui16MaxPacket, false));
            }
            break;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// submit a USB Endpoint 0 transfer
//
//*****************************************************************************
static uint32_t
am_hal_usb_ep0_xfer(am_hal_usb_state_t *pState, uint8_t ui8EpNum, uint8_t ui8EpDir, uint8_t *pui8Buf, uint32_t ui32Len)
{
    uint32_t status;
    uint16_t maxpacket;
    USB_Type *pUSB = USBn(pState->ui32Module);

    AM_HAL_USB_ENTER_CRITICAL;

    // Select the endpoint by index register
    EP_INDEX_Set(pUSB, ui8EpNum);

    if (pState->ep0_xfer.flags.busy == 0x1)
    {
        status = AM_HAL_STATUS_IN_USE;
    }
    else
    {
        status = AM_HAL_STATUS_SUCCESS;
        pState->ep0_xfer.flags.busy = 0x1;
    }

    if (status == AM_HAL_STATUS_IN_USE)
    {
        AM_HAL_USB_EXIT_CRITICAL;
        return status;
    }

    maxpacket = pState->ep0_maxpacket;

    pState->ep0_xfer.flags.dir = ui8EpDir;
    pState->ep0_xfer.buf = pui8Buf;
    pState->ep0_xfer.len = ui32Len;

    switch ( pState->eEP0State )
    {
        case AM_HAL_USB_EP0_STATE_IDLE:
            if ( (pState->bPendingInEndData || pState->bPendingOutEndData) && (ui32Len == 0) )
            {
                // Reset EP0 state and wait for the next command from host.
                am_hal_usb_ep0_state_reset(pState);
                pState->bPendingInEndData = false;
                pState->bPendingOutEndData = false;
            }
            else
            {
                AM_HAL_USB_EXIT_CRITICAL;
                return AM_HAL_STATUS_FAIL;
            }
            break;

        case AM_HAL_USB_EP0_STATE_SETUP:
            if (ui32Len == 0x0)
            {
                // There are 2 conditions that we are entering to this handling:
                // 1. Previous command was with data stage. However, the subsequent SETUP is
                //    received in ISR before the ACK stage handling is done. For this case,
                //    reset ep0_xfer and keep the state at SETUP.
                // 2. Previous command was command without data stage. We should send the EP0
                //    back to IDLE so that it is able to receive next SETUP correctly.

                // Case 1:
                if ( pState->bPendingOutEndData || pState->bPendingInEndData )
                {
                    am_hal_usb_xfer_reset(&pState->ep0_xfer);
                    pState->bPendingOutEndData = false;
                    pState->bPendingInEndData  = false;
                }
                // case 2:
                else
                {
                    // Upper layer USB stack just use zero length packet to confirm no data stage
                    // some requests like CLEAR_FEARURE, SET_ADDRESS, SET_CONFIGRATION, etc.
                    // end the control transfer from device side
                    CSR0_ServicedOutPktRdyAndDataEnd_Set(pUSB);

                    pState->eEP0State =
                        (ui8EpDir == AM_HAL_USB_EP_DIR_IN) ? AM_HAL_USB_EP0_STATE_STATUS_TX : AM_HAL_USB_EP0_STATE_STATUS_RX;
                }
            }
            else
            {
                // Enter the data stage if the request have the data stage
                // some requests like GET_*_DESCRIPTOR
                CSR0_ServicedOutPktRdy_Set(pUSB);

                // Clear SetupEnd flag since we are starting a new SETUP stage
                // here
                if (CSR0_SetupEnd(pUSB))
                {
                    CSR0_ServicedSetupEnd_Set(pUSB);
                }

                switch ( ui8EpDir )
                {
                    // Read requests handling
                    case AM_HAL_USB_EP_DIR_IN:
                        // Flag that we need to handle End Data later for OUT
                        // direction
                        pState->bPendingOutEndData = true;

                        // Load the first packet
                        if (ui32Len < maxpacket)
                        {
                            pState->ep0_xfer.remaining = 0x0;
                            pState->eEP0State = AM_HAL_USB_EP0_STATE_STATUS_TX;
                            am_hal_usb_fifo_loading(pUSB, 0x0, pui8Buf, ui32Len);

                            CSR0_InPktRdyAndDataEnd_Set(pUSB);
                        }
                        else
                        {
                            pState->ep0_xfer.remaining = ui32Len - maxpacket;
                            pState->eEP0State = AM_HAL_USB_EP0_STATE_DATA_TX;
                            am_hal_usb_fifo_loading(pUSB, 0x0, pui8Buf, maxpacket);

                            // The remaining packets will be loaded in the ep0 interrupt handler function
                            CSR0_InPktRdy_Set(pUSB);

                        }
                        break;
                    case AM_HAL_USB_EP_DIR_OUT:
                        // Write requests handling
                        // Waiting the host sending the data to the device

                        // Flag that we need to handle End Data later for IN
                        // direction
                        pState->bPendingInEndData = true;

                        pState->ep0_xfer.remaining = ui32Len;
                        pState->eEP0State = AM_HAL_USB_EP0_STATE_DATA_RX;
                        break;
                }
            }
            break;
        default:
            AM_HAL_USB_EXIT_CRITICAL;
            return AM_HAL_STATUS_FAIL;
    }

    AM_HAL_USB_EXIT_CRITICAL;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// submit a USB Non-Endpoint 0 transfer
//
//*****************************************************************************

static uint32_t
am_hal_usb_non_ep0_xfer(am_hal_usb_state_t *pState, uint8_t ui8EpNum, uint8_t ui8EpDir, uint8_t *pui8Buf, uint32_t ui32Len)
{
    uint32_t status;
    uint16_t maxpacket;

    // Non EP0 transfer handling
    am_hal_usb_ep_xfer_t *pXfer = &pState->ep_xfers[ui8EpNum - 1][ui8EpDir];
    USB_Type *pUSB = USBn(pState->ui32Module);

    AM_HAL_USB_ENTER_CRITICAL;

    // Select the endpoint by index register
    EP_INDEX_Set(pUSB, ui8EpNum);

    // Need to protect it by disabling interrupt
    // USB EP interrupt hander will update it.
    if (pXfer->flags.busy == 0x1)
    {
        status = AM_HAL_STATUS_IN_USE;
    }
    else
    {
        status = AM_HAL_STATUS_SUCCESS;
        pXfer->flags.busy = 0x1;
    }

    if (status == AM_HAL_STATUS_IN_USE)
    {
        AM_HAL_USB_EXIT_CRITICAL;
        return status;
    }

    pXfer->flags.dir = ui8EpDir;

    // Note: does not use automatic Bulk packet splitting option
    switch (ui8EpDir)
    {
        case  AM_HAL_USB_EP_DIR_IN:
            if (pState->eInXferMode == AM_HAL_USB_IN_DMA_MODE_1)
            {
                // Handling IN endpoint transfer
                maxpacket  = pState->epin_maxpackets[ui8EpNum - 1];
                am_hal_usb_fifo_loading_dma1(pUSB, ui8EpNum, pui8Buf, ui32Len);
                if (ui32Len % maxpacket == 0)
                {
                    pXfer->flags.zlp = 1;
                }

                pXfer->buf = pui8Buf;
                pXfer->len = ui32Len;

                if (ui32Len == 0)
                {
                    pXfer->remaining = 0x0;
                    INCSRL_InPktRdy_Set(pUSB);
                }
                INTRINE_Enable(pUSB, 0x1 << ui8EpNum);
            }
            else
            {
                // Handling IN endpoint transfer
                maxpacket  = pState->epin_maxpackets[ui8EpNum - 1];

                if (ui32Len < maxpacket)
                {
                    if (pState->eInXferMode == AM_HAL_USB_IN_DMA_MODE_0)
                    {
                        if (ui32Len == 0)
                        {
                            *((volatile uint32_t*)&ui32Len) = 0;
                            INCSRL_InPktRdy_Set(pUSB);
                            INTRINE_Enable(pUSB, 0x1 << ui8EpNum);
                        }
                        else
                        {
                            am_hal_usb_dma_loading_fifo(pUSB, ui8EpNum, pui8Buf, ui32Len);
                        }
                    }
                    else
                    {
                        am_hal_usb_fifo_loading(pUSB, ui8EpNum, pui8Buf, ui32Len);
                        INCSRL_InPktRdy_Set(pUSB);
                        INTRINE_Enable(pUSB, 0x1 << ui8EpNum);
                    }
                    pXfer->remaining = 0x0;
                }
                else
                {
                    if (pState->eInXferMode == AM_HAL_USB_IN_DMA_MODE_0)
                    {
                        am_hal_usb_dma_loading_fifo(pUSB, ui8EpNum, pui8Buf, maxpacket);
                    }
                    else
                    {
                        am_hal_usb_fifo_loading(pUSB, ui8EpNum, pui8Buf, maxpacket);
                        INCSRL_InPktRdy_Set(pUSB);
                        INTRINE_Enable(pUSB, 0x1 << ui8EpNum);
                    }
                    pXfer->remaining = ui32Len - maxpacket;
                }

                if (ui32Len == maxpacket)
                {
                    pXfer->flags.zlp = 1;
                }

                pXfer->buf = pui8Buf;
                pXfer->len = ui32Len;
            }
            break;
        case AM_HAL_USB_EP_DIR_OUT:

            // Handling OUT endpoint transfer
            maxpacket  = pState->epout_maxpackets[ui8EpNum - 1];
            pXfer->buf = pui8Buf;
            pXfer->len = ui32Len;
            pXfer->remaining = ui32Len;

            if (pState->eOutXferMode == AM_HAL_USB_OUT_DMA_MODE_1)
            {
                am_hal_usb_fifo_unloading_dma1(pUSB, pState, ui8EpNum, pXfer->buf, pXfer->len);
            }
            else
            {
                //
                // clear OutPktRdy bit before enabling the interrupt
                //
                OUTCSRL_OutPktRdy_Clear(pUSB);

                //
                // enable out endpoint interrupt for this EP.
                //
                INTROUTE_Enable(pUSB, 0x1 << ui8EpNum);
            }
            break;
    }

    AM_HAL_USB_EXIT_CRITICAL;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// submit a USB transfer
//
//*****************************************************************************
uint32_t
am_hal_usb_ep_xfer(void *pHandle, uint8_t ui8EpAddr, uint8_t *pui8Buf, uint32_t ui32Len)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP_NUM(ui8EpAddr))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif

    uint8_t ui8EpNum = am_hal_usb_ep_number(ui8EpAddr);
    uint8_t ui8EpDir = am_hal_usb_ep_dir(ui8EpAddr);

    // Handling EP0 setup control transfer
    if (ui8EpNum == AM_HAL_USB_EP0_NUMBER)
    {
        return am_hal_usb_ep0_xfer(pState, ui8EpNum, ui8EpDir, pui8Buf, ui32Len);
    }
    else
    {
        return am_hal_usb_non_ep0_xfer(pState, ui8EpNum, ui8EpDir, pui8Buf, ui32Len);
    }
}

//*****************************************************************
//
// USB Interrupt handling functions
//
//*****************************************************************

//*****************************************************************
//
// Discard OUT endpoint interrupt event
//
//*****************************************************************
static inline uint32_t
am_hal_usb_intr_out_status_clear(am_hal_usb_state_t *pState, uint8_t ui8EpNum)
{
    USB_Type *pUSB = USBn(pState->ui32Module);

    if (ui8EpNum > 5)
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    pState->ui32PendingIntrOut |= INTROUT_Get(pUSB);
    pState->ui32PendingIntrOut &= ~(0x01 << ui8EpNum);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// get all USB related interrupt status
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_status_get(void *pHandle, uint32_t *ui32IntrUsbStatus, uint32_t *ui32IntrInStatus, uint32_t *ui32IntrOutStatus,
                           uint32_t *ui32IntrDMAStatus, uint32_t *ui32IntrAutoDMAStatus)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    USB_Type *pUSB = USBn(pState->ui32Module);
    *ui32IntrUsbStatus = INTRUSB_Get(pUSB);
    *ui32IntrInStatus  = INTRIN_Get(pUSB);
    *ui32IntrOutStatus = INTROUT_Get(pUSB) | pState->ui32PendingIntrOut;
    *ui32IntrDMAStatus = DMAINTSTAT_Get(pUSB);
    *ui32IntrAutoDMAStatus = ADMACMPINTSTAT_Get(pUSB);

    pState->ui32PendingIntrOut = 0;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// enable the IN endpoints' interrupt
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_in_enable(void *pHandle, uint32_t ui32IntMask)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP(ui32IntMask))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    USB_Type *pUSB = USBn(pState->ui32Module);
    INTRINE_Enable(pUSB, ui32IntMask);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// disable the IN endpoints' interrupt
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_in_disable(void *pHandle, uint32_t ui32IntMask)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP(ui32IntMask))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    INTRINE_Disable(pUSB, ui32IntMask);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// clear the IN endpoints' interrupt status
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_in_clear(void *pHandle)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    volatile uint32_t tmp = 0;
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    tmp = INTRIN_Clear(pUSB);
    (void)tmp;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! get the IN endpoints' interrupt status
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_in_status_get(void *pHandle, uint32_t *pui32IntStatus, bool bEnabledOnly)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
   if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
       return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    uint32_t ui32IntStatus = 0;
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    ui32IntStatus = INTRIN_Get(pUSB);

    if (bEnabledOnly)
    {
        ui32IntStatus &= INTRINE_Get(pUSB);
    }

    *pui32IntStatus = ui32IntStatus;

   return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// enable the OUT endpoints' interrupt
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_out_enable(void *pHandle, uint32_t ui32IntMask)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP(ui32IntMask))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    INTROUTE_Enable(pUSB, ui32IntMask);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// disable the endpoints' interrupt
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_out_disable(void *pHandle, uint32_t ui32IntMask)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_EP(ui32IntMask))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    INTROUTE_Disable(pUSB, ui32IntMask);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// clear the OUT endpoints' interrupt status
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_out_clear(void *pHandle)
{
    volatile uint32_t tmp = 0;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    tmp = INTROUT_Clear(pUSB);
    (void)tmp;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// get the OUT endpoints' interrupt status
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_ep_out_status_get(void *pHandle, uint32_t *pui32IntStatus, bool bEnabledOnly)
{
    uint32_t ui32IntStatus = 0;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    ui32IntStatus = INTROUT_Get(pUSB);

    if (bEnabledOnly)
    {
        ui32IntStatus &= INTROUTE_Get(pUSB);
    }

    *pui32IntStatus = ui32IntStatus;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// enable the USB bus's interrupts
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_usb_enable(void *pHandle, uint32_t ui32IntMask)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_USB(ui32IntMask))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    INTRUSBE_Enable(pUSB, ui32IntMask);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// disable the USB bus's interrupts
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_usb_disable(void *pHandle, uint32_t ui32IntMask)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    if (AM_HAL_USB_CHK_USB(ui32IntMask))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    INTRUSBE_Disable(pUSB, ui32IntMask);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// clear the USB bus interrupts
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_usb_clear(void *pHandle)
{
    volatile uint32_t tmp = 0;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    USB_Type *pUSB = USBn(pState->ui32Module);

    tmp = INTRUSB_Clear(pUSB);
    (void)tmp;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// get the USB bus interrupt status
//
//*****************************************************************************
uint32_t
am_hal_usb_intr_usb_status_get(void *pHandle, uint32_t *pui32IntStatus, bool bEnabledOnly)
{
    uint32_t ui32IntStatus = 0;
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    ui32IntStatus = INTRUSB_Get(pUSB);

    if (bEnabledOnly)
    {
        ui32IntStatus &= INTRUSBE_Get(pUSB);
    }

    *pui32IntStatus = ui32IntStatus;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// register a USB bus event callback function
//
//*****************************************************************************
uint32_t
am_hal_usb_register_dev_evt_callback(void *pHandle, const am_hal_usb_dev_evt_callback cb)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    if (cb != NULL)
    {
        pState->dev_evt_callback = cb;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// register a setup requst callback function
//
//*****************************************************************************
uint32_t
am_hal_usb_register_ep0_setup_received_callback(void *pHandle, const am_hal_usb_ep0_setup_received_callback cb)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    if (cb != NULL)
    {
        pState->ep0_setup_callback = cb;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// register a transfer completion callback function
//
//*****************************************************************************
uint32_t
am_hal_usb_register_ep_xfer_complete_callback(void *pHandle, const am_hal_usb_ep_xfer_complete_callback cb)
{
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    if (cb != NULL)
    {
        pState->ep_xfer_complete_callback = cb;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    return AM_HAL_STATUS_SUCCESS;
}


//*****************************************************************************
//
// Send setup request to upper layer
//
//*****************************************************************************
static void
am_hal_usb_ep0_handle_setup_req(am_hal_usb_state_t *pState, USB_Type *pUSB)
{
    uint8_t setup_req[8];

    if (CSR0_OutPktRdy(pUSB))
    {
        uint16_t count0 = COUNT0(pUSB);
        am_hal_usb_fifo_unloading(pUSB, AM_HAL_USB_EP0_NUMBER, setup_req, count0);
        pState->eEP0State = AM_HAL_USB_EP0_STATE_SETUP;

        #ifdef AM_HAL_USB_TEST_MODE_ENABLED
        if ((setup_req[1] == USB_REQ_SET_FEATURE) && (*((uint16_t*)&setup_req[2]) == USB_REQ_FEATURE_TEST_MODE))
        {
            // Select endpoint 0
            EP_INDEX_Set(pUSB, 0);

            // Give enough time for Test Command to be acked, and Status Stage to complete
            CSR0_ServicedOutPktRdyAndDataEnd_Set(pUSB);
            CSR0_InPktRdyAndDataEnd_Set(pUSB);

            // Delay 2ms to make sure that the ACK and STATUS stage has enough
            // time to take place
            // Note: According to USB2.0 Spec Section 7.1.20, transition to
            //       test mode must be complete no later than 3ms after
            //       completion of the status stage of the request. Hence, this
            //       delay cannot be too long.
            am_hal_delay_us(2000);

            // Enter test mode
            am_hal_usb_enter_test_mode(pState);

            // Execute test modes
            switch(setup_req[5])
            {
                case USB_REQ_TEST_MODE_TESTPKT:
                    am_hal_usb_test_mode(pState, AM_HAL_USB_TEST_PACKET);
                    break;

                case USB_REQ_TEST_MODE_J:
                    am_hal_usb_test_mode(pState, AM_HAL_USB_TEST_J);
                    break;

                case USB_REQ_TEST_MODE_K:
                    am_hal_usb_test_mode(pState, AM_HAL_USB_TEST_K);
                    break;

                case USB_REQ_TEST_MODE_SE0NAK:
                    am_hal_usb_test_mode(pState, AM_HAL_USB_TEST_SE0_NAK);
                    break;
            }
        }
        #endif

        //
        // Let the upper layer USB device stack to handle this request
        //
        pState->ep0_setup_callback(setup_req);
    }
}

//*****************************************************************************
//
// Endpoint 0 Handling
//
//*****************************************************************************
static void
am_hal_usb_ep0_handling(am_hal_usb_state_t *pState, USB_Type *pUSB)
{
    uint8_t *buf;
    uint16_t index, remaining, maxpacket, count0;

    // Select the EP0
    EP_INDEX_Set(pUSB, AM_HAL_USB_EP0_NUMBER);

    maxpacket = pState->ep0_maxpacket;

    switch ( pState->eEP0State )
    {
        case AM_HAL_USB_EP0_STATE_IDLE:
            // process the setup request
            am_hal_usb_ep0_handle_setup_req(pState, pUSB);
            break;

        case AM_HAL_USB_EP0_STATE_SETUP:
            // This case is for unsupported setup requests
            if (CSR0_SentStall(pUSB))
            {
                CSR0_SentStall_Clear(pUSB);
                // Return to the IDLE state, may clear the SendStall bit when
                // USB host clear the stall
                am_hal_usb_ep0_state_reset(pState);
                am_hal_usb_xfer_reset(&pState->ep0_xfer);
            }
            break;

        case AM_HAL_USB_EP0_STATE_DATA_RX:
            remaining = pState->ep0_xfer.remaining;
            buf       = pState->ep0_xfer.buf;
            index     = pState->ep0_xfer.len - remaining;

            // 7.6 error handling
            // how to handle Setup End error
            if (CSR0_SetupEnd(pUSB))
            {
                CSR0_ServicedSetupEnd_Set(pUSB);
                if (!CSR0_OutPktRdy(pUSB))
                {
                    am_hal_usb_xfer_complete(pState, &pState->ep0_xfer, 0x0, pState->ep0_xfer.len - pState->ep0_xfer.remaining, USB_XFER_ERROR, NULL);
                    am_hal_usb_ep0_state_reset(pState);
                }
                return;
            }

            // 7.6 error handling
            // 3. The host sends more than MaxP data bytes in an OUT data packet
            if (CSR0_SentStall(pUSB))
            {
                // Clear the SentStall bit
                CSR0_SentStall_Clear(pUSB);
                return;
            }

            // Write requests handling, in fact no such request is used
            // in TinyUSB, like TUSB_REQ_SET_DESCRIPTOR
            if (CSR0_OutPktRdy(pUSB))
            {
                count0 = COUNT0(pUSB);
                pState->ep0_xfer.remaining -= count0;
                // Unload the FIFO data
                am_hal_usb_fifo_unloading(pUSB, 0x0, pState->ep0_xfer.buf + index, count0);

                if (count0 < maxpacket)
                {
                    pState->eEP0State = AM_HAL_USB_EP0_STATE_STATUS_RX;
                    CSR0_ServicedOutPktRdyAndDataEnd_Set(pUSB);
                }
                else
                {
                    CSR0_ServicedOutPktRdy_Set(pUSB);
                }
            }
            break;
       case AM_HAL_USB_EP0_STATE_DATA_TX:
            // Read requests handling
            remaining = pState->ep0_xfer.remaining;
            buf       = pState->ep0_xfer.buf;
            index     = pState->ep0_xfer.len - remaining;

            if (CSR0_SetupEnd(pUSB))
            {
                CSR0_ServicedSetupEnd_Set(pUSB);
                am_hal_usb_xfer_complete(pState, &pState->ep0_xfer, 0x0 | AM_HAL_USB_EP_DIR_IN_MASK, pState->ep0_xfer.len - pState->ep0_xfer.remaining, USB_XFER_ERROR, NULL);
                am_hal_usb_ep0_state_reset(pState);
                return;
            }

            // see page 62
            if (CSR0_SentStall(pUSB))
            {
                // Clear the SentStall bit
                CSR0_SentStall_Clear(pUSB);
                return;
            }

            if (CSR0_InPktRdy(pUSB) == 0x0) //In data packet FIFO is empty
            {
                if (remaining <= maxpacket)
                {
                    am_hal_usb_fifo_loading(pUSB, 0x0, buf + index, remaining);
                    pState->ep0_xfer.remaining = 0;
                    pState->eEP0State = AM_HAL_USB_EP0_STATE_STATUS_TX;

                    CSR0_InPktRdyAndDataEnd_Set(pUSB);
                }
                else
                {
                    pState->ep0_xfer.remaining -= maxpacket;
                    am_hal_usb_fifo_loading(pUSB, 0x0, buf + index, maxpacket);
                    CSR0_InPktRdy_Set(pUSB);
                }
            }

            break;
        case AM_HAL_USB_EP0_STATE_STATUS_RX:
            // See 7.6 error handling
            // 1. The host sends more data during the OUT Data phase of a write request than was specified in the command.
            //    This condition is detected when the host sends an OUT token after the DataEnd bit (CSR0.D3) has been set.
            // 4. The host sends a non-zero length DATA1 packet during the STATUS phase of a read request.
            if (CSR0_SentStall(pUSB))
            {
                // Clear the SentStall bit
                CSR0_SentStall_Clear(pUSB);
                // Notify the upper layer USB stack to handle it
                am_hal_usb_xfer_complete(pState, &pState->ep0_xfer, 0x0, pState->ep0_xfer.len - pState->ep0_xfer.remaining, USB_XFER_STALL, NULL);
            }
            else
            {
                am_hal_usb_xfer_complete(pState, &pState->ep0_xfer, 0x0, pState->ep0_xfer.len - pState->ep0_xfer.remaining, USB_XFER_DONE, NULL);
            }
            am_hal_usb_ep0_state_reset(pState);

            //
            // check if a new next setup request is coming
            //
            am_hal_usb_ep0_handle_setup_req(pState, pUSB);
            break;

        case AM_HAL_USB_EP0_STATE_STATUS_TX:
            // See 7.6 error handling SendStall
            // 2. The host request more data during the IN Data phase of a read request than was specified in the command.
            // This condition is detected when the host sends an IN token after the DataEnd bit in the CSR0 register has been set.
            if (CSR0_SentStall(pUSB))
            {
                // Clear the SentStall bit
                CSR0_SentStall_Clear(pUSB);
                // Notify the upper layer USB stack to handle it
                am_hal_usb_xfer_complete(pState, &pState->ep0_xfer, 0x0 | AM_HAL_USB_EP_DIR_IN_MASK, pState->ep0_xfer.len - pState->ep0_xfer.remaining, USB_XFER_STALL, NULL);
            }
            else
            {
                am_hal_usb_xfer_complete(pState, &pState->ep0_xfer, 0x0 | AM_HAL_USB_EP_DIR_IN_MASK, pState->ep0_xfer.len - pState->ep0_xfer.remaining, USB_XFER_DONE, NULL);
            }
            am_hal_usb_ep0_state_reset(pState);

            //
            // check if a new setup request is coming
            //
            am_hal_usb_ep0_handle_setup_req(pState, pUSB);
            break;

        default:
            // Never come here
            break;
    }
}

//*****************************************************************************
//
// Bulk In endpoint interrupt handling
//
//*****************************************************************************
static void
am_hal_usb_in_ep_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum)
{
    am_hal_usb_ep_xfer_t *pXfer;
    uint32_t maxpacket;

    pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_IN];
    maxpacket = pState->epin_maxpackets[ui8EpNum - 1];

    EP_INDEX_Set(pUSB, ui8EpNum);

    // Note: automatic bulk packet splitting option is not enabled

    // 8.1.3 Bulk In endpoint error handling
    if (INCSRL_SentStall(pUSB))
    {
        // Just clear the SentStall bit
        // leave the SendStall bit set
        INCSRL_SentStall_Clear(pUSB);
        return;
    }

    // In endpoint FIFO is empty
    if ((INCSRL_FIFONotEmpty(pUSB) == 0) && pXfer->flags.busy)
    {
        // Packet has been sent out
        if (pXfer->remaining == 0x0)
        {
#ifdef AM_HAL_USB_FEATURE_ZERO_LENGTH_PACKET
            // If zero packet is needed
            if (pXfer->flags.zlp)
            {
                INCSRL_InPktRdy_Set(pUSB);
            }
#endif
            INTRINE_Disable(pUSB, 0x1 << ui8EpNum);
            am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum | AM_HAL_USB_EP_DIR_IN_MASK, pXfer->len, USB_XFER_DONE, NULL);
            return;
        }

        uint16_t min = pXfer->remaining < maxpacket ? pXfer->remaining : maxpacket;
        if (pState->eInXferMode == AM_HAL_USB_IN_DMA_MODE_0)
        {
            am_hal_usb_dma_loading_fifo(pUSB, ui8EpNum, pXfer->buf + pXfer->len - pXfer->remaining, min);
        }
        else
        {
            am_hal_usb_fifo_loading(pUSB, ui8EpNum, pXfer->buf + pXfer->len - pXfer->remaining, min);
            INCSRL_InPktRdy_Set(pUSB);
        }

        pXfer->remaining -= min;

        if (pXfer->remaining == 0x0 && min == maxpacket)
        {
            pXfer->flags.zlp = 1;
        }

        return;
    }
}

//*****************************************************************************
//
// Bulk In ADMA interrupt handling
//
//*****************************************************************************
static void
am_hal_usb_in_ep_dma1_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum)
{
    am_hal_usb_ep_xfer_t *pXfer;
    uint32_t maxpacket;

    pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_IN];
    maxpacket = pState->epin_maxpackets[ui8EpNum - 1];

    EP_INDEX_Set(pUSB, ui8EpNum);

    // Note: automatic bulk packet splitting option is not enabled

    // 8.1.3 Bulk In endpoint error handling
    if (INCSRL_SentStall(pUSB))
    {
        // Just clear the SentStall bit
        // leave the SendStall bit set
        INCSRL_SentStall_Clear(pUSB);
        return;
    }

    // If ADMA completed, Clear DMA Settings
    if (pUSB->ADMACMPINTSTAT & (0x1 << (ui8EpNum - 1)))
    {
        pUSB->ADMACMPINTCLR |= 1 << (ui8EpNum - 1);

        pUSB->ADMAEN &= ~(1 << (ui8EpNum - 1));

        INCSRU_AutoSet_Clear(pUSB);
        INCSRU_DMAReqEnab_Clear(pUSB);
        INCSRU_DMAReqMode_Clear(pUSB);

        // Set InPktRdy manually for small packet
        if (pXfer->len % maxpacket)
        {
            INCSRL_InPktRdy_Set(pUSB);
        }
    }

    // If Endpoint interrupt occurs, complete the transaction
    if ( (INCSRL_FIFONotEmpty(pUSB) == 0) && pXfer->flags.busy )
    {
        INTRINE_Disable(pUSB, 0x1 << ui8EpNum);
        pXfer->remaining = 0x0;
        am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum | AM_HAL_USB_EP_DIR_IN_MASK, pXfer->len, USB_XFER_DONE, NULL);
    }
}

//*****************************************************************************
//
// Bulk Out endpoint handling
//
//*****************************************************************************
static void
am_hal_usb_out_ep_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum)
{
    uint16_t count;
    uint16_t maxpacket;
    am_hal_usb_ep_xfer_t *pXfer;

    pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_OUT];

    EP_INDEX_Set(pUSB, ui8EpNum);

    // 8.2.3 Bulk Out endpoint error handling
    if (OUTCSRL_SentStall(pUSB))
    {
        // Just clear the SentStall bit
        // leave the SendStall bit set
        OUTCSRL_SentStall_Clear(pUSB);
        return;
    }

    if (OUTCSRL_OutPktRdy(pUSB))
    {
        count = OUTCOUNT(pUSB);
        maxpacket = pState->epout_maxpackets[ui8EpNum - 1];

        if (pXfer->remaining < count)
        {
            am_hal_usb_fifo_unloading(pUSB, ui8EpNum, pXfer->buf + pXfer->len - pXfer->remaining, pXfer->remaining);
            pXfer->remaining = 0;
        }
        else
        {
            // Buffer is enough to receive it
            am_hal_usb_fifo_unloading(pUSB, ui8EpNum, pXfer->buf + pXfer->len - pXfer->remaining, count);
            pXfer->remaining -= count;
        }

        if (pXfer->remaining == 0x0 || count < maxpacket)
        {
            INTROUTE_Disable(pUSB, 0x1 << ui8EpNum);
            am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum, pXfer->len - pXfer->remaining, USB_XFER_DONE, NULL);
            return;
        }
        OUTCSRL_OutPktRdy_Clear(pUSB);
    }
}

//*****************************************************************************
//
// DMA Mode 0 Bulk Out endpoint handling
//
//*****************************************************************************
static void
am_hal_usb_out_ep_dma0_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum)
{
    uint16_t count;
    uint16_t maxpacket;
    am_hal_usb_ep_xfer_t *pXfer;
    pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_OUT];

    EP_INDEX_Set(pUSB, ui8EpNum);

    // 8.2.3 Bulk Out endpoint error handling
    if (OUTCSRL_SentStall(pUSB))
    {
        // Just clear the SentStall bit
        // leave the SendStall bit set
        OUTCSRL_SentStall_Clear(pUSB);
        return;
    }

    // If this is the last packet we expect to received or we have received an ZLP, callback XFER_DONE event
    count = OUTCOUNT(pUSB);
    if ( (pXfer->flags.lastpkt) || ((count == 0) && (pXfer->len == pXfer->remaining)) )
    {
        INTROUTE_Disable(pUSB, 0x1 << ui8EpNum);
        am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum, pXfer->len - pXfer->remaining, USB_XFER_DONE, NULL);
    }
    // Else, Continue receiving
    else
    {
        if (OUTCSRL_OutPktRdy(pUSB))
        {
            count = OUTCOUNT(pUSB);
            maxpacket = pState->epout_maxpackets[ui8EpNum - 1];

            // Send Data using DMA Mode 0
            if (pXfer->remaining < count)
            {
                am_hal_usb_dma_unloading_fifo(pUSB, ui8EpNum, pXfer->buf + pXfer->len - pXfer->remaining, pXfer->remaining);
                pXfer->remaining = 0;
            }
            else
            {
                // Buffer is enough to receive it
                am_hal_usb_dma_unloading_fifo(pUSB, ui8EpNum, pXfer->buf + pXfer->len - pXfer->remaining, count);
                pXfer->remaining -= count;
            }

            // If it is short packet or data count exhuasted. Mark it as last packet.
            if ((pXfer->remaining == 0) || (count < maxpacket))
            {
                pXfer->flags.lastpkt = true;
            }
        }
    }
}

//*****************************************************************************
//
// ADMA mode Bulk Out EP interrupt handling
//
//*****************************************************************************
static void
am_hal_usb_out_ep_dma1_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum)
{
    uint16_t count;
    uint16_t maxpacket;
    uint32_t ui32XferLen;
    uint32_t ui32AdmaTarAddr;
    am_hal_usb_ep_xfer_t *pXfer;

    pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_OUT];

    EP_INDEX_Set(pUSB, ui8EpNum);

    // 8.2.3 Bulk Out endpoint error handling
    if (OUTCSRL_SentStall(pUSB))
    {
        // Just clear the SentStall bit
        // leave the SendStall bit set
        OUTCSRL_SentStall_Clear(pUSB);
        return;
    }

    if (OUTCSRL_OutPktRdy(pUSB))
    {
        count = OUTCOUNT(pUSB);
        maxpacket = pState->epout_maxpackets[ui8EpNum - 1];

        if (count < maxpacket )
        {
            if ( pXfer->len )
            {
                //
                // If ADMA complete interrupt and endpoint interrupt occurs,
                // handle ADMA complete interrupt instead. In this case,
                // endpoint interrupt is actually caused by data that are
                // pushed into FIFO after ADMA is completed.
                //
                if ( pUSB->ADMACMPINTSTAT & (1 << (ui8EpNum + 4)) )
                {
                    am_hal_usb_out_ep_dma1_adma_handling(pState, pUSB, ui8EpNum);
                    return;
                }

                pUSB->ADMACMPINTCLR |= 1 << (ui8EpNum + 4);

                pUSB->ADMAEN &= ~(1 << (ui8EpNum + 4));

                OUTCSRU_AutoClear_Clear(pUSB);

                ui32AdmaTarAddr = AM_REGVAL((uint32_t)&(pUSB->ADMATARGADDR0) + 4 * (ui8EpNum + 4));
                ui32XferLen = ui32AdmaTarAddr - (uint32_t)pXfer->buf + count;
                am_hal_usb_fifo_unloading(pUSB, ui8EpNum, (uint8_t *)ui32AdmaTarAddr, count);
                INTROUTE_Disable(pUSB, 0x1 << ui8EpNum);
                am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum, ui32XferLen, USB_XFER_DONE, NULL);
                OUTCSRL_OutPktRdy_Clear(pUSB);
                return;
            }
            else if (count == 0)
            {
                OUTCSRL_OutPktRdy_Clear(pUSB);
                return;
            }
        }

    }
}

//*****************************************************************************
//
// Bulk Out ADMA Complete Interrupt Handling
//
//*****************************************************************************
static void
am_hal_usb_out_ep_dma1_adma_handling(am_hal_usb_state_t *pState, USB_Type *pUSB, uint8_t ui8EpNum)
{
    uint32_t ui32XferLen;
    uint32_t ui32AdmaTarAddr;
    am_hal_usb_ep_xfer_t *pXfer;

    pXfer = &pState->ep_xfers[ui8EpNum - 1][AM_HAL_USB_EP_DIR_OUT];

    EP_INDEX_Set(pUSB, ui8EpNum);

    // 8.2.3 Bulk Out endpoint error handling
    if (OUTCSRL_SentStall(pUSB))
    {
        // Just clear the SentStall bit
        // leave the SendStall bit set
        OUTCSRL_SentStall_Clear(pUSB);
        return;
    }

    if ( pUSB->ADMACMPINTSTAT & (1 << (ui8EpNum + 4)) )
    {
        pUSB->ADMACMPINTCLR |= 1 << (ui8EpNum + 4);
        pUSB->ADMAEN &= ~(1 << (ui8EpNum + 4));
        OUTCSRU_AutoClear_Clear(pUSB);
        ui32AdmaTarAddr = AM_REGVAL((uint32_t)&(pUSB->ADMATARGADDR0) + 4 * (ui8EpNum + 4));
        ui32XferLen = ui32AdmaTarAddr - (uint32_t)pXfer->buf;
        INTROUTE_Disable(pUSB, 0x1 << ui8EpNum);
        am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum, ui32XferLen, USB_XFER_DONE, NULL);
        return;
    }
}

//*****************************************************************************
//
// USB interrupt service routine
//
//*****************************************************************************
void
am_hal_usb_interrupt_service(void *pHandle,
                             uint32_t ui32IntrUsbStatus,
                             uint32_t ui32IntrInStatus,
                             uint32_t ui32IntrOutStatus,
                             uint32_t ui32DMAStatus,
                             uint32_t ui32IntrAutoDMAStatus)
{
    uint8_t i;
    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return;
    }
#endif

    USB_Type *pUSB = USBn(pState->ui32Module);

    // Handling the resume interrupt
    if (ui32IntrUsbStatus & USB_INTRUSB_Resume_Msk)
    {
        //
        // Turning XCVRs on
        //
        USBPHY->REG10 |= 0x2;

        // Back to active state
        pState->eDevState = AM_HAL_USB_DEV_STATE_RESUMING;
        if (pState->dev_evt_callback)
        {
            pState->dev_evt_callback(AM_HAL_USB_DEV_EVT_RESUME);
        }
    }

    // Handling the reset interrupt
    if (ui32IntrUsbStatus & USB_INTRUSB_Reset_Msk)
    {
        // Back to the init state
        pState->eDevState = AM_HAL_USB_DEV_STATE_INIT;
        am_hal_usb_ep_xfer_t *pXfer = &pState->ep0_xfer;
        if (pXfer->flags.busy == 0x1)
        {
            // Notify the upper layer, ep0 transfer is aborted
            am_hal_usb_xfer_complete(pState, pXfer, am_hal_usb_ep_addr(0x0, pXfer->flags.dir), pXfer->len - pXfer->remaining, USB_XFER_ABORT, NULL);
        }
        am_hal_usb_ep0_state_reset(pState);

        for (int i = 0; i < AM_HAL_USB_EP_MAX_NUMBER; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                pXfer = &pState->ep_xfers[i][j];
                // Notify the upper layer, endpoint transfer is aborted
                if (pXfer->flags.busy)
                {
                    am_hal_usb_xfer_complete(pState, pXfer, am_hal_usb_ep_addr(i + 1, j), pXfer->len - pXfer->remaining, USB_XFER_ABORT, NULL);
                }
            }
        }
        memset((void *)&pState->ep_xfers, 0x0, sizeof(pState->ep_xfers));

        // Reset the ep fifo allocation
        am_hal_usb_ep_fifo_reset(&pState->ui32Allocated);

        // Disable all IN/OUT EP interrupts when USB is reset
        // when receiving the set configuration request
        // enable the related EP interrupts
        INTRINE_Disable(pUSB, AM_HAL_USB_EP_MASK);
        INTROUTE_Disable(pUSB, AM_HAL_USB_EP_MASK);

        // Disable the SOF interrupt
        // enable it only when upper layer stack need it
        INTRUSBE_SOF_Disable(pUSB);
        pState->sofReq.sofIntrReq = 0;

        //
        // Always enable the SuspendM
        //
        INTRUSBE_Suspend_Enable(pUSB);

        if (pState->dev_evt_callback)
        {
            pState->dev_evt_callback(AM_HAL_USB_DEV_EVT_BUS_RESET);
        }
    }

    // Handling the SOF interrupt if it is enabled
    if (ui32IntrUsbStatus & USB_INTRUSB_SOF_Msk)
    {
        #ifdef ENABLE_ADMA_OUT_XFER_END_DETECTION
        // If SOF interrupt is enabled by OUT_EP xfer
        uint8_t ui8OutEpIntrReq = pState->sofReq.sofIntrReq_b.out_ep;
        uint8_t ui8EpNum = 1;
        while (ui8OutEpIntrReq)
        {
            if (ui8OutEpIntrReq & 0x01)
            {
                am_hal_usb_ep_xfer_t* pXfer = &pState->ep_xfers[ui8EpNum-1][AM_HAL_USB_EP_DIR_OUT];

                // Get current ADMA target addr
                uint32_t ui32AdmaTarAddr = AM_REGVAL((uint32_t)&(pUSB->ADMATARGADDR0) + ((ui8EpNum + 4) << 2));

                if (pXfer->pStartBufAddr != ui32AdmaTarAddr)
                {
                    if (pXfer->pLastBufAddr == ui32AdmaTarAddr)
                    {
                        pXfer->timeout_counter --;

                        // If Transfer timeout
                        if ( pXfer->timeout_counter == 0 )
                        {
                            EP_INDEX_Set(pUSB, ui8EpNum);
                            pUSB->ADMAEN &= ~(1 << (ui8EpNum + 4));
                            OUTCSRU_AutoClear_Clear(pUSB);
                            uint32_t ui32XferLen = ui32AdmaTarAddr - (uint32_t)pXfer->buf;
                            INTROUTE_Disable(pUSB, 0x1 << ui8EpNum);
                            am_hal_usb_xfer_complete(pState, pXfer, ui8EpNum, ui32XferLen, USB_XFER_DONE, NULL);
                        }
                    }
                    else
                    {
                        pXfer->pLastBufAddr   = ui32AdmaTarAddr;
                        pXfer->timeout_counter = AM_HAL_USB_TIMEOUT;
                    }
                }
            }
            ui8OutEpIntrReq >>= 1;
            ui8EpNum++;
        }
        #endif

        if ( pState->sofReq.sofIntrReq_b.stack )
        {
            // Notify the SOF event
            if (pState->dev_evt_callback)
            {
                pState->dev_evt_callback(AM_HAL_USB_DEV_EVT_SOF);
            }
        }
    }

    if (ui32IntrInStatus)
    {
        // Handling the EP0 interrupt
        if (ui32IntrInStatus & USB_INTRIN_EP0_Msk)
        {
            am_hal_usb_ep0_handling(pState, pUSB);
            ui32IntrInStatus &= ~USB_INTRIN_EP0_Msk;
        }


        // Handling IN Endpoint one by one
        for (i = AM_HAL_USB_EP1_NUMBER; i <= AM_HAL_USB_EP_MAX_NUMBER; i++)
        {

            if (ui32IntrInStatus & (0x1 << i))
            {
                if ( pState->eInXferMode == AM_HAL_USB_IN_DMA_MODE_1 )
                {
                    am_hal_usb_in_ep_dma1_handling(pState, pUSB, i);
                }
                else
                {
                    am_hal_usb_in_ep_handling(pState, pUSB, i);
                }

            }
        }
    }

    // Handling OUT Endpoint one by one
    if ( ui32IntrOutStatus )
    {
        for (i = AM_HAL_USB_EP1_NUMBER; i <= AM_HAL_USB_EP_MAX_NUMBER; i++)
        {
            if (ui32IntrOutStatus & (0x1 << i))
            {
                if ( pState->eOutXferMode == AM_HAL_USB_OUT_DMA_MODE_1 )
                {

                    am_hal_usb_out_ep_dma1_handling(pState, pUSB, i);
                }
                else if ( pState->eOutXferMode == AM_HAL_USB_OUT_DMA_MODE_0 )
                {
                    am_hal_usb_out_ep_dma0_handling(pState, pUSB, i);
                }
                else
                {
                    am_hal_usb_out_ep_handling(pState, pUSB, i);
                }

            }
        }
    }

    if ( ui32DMAStatus )
    {
        // Disable DMA Mode0 Engine
        DMACTRL_DMAEN_Disable(pUSB);

        // Clear DMA Interrupt
        DMAINTSTAT_Clear(pUSB, ui32DMAStatus);

        // Clear DMA Mode0 busy status
        g_bUSBDMA0Busy = false;

        // Handle the completed DMA
        if ( pUSB->DMACTRL_b.DMADIR == AM_HAL_USB_IN_DIR )
        {
            // Select the endpoint by index register
            EP_INDEX_Set(pUSB, pUSB->DMACTRL_b.DMAEP);
            INCSRL_InPktRdy_Set(pUSB);
            INTRINE_Enable(pUSB, 0x1 << pUSB->DMACTRL_b.DMAEP);
        }
        else
        {
            am_hal_usb_ep_xfer_t* pXfer = &pState->ep_xfers[USB->DMACTRL_b.DMAEP - 1][AM_HAL_USB_EP_DIR_OUT];
            EP_INDEX_Set(pUSB, pUSB->DMACTRL_b.DMAEP);
            if ( pXfer->flags.lastpkt )
            {
                am_hal_usb_out_ep_dma0_handling(pState, pUSB, USB->DMACTRL_b.DMAEP);
            }
            else
            {
                OUTCSRL_OutPktRdy_Clear(pUSB);
            }
        }

        // Check whether there are more interrupt request in the queue
        if ( !am_hal_queue_empty(&g_sUSBDMA0Queue) )
        {
            am_hal_usb_dma0_queue_t sEnt;
            am_hal_queue_item_get(&g_sUSBDMA0Queue, &sEnt, 1);
            am_hal_usb_dma_mode0_configure(pUSB, sEnt.ui8EpNum, sEnt.eDir, sEnt.ui32BufAddr, sEnt.ui32Count);
        }
    }

    // Handling ADMA interrupts
    if ( ui32IntrAutoDMAStatus )
    {
        if (pState->eInXferMode == AM_HAL_USB_IN_DMA_MODE_1)
        {
            for (i = 0 ; i <= 4; i++)
            {
                if (ui32IntrAutoDMAStatus & (0x1 << i))
                {
                    am_hal_usb_in_ep_dma1_handling(pState, pUSB, i + 1);
                }
            }
        }

        if (pState->eOutXferMode == AM_HAL_USB_OUT_DMA_MODE_1)
        {
            for (i = 5 ; i <= 9; i++)
            {
                if (ui32IntrAutoDMAStatus & (0x1 << i))
                {
                    am_hal_usb_out_ep_dma1_adma_handling(pState, pUSB, i - 4);
                }
            }
        }
    }



    // Handling the suspend interrupt finally
    if (ui32IntrUsbStatus & USB_INTRUSB_Suspend_Msk)
    {
        //
        // Turning XCVRs off for more power saving
        //
        USBPHY->REG10 &= 0xFD;

        pState->eDevState = AM_HAL_USB_DEV_STATE_SUSPENDING;
        if (pState->dev_evt_callback)
        {
            pState->dev_evt_callback(AM_HAL_USB_DEV_EVT_SUSPEND);
        }
    }

    return;
}

//*****************************************************************************
//
// Set USB Xfer Mode
//
//*****************************************************************************
uint32_t
am_hal_usb_set_xfer_mode(void *pHandle, am_hal_usb_xfer_mode_e eXferMode)
{

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
#endif

    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    uint32_t ui32RetVal = AM_HAL_STATUS_SUCCESS;

    switch (eXferMode)
    {
        case AM_HAL_USB_OUT_PIO_MODE:
            pState->eOutXferMode = AM_HAL_USB_OUT_PIO_MODE;
            break;
        case AM_HAL_USB_OUT_DMA_MODE_0:
            pState->eOutXferMode = AM_HAL_USB_OUT_DMA_MODE_0;
            break;
        case AM_HAL_USB_OUT_DMA_MODE_1:
            pState->eOutXferMode = AM_HAL_USB_OUT_DMA_MODE_1;
            break;
        case AM_HAL_USB_IN_PIO_MODE:
            pState->eInXferMode = AM_HAL_USB_IN_PIO_MODE;
            break;
        case AM_HAL_USB_IN_DMA_MODE_0:
            pState->eInXferMode = AM_HAL_USB_IN_DMA_MODE_0;
            break;
        case AM_HAL_USB_IN_DMA_MODE_1:
            pState->eInXferMode = AM_HAL_USB_IN_DMA_MODE_1;
            break;
        default:
            ui32RetVal = AM_HAL_STATUS_INVALID_ARG;
            break;
    }

    return ui32RetVal;
}

//*****************************************************************************
//
// Enable/Disable Double Buffer for the Endpoint-Direction combination
//
//*****************************************************************************
uint32_t
am_hal_usb_enable_ep_double_buffer(void *pHandle, uint8_t epnum, am_hal_usb_xfer_dir_e dir, bool enable)
{
    #ifndef AM_HAL_DISABLE_API_VALIDATION
    if (!AM_HAL_USB_CHK_HANDLE(pHandle))
    {
        return AM_HAL_STATUS_INVALID_HANDLE;
    }
    #endif

    //
    // Check Endpoint Number
    //
    if ((epnum == 0) || (epnum > AM_HAL_USB_EP_MAX_NUMBER))
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    am_hal_usb_state_t *pState = (am_hal_usb_state_t *)pHandle;
    uint32_t ui32RetVal = AM_HAL_STATUS_SUCCESS;

    //
    // Actual setting of the configuration
    //
    am_hal_usb_dbugger_enable_t *config = (dir == AM_HAL_USB_OUT_DIR)? &pState->sDBufferConfig.out_ep:
                                                                       &pState->sDBufferConfig.in_ep;

    if (enable)
    {
        config->en |= (0x01 << (epnum - 1));
    }
    else
    {
        config->en &= ~(0x01 << (epnum - 1));
    }

    return ui32RetVal;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
