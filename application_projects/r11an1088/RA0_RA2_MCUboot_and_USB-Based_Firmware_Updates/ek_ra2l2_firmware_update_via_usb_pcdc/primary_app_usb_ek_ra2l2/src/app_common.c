/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "app_common.h"

static volatile uint32_t g_sys_tick_cnt = 0;
static uint32_t g_value_load = 0;

void sys_tick_init(uint32_t tick_interval_ms)
{
    g_value_load = SystemCoreClock / tick_interval_ms;

    uint32_t err = SysTick_Config(g_value_load);
    assert(err == 0);
}

void SysTick_Handler(void)
{
    g_sys_tick_cnt++;
}

uint32_t sys_tick_get(void)
{
    return g_sys_tick_cnt;
}
