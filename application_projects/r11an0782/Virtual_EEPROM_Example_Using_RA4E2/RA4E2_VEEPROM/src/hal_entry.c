/*
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "hal_data.h"
#include "../Modules/Serial_ASCII/serial_user.h"
#include "../Modules/Scheduler/scheduler.h"
#include "veeprom.h"
#include <string.h>

extern bsp_leds_t g_bsp_leds;

uint8_t flashEnable = true;
uint8_t toggleMe = false;
uint16_t onTime = DEFAULT_ON_TIME;
uint16_t offTime = DEFAULT_OFF_TIME;

uint16_t onTimeSetting = DEFAULT_ON_TIME;
uint16_t offTimeSetting = DEFAULT_OFF_TIME;

uint8_t enableBlueLED = false;

uint8_t refData[REF_DATA_LENGTH] = DEFAULT_REF_DATA;

uint8_t writeReference = false;
uint8_t writeParameters = false;
uint8_t callback_called = false;
rm_vee_state_t vee_done_state;

// *** BEGIN FOR TESTING ONLY ***
// This variable will be used as a breakpoint.
// Declaring it volatile will ensure it does not get optimized out.
volatile uint8_t stopHere;
// *** END FOR TESTING ONLY ***

uint16_t numberOne;
uint16_t numberTwo;
uint16_t *p_numberOne;

/*******************************************************************************************************************//**
 * @brief  Blinky example application
 *
 * Blinks all leds at a rate of 1 second using the software delay function provided by the BSP.
 *
 **********************************************************************************************************************/
void hal_entry (void)
{
#if BSP_TZ_SECURE_BUILD

    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif

    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if (0 == leds.led_count)
    {
        while (1)
        {
            ;                          // There are no LEDs on this board
        }
    }

    /* Holds level to set for pins */
    bsp_io_level_t pin_level = BSP_IO_LEVEL_LOW;

    R_SCI_UART_Open (&UART_CTRL, &UART_CFG);

    SysTick_Config (SystemCoreClock / 1000);

    SendString (PROJ_REV, strlen (PROJ_REV), NoStripZeros, NoAddCRLF);

    OpenAndRetrieveVEEPROM_Values ();

    // ***** End of VEEPROM startup management *****

    while (1)
    {
        //---------------------------------
        // 10mS Tasks
        if (ten_mS_Flag)
        {
            ten_mS_Flag = false;

        }  // end of 10mS Tasks
           //---------------------------------

        //---------------------------------
        // 25mS Tasks
        if (twentyfive_mS_Flag)
        {
            twentyfive_mS_Flag = false;

            if (flashEnable == true)
            {
                if (BSP_IO_LEVEL_LOW == pin_level) // next state will be low, which means it is currently high
                {
                    if (--onTime == 0)
                    {
                        onTime = onTimeSetting;
                        toggleMe = true;
                    }
                }
                else // next state will be high, which means it is currently low
                {
                    if (--offTime == 0)
                    {
                        offTime = offTimeSetting;
                        toggleMe = true;
                    }
                }

                // once the on or off timer is expired, it's time to change the state of the pin
                if (toggleMe == true)
                {

                    /* Enable access to the PFS registers. If using r_ioport module then register protection is automatically
                     * handled. This code uses BSP IO functions to show how it is used.
                     */
                    R_BSP_PinAccessEnable ();

                    /* Update all board LEDs */
                    for (uint32_t i = 0; i < leds.led_count; i++)
                    {
                        /* Get pin to toggle */
                        uint32_t pin = leds.p_leds[i];

                        if ((enableBlueLED == true) || (i != BLUE_LED_INDEX))
                        {
                            /* Write to this pin */
                            R_BSP_PinWrite ((bsp_io_port_pin_t) pin, pin_level);
                        }
                    }

                    /* Protect PFS registers */
                    R_BSP_PinAccessDisable ();

                    /* Toggle level for next write */
                    if (BSP_IO_LEVEL_LOW == pin_level)
                    {
                        pin_level = BSP_IO_LEVEL_HIGH;
                    }
                    else
                    {
                        pin_level = BSP_IO_LEVEL_LOW;
                    }

                    toggleMe = false;
                }
            }
        }  // end of 25mS Tasks
           //---------------------------------

        //---------------------------------
        // 100mS Tasks
        if (hundred_mS_Flag)
        {
            hundred_mS_Flag = false;

            if (writeParameters == true)
            {
                WriteVEEPROM_Parameters ();

            }

            if (writeReference == true)
            {
                WriteVEEPROM_Reference ();

            }

        }  // end of 100mS Tasks
           //---------------------------------

        //---------------------------------
        // 1 Sec Tasks
        if (one_S_Flag)
        {
            one_S_Flag = false;

//          if (flashEnable == true) {
//              /* Enable access to the PFS registers. If using r_ioport module then register protection is automatically
//               * handled. This code uses BSP IO functions to show how it is used.
//               */
//              R_BSP_PinAccessEnable();
//
//              /* Update all board LEDs */
//              for (uint32_t i = 0; i < leds.led_count; i++)
//              {
//                  /* Get pin to toggle */
//                  uint32_t pin = leds.p_leds[i];
//
//                  /* Write to this pin */
//                  R_BSP_PinWrite((bsp_io_port_pin_t) pin, pin_level);
//              }
//
//              /* Protect PFS registers */
//              R_BSP_PinAccessDisable();
//
//              /* Toggle level for next write */
//              if (BSP_IO_LEVEL_LOW == pin_level)
//              {
//                  pin_level = BSP_IO_LEVEL_HIGH;
//              }
//              else
//              {
//                  pin_level = BSP_IO_LEVEL_LOW;
//              }
//          }

        } // end of 1Sec Tasks
          //---------------------------------

        //---------------------------------
        // Every time through the loop

        if (processPacket == true)
        {
            ProcessPacket ();
        }

        if (!RxBufferEmpty ())
        {
            ProcessReceiveBuffer ();
        }

        // end Every time through the loop
        //---------------------------------

    }
}

/* Callback function */
void vee_callback(rm_vee_callback_args_t *p_args)
{
    callback_called = true;
    vee_done_state = p_args->state;
}
