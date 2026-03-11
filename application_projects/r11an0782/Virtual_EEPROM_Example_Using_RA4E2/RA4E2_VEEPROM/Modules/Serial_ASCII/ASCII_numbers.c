// Number Conversions
/*
 * ascii_numbers.c
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
#include "ASCII_numbers.h"


// modular variables


// external variables



// Code
uint8_t ConvertASCII2UINT16(const char * numstring, uint8_t _len, const char _terminator, volatile uint16_t * _number)
{
  uint8_t i;
  uint8_t error;
  uint8_t done;
  uint16_t value;
  
  
  error = 0;
  i = 0;
  value = 0;
  done = false;
  
  while ((i< _len) && (done != true) && (error == 0)) {
    if ((numstring[i] >= '0') && (numstring[i] <= '9')) { 
      value *=10;
      value += (uint16_t)(numstring[i] - 0x30);
      i++;
      if ((numstring[i] == _terminator) || (numstring[i] == 0)) {
        done = true;
      }
    }
    else {
      error = 1;
    }
  }
  
  if (error == 0) {
    *_number = value;
  }
  
  return error;
}
