/***********************************************************************************************************************
 * File Name    : menu.c
 * Description  : Contains menu item implementations
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/

#include "stdbool.h"
#include "bsp_api.h"
#include "menu.h"
#include "info_header.h"
#include "comms.h"
#include "xmodem.h"

flash_result_t blank_check_result = FLASH_RESULT_BLANK;

void menu_display(void)
{
    uint8_t tx_str[80];

    sprintf((char *)tx_str, "\n\r");
    comms_send(tx_str, strlen((char *)tx_str));
    sprintf((char *)tx_str, "Please select from below menu options:\n\r");
    comms_send(tx_str, strlen((char *)tx_str));

    sprintf((char *)tx_str, "\n\r1 - Display image slot info\n\r");
    comms_send(tx_str, strlen((char *)tx_str));
    sprintf((char *)tx_str, "2 - Download and boot the new image (XModem)\n\r");
    comms_send(tx_str, strlen((char *)tx_str));

    comms_send((uint8_t *)">", 1);
}

app_menu_info_t menu_process(void)
{
    app_menu_info_t app_menu_info = {.ret = FSP_SUCCESS, .data = 0};

    uint8_t tx_str[80] = {0};
    uint8_t rx_str[20] = {0};

    rx_str[0] = 0;
    uint32_t len = 1;
    app_menu_info.ret = comms_read(rx_str, &len, INPUT_TIMEOUT);
    if (app_menu_info.ret != FSP_SUCCESS)
    {
        return app_menu_info;
    }

    comms_send(rx_str, 1);
    comms_send((uint8_t *)"\n\r", 2);

    app_menu_info.data = rx_str[0];

    switch (rx_str[0])
    {
        case '1':
            display_image_slot_info();
        	break;

        case '2':
            /* Download a new non-secure image in the secondary slot using the XModem protocol */
            /* Erase the secondary slot first */
            /* Check if it needs erasing first */
            sprintf((char *)tx_str, "Blank checking the secondary slot...\r\n");
            comms_send(tx_str, strlen((char *)tx_str));

            bool is_blank = true;
            uint8_t * p_src;

            p_src = (uint8_t *) SECONDARY_IMAGE_START_ADDRESS;
            do {
                if (*p_src != 0xFF)
                {
                    is_blank = false;
                }
                p_src++;
            } while ((is_blank == true) && (p_src < (uint8_t *)SECONDARY_IMAGE_END_ADDRESS));

            if (true == is_blank)
            {
                sprintf((char *)tx_str, "The secondary slot blank\r\n");
                comms_send(tx_str, strlen((char *)tx_str));
            }
            else
            {
                sprintf((char *)tx_str, "Erasing the secondary slot...\r\n");
                comms_send(tx_str, strlen((char *)tx_str));
                interrupt_control(DISABLE);
                app_menu_info.ret = R_FLASH_LP_Erase(&g_flash0_ctrl, (uint32_t) SECONDARY_IMAGE_START_ADDRESS, SECONDARY_IMAGE_NUM_BLOCKS);

                if (FSP_SUCCESS == app_menu_info.ret)
                {
                    app_menu_info.ret = R_FLASH_LP_BlankCheck(&g_flash0_ctrl, SECONDARY_IMAGE_START_ADDRESS, SECONDARY_IMAGE_END_ADDRESS - SECONDARY_IMAGE_START_ADDRESS, &blank_check_result);

                    if (FSP_SUCCESS == app_menu_info.ret)
                    {
                        if (FLASH_RESULT_BLANK != blank_check_result)
                        {
                            sprintf((char *)tx_str, "ERROR: Flash is not blank after erasing\r\n");
                            comms_send(tx_str, strlen((char *)tx_str));
                        }
                    }
                }
                interrupt_control(RE_ENABLE);

                if (FSP_SUCCESS == app_menu_info.ret)
                {
                    sprintf((char *)tx_str, "Secondary slot erased\r\n");
                    comms_send(tx_str, strlen((char *)tx_str));
                }
                else
                {
                    sprintf((char *)tx_str, "ERROR: Erasing the secondary slot\r\n");
                    comms_send(tx_str, strlen((char *)tx_str));

                    break;
                }
            }

            /* XModem download and flash programming */
            sprintf((char *)tx_str, "Please start Xmodem transfer on Terminal\r\n");
            comms_send(tx_str, strlen((char *)tx_str));
            sprintf((char *)tx_str, "Start Xmodem transfer...\r\n");
            comms_send(tx_str, strlen((char *)tx_str));
            xmodem_status_t status = xmodem_download_and_program_flash(SECONDARY_IMAGE_START_ADDRESS);
            if (XMODEM_SUCCESS != status)
            {
                switch (status)
                {
                    case XMODEM_ADDRESS_ERROR:
                        sprintf((char *)tx_str, "ERROR: Flash address invalid\r\n");
                        break;
                    case XMODEM_TIMEOUT_ERROR:
                        sprintf((char *)tx_str, "ERROR: Timeout during Xmodem download\r\n");
                        break;

                    case XMODEM_PROG_ERROR:
                        sprintf((char *)tx_str, "ERROR: Flash programming error\r\n");
                        break;
                    case XMODEM_COMPLETED:
                        sprintf((char *)tx_str, "INFO: Resetting the system\r\n");
                        break;
                    default:
                        sprintf((char *)tx_str, "ERROR: unknown (%d)\r\n", status);
                        break;
                }

                comms_send(tx_str, strlen((char *)tx_str));

                if (status == XMODEM_COMPLETED)
                {
                    NVIC_SystemReset();
                }
            }
        	break;
        default:
            break;
    }

    return app_menu_info;
}

void interrupt_control(enable_disable_t enable_disable)
{
    static uint32_t control_reg_value;
    static uint32_t old_primask;

    if (DISABLE == enable_disable)
    {
        /** Store the interrupt state */
        old_primask = __get_PRIMASK();

        /* Disable other threads whilst flash erasing */
        /* Disable the SysTick timer */
        control_reg_value = SysTick->CTRL;
        SysTick->CTRL = 0;
        NVIC_DisableIRQ( SysTick_IRQn ); /* Disable the SysTick timer IRQ */
        NVIC_ClearPendingIRQ( SysTick_IRQn ); /* Clear any pending SysTick timer IRQ */

        __disable_irq(); /* Disable interrupts */
    }
    else
    {
        NVIC_EnableIRQ( SysTick_IRQn ); /* Enable the SysTick timer IRQ */
        SysTick->CTRL = control_reg_value; /* Restart the SysTick timer */

        /** Restore the interrupt state */
        __set_PRIMASK( old_primask ); /* Enable interrupts */
    }
}
