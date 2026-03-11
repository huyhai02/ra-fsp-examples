/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "app_led.h"
#include "app_common.h"

static uint32_t tick_led_ms = 0;
static bsp_io_level_t pin_level = BSP_IO_LEVEL_LOW;

fsp_err_t app_led(bsp_leds_t *leds)
{
    R_BSP_PinAccessEnable();

    if (sys_tick_get() - tick_led_ms >= 1000)
    {
        tick_led_ms = sys_tick_get();

        pin_level = (pin_level == BSP_IO_LEVEL_HIGH) ? BSP_IO_LEVEL_LOW : BSP_IO_LEVEL_HIGH;

        for (uint8_t i = 0; i < leds->led_count; i++)
        {
            uint32_t pin = leds->p_leds[i];
            R_BSP_PinWrite((bsp_io_port_pin_t) pin, pin_level);
        }
    }

    R_BSP_PinAccessDisable();

    return FSP_SUCCESS;
}
