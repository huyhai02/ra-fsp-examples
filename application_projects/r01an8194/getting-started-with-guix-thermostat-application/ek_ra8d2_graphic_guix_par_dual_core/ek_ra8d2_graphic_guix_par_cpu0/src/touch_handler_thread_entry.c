/*******************************************************************************************************
 * File Name    : touch_handler_thread_entry.c
 * Description  : Contains entry function of touch handler thread.
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "touch_handler_thread.h"
#include <stdint.h>
#include "common_utils.h"
#include "hal_data.h"
#include "bsp_api.h"

uint32_t touch_message = 0;
/* Touch Handler Thread entry function */
void touch_handler_thread_entry(void)
{
    int err = 0;
    UINT status = 0;
    GX_EVENT gxe = {0};

    /* Open IPC0 channel for touch data communication with CPU1 */
    err = R_IPC_Open(&g_ipc0_ctrl, &g_ipc0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }
    while (1)
    {
        /* Wait for touch data from CPU1 via IPC */
        status = tx_semaphore_get(&g_touch_semaphore, TX_WAIT_FOREVER);
        if (TX_SUCCESS != status)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }

        /* Process touch coordinates and generate GUIX events */
        if(touch_message != 0xFFFFFFFF)
        {
            uint16_t touch_x = (uint16_t)(touch_message & 0xFFFF);
            uint16_t touch_y = (uint16_t)((touch_message >> 16) & 0xFFFF);
            gxe.gx_event_payload.gx_event_pointdata.gx_point_x = touch_x;
            gxe.gx_event_payload.gx_event_pointdata.gx_point_y = touch_y;
            gxe.gx_event_type = GX_EVENT_PEN_DOWN;
            gx_system_event_send(&gxe);
        }
        else
        {
            if (GX_EVENT_PEN_DOWN == gxe.gx_event_type)
            {
                gxe.gx_event_type = GX_EVENT_PEN_UP;
                gx_system_event_send(&gxe);
            }
        }
        tx_thread_sleep(1);
    }
}

/*******************************************************************************************************************//**
 * @brief IPC callback - receives touch data from CPU1 and wakes touch handler thread
 **********************************************************************************************************************/
void ipc00_callback(ipc_callback_args_t *p_args)
{
    switch(p_args->event)
    {
        case IPC_EVENT_MESSAGE_RECEIVED:
        {
            /* Save touch data and wake handler thread */
            touch_message = p_args->message;
            tx_semaphore_put(&g_touch_semaphore);
            break;
        }
        default:
            break;
    }
}

