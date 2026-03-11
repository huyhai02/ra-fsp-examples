/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "app_led.h"
#include "app_common.h"

static uint32_t tick_led_ms = 0;
static uint8_t led_index = 0;

fsp_err_t app_led(bsp_leds_t *leds)
{
    R_BSP_PinAccessEnable();

    if (sys_tick_get() - tick_led_ms >= 1000)
    {
        tick_led_ms = sys_tick_get();

        if (led_index < leds->led_count)
        {
            uint32_t pin = leds->p_leds[led_index];
            R_BSP_PinWrite((bsp_io_port_pin_t) pin, BSP_IO_LEVEL_HIGH);
        }
        else
        {
            for (uint32_t i = 0; i < leds->led_count; i++)
            {
                uint32_t pin = leds->p_leds[i];
                R_BSP_PinWrite((bsp_io_port_pin_t) pin, BSP_IO_LEVEL_LOW);
            }
            led_index = 0;
            return FSP_SUCCESS;
        }

        led_index++;
    }

    R_BSP_PinAccessDisable();

    return FSP_SUCCESS;
}
