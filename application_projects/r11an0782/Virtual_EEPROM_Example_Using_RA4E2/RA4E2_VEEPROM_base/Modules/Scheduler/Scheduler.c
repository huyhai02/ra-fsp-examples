/*
 * Scheduler.c
 *
 *  Created on: 25JUN2023
 *      Author: VIn D'Agostino, D'Agostino Industries Group, Inc.
 */

#include "project.h"
#include <stdint.h>
#include "scheduler.h"



uint16_t ten_mS_Counter = TEN_mS_SEED;
uint8_t ten_mS_Flag = false;

uint16_t twentyfive_mS_Counter = TWENTYFIVE_mS_SEED;
uint8_t twentyfive_mS_Flag = false;

uint16_t hundred_mS_Counter = ONEHUNDRED_mS_SEED;
uint8_t hundred_mS_Flag = false;

uint16_t one_S_Counter = ONE_S_SEED;
uint8_t one_S_Flag = false;



void SysTick_Handler(void)
{
    ten_mS_Counter--;
    if (ten_mS_Counter == 0) {
      ten_mS_Flag = true;
      ten_mS_Counter = TEN_mS_SEED;
    }

    twentyfive_mS_Counter--;
    if (twentyfive_mS_Counter == 0) {
      twentyfive_mS_Flag = true;
      twentyfive_mS_Counter = TWENTYFIVE_mS_SEED;
    }

    hundred_mS_Counter--;
    if (hundred_mS_Counter == 0) {
      hundred_mS_Flag = true;
      hundred_mS_Counter = ONEHUNDRED_mS_SEED;
    }

    one_S_Counter--;
    if (one_S_Counter == 0) {
      one_S_Flag = true;
      one_S_Counter = ONE_S_SEED;
    }
}


