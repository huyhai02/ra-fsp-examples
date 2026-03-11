/*
 * ASCII_Numbers.h
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


#ifndef _ASCII_NUMBERS_H_
#define _ASCII_NUMBERS_H_


// Includes
#include "project.h"
#include <stdint.h>


// typedefs, structs, and enums



// defines



// private modular variables



// public variables



// public function prototypes
uint8_t ConvertASCII2UINT16(const char * numstring, uint8_t _len, const char _terminator, volatile uint16_t * value);


#endif
