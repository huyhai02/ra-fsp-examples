/***********************************************************************************************************************
* File Name    : r_sci_b_uart_instance_cfg.c
* Description  : Contains macros, data structures, and common functions used for the UART configuration.
***********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#if (USE_VIRTUAL_COM == 1)
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

#if BSP_PERIPHERAL_SAU_UART_PRESENT
#include "r_sau_uart_instance_cfg.h"
#include "../serial.h"

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

sau_uart_instance_ctrl_t g_serial_ctrl;

sau_uart_baudrate_setting_t g_serial_baud_setting =
{
/* Actual calculated bitrate: 115942 */
.stclk = 68,
  .prs = 1, .operation_clock = SAU_UART_OPERATION_CLOCK_CK0, };

/** UART extended configuration for UARTonSCI HAL driver */
const sau_uart_extended_cfg_t g_serial_cfg_extend =
{ .sequence = SAU_UART_DATA_SEQUENCE_LSB, .signal_level = SAU_UART_SIGNAL_LEVEL_STANDARD, .p_baudrate =
          &g_serial_baud_setting, };

/** UART interface configuration */
const uart_cfg_t g_serial_cfg =
{ .channel = SERIAL_CHANNEL, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          serial_callback,
    .rxi_ipl = (2), .txi_ipl = (2), .eri_ipl = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_UARTX_RXI)
                .rxi_irq             = VECTOR_NUMBER_UARTX_RXI,
#else
    .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_UARTX_TXI)
                .txi_irq             = VECTOR_NUMBER_UARTX_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_UARTX_ERI)
                .eri_irq             = VECTOR_NUMBER_UARTX_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
  .p_context = NULL,
  .p_extend = &g_serial_cfg_extend,

#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx   = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
        };

/* Instance structure to use this module. */
const uart_instance_t g_serial =
{ .p_ctrl = &g_serial_ctrl, .p_cfg = &g_serial_cfg, .p_api = &g_uart_on_sau };

#endif /* BSP_PERIPHERAL_SAU_UART_PRESENT */
#endif /* USE_VIRTUAL_COM */
