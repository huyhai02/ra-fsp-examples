/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _APP_COMMON_H_
#define _APP_COMMON_H_

#include "hal_data.h"

#define RESET_VALUE             (0x00)

void sys_tick_init(uint32_t tick_interval_ms);
uint32_t sys_tick_get(void);

#endif
