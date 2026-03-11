/*
 * Serial_user.h
 *
 *  Created on: 25JUN2023
 *      Author: VIn D'Agostino, D'Agostino Industries Group, Inc.
 */

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/

/***********************************************************************************************************************
 * This file is used with the contents of the "Serial_ASCII" module which is supplied for Renesas use by
 * D'Agostino Industries Group, Inc. It is part of a simple serial interface management system. "<module>_user" files
 * (.c and .h) are intended to be modified on an application by application basis, and <module> files (.c and .h) are
 * intended to be used as provided. <module_user.h is the only file required to be included in referencing files.
 **********************************************************************************************************************/


#ifndef _SERIAL_USER_
#define _SERIAL_USER_

#include "serial.h"


//Public #defines
#define TX_BUFFER_SIZE 50
#define RX_BUFFER_SIZE 50

// Packet Definition
#define COMMAND_LOCATION 1
#define PARAMETER_START_LOCATION 2

// Com port definitions
#define UART_CFG vcp_0_cfg
#define UART_CTRL vcp_0_ctrl


// public variables
extern uint8_t processPacket;

// prototypes

uint8_t ProcessReceiveBuffer(void);
uint8_t ProcessPacket(void);

#endif
