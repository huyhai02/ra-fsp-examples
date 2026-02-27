/***********************************************************************************************************************
 * File Name    : sau_uart_ep.h
 * Description  : Contains data structures and functions used in sau_uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef SAU_UART_EP_H_
#define SAU_UART_EP_H_

#include "common_utils.h"

/* Macro for RTT Viewer */
#define RTT_DELAY_VALUE     (200U)

/* Macros for SAU UART receive */
#define NULL_CHAR           ('\0')
#define CR_ASCII            (0x0D)
#define LF_ASCII            (0x0A)

#ifdef BOARD_RA0E3_FPB
#define BUFFER_LEN          (64U)
#define LENGTH              "64"
#else
#define BUFFER_LEN          (256U)
#define LENGTH              "256"
#endif

/* Macros for SAU UART transmit */
#define TIME_OUT_FOR_ONE_BYTE       (1000)
#define TIME_OUT_VALUE              (1U)
#define TIME_OUT_UNIT               (BSP_DELAY_UNITS_MICROSECONDS)

/* Macros for echo message */
#define ECHO_MESSAGE                "Received a string from RA board: "
#define ECHO_MESSAGE_LEN            (strlen(ECHO_MESSAGE))

#define EP_INFO         "\r\nThe example project demonstrates the basic functionality of the SAU UART driver on"\
                        "\r\nRenesas RA MCUs based on FSP. Upon successfully initializing the SAU UART module,"\
                        "\r\na message requesting the user to open the Tera Term application on the PC along"\
                        "\r\nwith the configuration parameters for the Tera Term application will be displayed on"\
                        "\r\nthe RTT Viewer. After opening and configuring the Tera Term application, the user can"\
                        "\r\nenter any ASCII character string with a length less than "LENGTH" characters into the"\
                        "\r\nTera Term application, then press enter-key to send it to the RA board. The RA Board"\
                        "\r\nwill echo any data received back to the Tera Term application on the host PC.\r\n"

#define EP_MESSAGE      "\r\nPlease open the Tera Term application on the PC and set up the serial connection to:"\
                        "\r\nThe COM port is facilitated by the PmodUSBUART, operating at a baud rate of 115200 bps,"\
                        "\r\nwith a data length of 8 bits, no parity check, one-bit stop, and without any "\
                        "flow control.\r\n\r\n"

/* Public function declaration */
void sau_uart_ep_entry(void);

#endif /* SAU_UART_EP_H_ */
