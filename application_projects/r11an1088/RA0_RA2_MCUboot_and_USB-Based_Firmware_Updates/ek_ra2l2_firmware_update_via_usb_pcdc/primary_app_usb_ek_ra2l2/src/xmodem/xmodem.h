/***********************************************************************************************************************
 * File Name    : xmodem.h
 * Description  : Contains XModem protocol macro definitions and function definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/


#ifndef _XMODEM_H
#define _XMODEM_H

#include "hal_data.h"

#define SOH					0x01
#define EOT					0x04
#define ACK					0x06
#define	NAK					0x15
#define CAN					0x18

typedef enum {
    XMODEM_SUCCESS = 0,

    XMODEM_ADDRESS_ERROR,
    XMODEM_COMMS_ERROR,
    XMODEM_TIMEOUT_ERROR,
    XMODEM_PROG_ERROR,
    XMODEM_SEQUENCE_ERROR,
    XMODEM_CHECKSUM_ERROR,

    XMODEM_DUPLICATE_PACKET,

    XMODEM_EOT_RECEIVED,
    XMODEM_PACKET_RECEIVED,
    XMODEM_COMPLETED,

    XMODEM_ERROR
} xmodem_status_t;

typedef struct
{
    fsp_err_t (*xmodem_comm_read)(uint8_t *p_dest, uint32_t *len, uint32_t timeout_milliseconds);
    fsp_err_t (*xmodem_comm_send)(uint8_t *p_src, uint32_t len);
} xmodem_config_t;

xmodem_status_t xmodem_init(xmodem_config_t *xmodem_cfg);
xmodem_status_t xmodem_download_and_program_flash(uint32_t flash_address);

#endif
