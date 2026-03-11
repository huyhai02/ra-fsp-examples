/*
 * Serial_user.c
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

#include "project.h"

#include "serial.h"
#include "serial_user.h"
#include "ASCII_numbers.h"
#include <string.h>
#include <stdio.h>

// packet management
uint8_t packetBuffer[16];
uint8_t inPacket = false;
uint8_t nextPacketChar = 0;
uint8_t processPacket = false;

uint8_t processBinaryPacket = false;
int16_t packetLength = -1;
uint8_t escapeDetected = false;

extern uint8_t rxBuffer[RX_BUFFER_SIZE];

// Process Variables from other modules/code
extern uint8_t flashEnable;
extern uint16_t onTimeSetting;
extern uint16_t offTimeSetting;
extern uint8_t enableBlueLED;
//extern uint8_t serialNo[SER_NO_LENGTH];
extern char refData[REF_DATA_LENGTH];

extern uint8_t writeReference;
extern uint8_t writeParameters;



// function to process the input buffer with an ASCII-based protocol
// to be placed in main loop and called of the buffer contains received characters
// not yet processed
// arguments: none
// returns: error code: 0 = no error (default)
uint8_t ProcessReceiveBuffer(void)
{
//  SendString((char const *)&rxBuffer[nextSerialRx2Proc], 1, StripZeros, NoAddCRLF);
  if (rxBuffer[nextSerialRx2Proc] == '$') {
    inPacket = true;
    packetBuffer[0] = rxBuffer[nextSerialRx2Proc];
    nextPacketChar = 1;
  }
  else {
    if (inPacket == true) {
        packetBuffer[nextPacketChar++] = rxBuffer[nextSerialRx2Proc];

        if (rxBuffer[nextSerialRx2Proc] == '\n') {
          processPacket = true;
          inPacket = false;
        }
    }
  }
  
  if (++nextSerialRx2Proc >= RX_BUFFER_SIZE) {
    nextSerialRx2Proc = 0;
  }

  return 0;
}


// Packet processing function. packet form: "$<cmd>[<parameter>]\n"
// for simplicity, commands are single characters.
// Each command is a case in a switch statement. Upper or lower case characters are valid.
uint8_t ProcessPacket(void)
{
    uint8_t errorCode = 0;
    uint16_t tempValue;
    char message[50];


    switch (packetBuffer[COMMAND_LOCATION]) {
    // list of commands
    // each command has intentional fall-thru for upper/lower case
    case 'f':     // r = turn on LED
    case 'F':
        flashEnable = (packetBuffer[PARAMETER_START_LOCATION] == '1') ? true : false;
        SendString("$f\n", strlen("$f\n"), NoStripZeros, NoAddCRLF);
        break;
    case 'n':     // n = adjust the on time
    case 'N':
        errorCode = ConvertASCII2UINT16((char*)&packetBuffer[PARAMETER_START_LOCATION], 5, '\n', &tempValue);
        if ((tempValue <= MAX_TIME) && (tempValue >= MIN_TIME)) {
            onTimeSetting = tempValue;
            sprintf(message, "$n%d\n", onTimeSetting);
            SendString(message, (uint16_t)strlen(message), NoStripZeros, NoAddCRLF);
        }
        break;
    case 's':     // s = adjust the off time
    case 'S':
        errorCode = ConvertASCII2UINT16((char*)&packetBuffer[PARAMETER_START_LOCATION], 5, '\n', &tempValue);
        if ((tempValue <= MAX_TIME) && (tempValue >= MIN_TIME)) {
            offTimeSetting = tempValue;
            sprintf(message, "$s%d\n", offTimeSetting);
            SendString(message, (uint16_t)strlen(message), NoStripZeros, NoAddCRLF);
        }
        break;
    case 'b':     // r = turn on Blue LED
    case 'B':
        enableBlueLED = (packetBuffer[PARAMETER_START_LOCATION] == '1') ? true : false;
        if (enableBlueLED == true) {
            if (refData[strlen(refData)-1]!= 'B') {
                refData[strlen(refData)]= 'B';
                refData[strlen(refData)+ 1]= 0;
            }
        }
        else {
            if (refData[strlen(refData)-1]== 'B') {
                refData[strlen(refData)-1]= 0;
            }
        }
        break;
    case 'i':     // change Serial number and Model Number in Ref data
    case 'I':
        tempValue = PARAMETER_START_LOCATION;
//        refData[2] = (char) packetBuffer[PARAMETER_START_LOCATION];
        while ((tempValue < SER_NO_LENGTH - 1) && (packetBuffer[tempValue] != '\n')) {
            refData[tempValue] = (char) packetBuffer[tempValue];
            tempValue++;
        }
//        refData[tempValue] = 0;
        sprintf(message, "$i\n%s\n", refData);
        SendString(message, (uint16_t)strlen(message), NoStripZeros, NoAddCRLF);
        break;
    case 'v':
    case 'V':
        sprintf(message, "$v\n%s\n", refData);
        SendString(message, (uint16_t)strlen(message), NoStripZeros, NoAddCRLF);
        break;
    case 'w':     // write out the parameters to the current section
    case 'W':
        writeParameters = true;
        SendString("$w\n", strlen("$w\n"), NoStripZeros, NoAddCRLF);
        break;
    case 'u':     // write out the reference info to the current section
    case 'U':
        writeReference = true;
        SendString("$u\n", strlen("$u\n"), NoStripZeros, NoAddCRLF);
        break;
    }

    processPacket = false;

    return errorCode;
}

