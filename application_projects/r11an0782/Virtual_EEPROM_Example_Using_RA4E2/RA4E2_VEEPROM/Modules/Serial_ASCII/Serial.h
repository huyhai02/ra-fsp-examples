/*
 * Serial.h
 *
 *  Created on: 25JUN2023
 *      Author: VIn D'Agostino, D'Agostino Industries Group, Inc.
 */


#ifndef _SERIAL_H_
#define _SERIAL_H_


// Includes
//#include "main.h"
#include "project.h"
#include <stdint.h>


// typedefs, structs, and enums
typedef enum _strip_zeros_ {
  StripZeros,
  NoStripZeros
} strip_zeros;

typedef enum _add_crlf_ {
  AddCRLF,
  NoAddCRLF
} add_CRLF;


// public variables
extern uint8_t nextSerialTxOut;
extern uint8_t nextSerialTxIn;
extern uint8_t nextSerialRxIn;
extern uint8_t nextSerialRx2Proc;


// public function prototypes
uint8_t SendString(const char * _msg, uint16_t _len, strip_zeros _supressZeros, add_CRLF _add_crlf);
uint16_t CheckBuffer(void);
uint8_t RxBufferEmpty(void);


#endif
