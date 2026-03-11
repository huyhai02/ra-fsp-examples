/*
 * Serial.c
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

// includes
#include "project.h"
#include "hal_data.h"
#include "serial_user.h"
#include "serial.h"

// modular variables
uint8_t rxBuffer[RX_BUFFER_SIZE]; //
uint8_t txBuffer[TX_BUFFER_SIZE]; //
uint8_t nextSerialTxOut = 0;
uint8_t nextSerialTxIn = 0;
uint8_t nextSerialRxIn = 0;
uint8_t nextSerialRx2Proc = 0;

uint16_t commError = 0;

// external variables

// Function to submit char and binary strings into the Xmit buffer
// it has a switch to decide if trailing zeros should be put into the buffer or ignored
uint8_t SendString(const char *_msg, uint16_t _len, strip_zeros _supressZeros, add_CRLF _add_crlf)
{
    uint8_t uartIdle = false;
    uint8_t status = 0;
    uint16_t freeBufferSpace;
    uint8_t i;
    uint16_t totalLength = _len;

    if (_add_crlf == AddCRLF)
        totalLength += 2;

    freeBufferSpace = CheckBuffer ();
    uartIdle = (nextSerialTxOut == nextSerialTxIn) ? true : false;

    if (totalLength < freeBufferSpace)
    {
        for (i = 0; i < _len; i++)
        {
            if ((_msg[i] != 0) || (_supressZeros == NoStripZeros))
            {
                txBuffer[nextSerialTxIn] = (uint8_t) _msg[i];
                if (++nextSerialTxIn >= TX_BUFFER_SIZE)
                    nextSerialTxIn = 0;
            }
        }

        if (_add_crlf == AddCRLF)
        {
            txBuffer[nextSerialTxIn] = '\r';
            if (++nextSerialTxIn >= TX_BUFFER_SIZE)
                nextSerialTxIn = 0;

            txBuffer[nextSerialTxIn] = '\n';
            if (++nextSerialTxIn >= TX_BUFFER_SIZE)
                nextSerialTxIn = 0;
        }

        if (uartIdle)
        {
            R_SCI_UART_Write (&vcp_0_ctrl, &txBuffer[nextSerialTxOut], sizeof(txBuffer[nextSerialTxOut]));
            if (++nextSerialTxOut >= TX_BUFFER_SIZE)
                nextSerialTxOut = 0;
        }
    }
    else
        status = 1;

    return status;
}

uint16_t CheckBuffer(void)
{
    uint16_t bufferAvailBytes = 0;

    if (nextSerialTxOut > nextSerialTxIn)
        bufferAvailBytes = nextSerialTxOut - nextSerialTxIn;
    else
        bufferAvailBytes = (uint16_t) (TX_BUFFER_SIZE - (nextSerialTxIn - nextSerialTxOut));

    return bufferAvailBytes;
}

uint8_t RxBufferEmpty(void)
{
    return (nextSerialRx2Proc == nextSerialRxIn) ? true : false;
}

//--------------------- RENESAS SPECIFIC ISR PROCESSING -------------------------
void vcp_0_cb(uart_callback_args_t *p_args)
{
//    volatile uint8_t char2Echo;   // use this variable for initial comm test
//                                    // take in from RX and echo back to TX
//    // With these two lines
//    char2Echo = (uint8_t)p_args->data;
//    R_SCI_UART_Write(&g_uart0_ctrl, &char2Echo, sizeof(char2Echo));

    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Received a character */
        case UART_EVENT_RX_CHAR:
        {
            rxBuffer[nextSerialRxIn] = (uint8_t) p_args->data;
            if (++nextSerialRxIn >= RX_BUFFER_SIZE)
                nextSerialRxIn = 0;

        }
        break;
            /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
        {
            if (nextSerialTxOut != nextSerialTxIn)
            {
                R_SCI_UART_Write (&UART_CTRL, &txBuffer[nextSerialTxOut], sizeof(txBuffer[nextSerialTxOut]));
                if (++nextSerialTxOut >= TX_BUFFER_SIZE)
                    nextSerialTxOut = 0;
            }
        }
        break;

        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        case UART_EVENT_ERR_OVERFLOW:
        case UART_EVENT_BREAK_DETECT:
        case UART_EVENT_TX_DATA_EMPTY:
        case UART_EVENT_RX_COMPLETE:
            commError++;
        break;
    }

}

