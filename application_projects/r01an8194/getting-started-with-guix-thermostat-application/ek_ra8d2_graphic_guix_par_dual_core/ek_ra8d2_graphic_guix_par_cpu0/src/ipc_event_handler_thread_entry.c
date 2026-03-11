/*******************************************************************************************************
 * File Name    : ipc_event_handler_thread_entry.c
 * Description  : Contains entry function of IPC event handler thread.
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#include "ipc_event_handler_thread.h"
#include <stdint.h>
#include "hal_data.h"
#include "common_utils.h"
#include "bsp_linker_info.h"

static volatile shared_data_t *share_memory = (shared_data_t *) BSP_PARTITION_SHARED_MEM_START;

/* Ipc Event Handler Thread entry function */
void ipc_event_handler_thread_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;
    UINT status   = 0;

    /* Open IPC1 channel to CPU1 for sensor data */
    err = R_IPC_Open(&g_ipc1_ctrl, &g_ipc1_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    while (1)
    {
        /* Wait for sensor data from CPU1 */
        status = tx_semaphore_get(&g_rtc_semaphore, TX_WAIT_FOREVER);
        if (TX_SUCCESS != status)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }

        /* Update GUI with time and temperature */
        tx_mutex_get(&g_sys_mutex, TX_WAIT_FOREVER);
        send_hmi_message(GXEVENT_MSG_TIME_UPDATE);
        tx_mutex_put(&g_sys_mutex);
        send_hmi_message(GXEVENT_MSG_UPDATE_TEMPERATURE);

        /* Acknowledge CPU1 */
        R_IPC_EventGenerate(&g_ipc1_ctrl, IPC_GENERATE_EVENT_IRQ1);

        tx_thread_sleep(1);
    }
}

/*******************************************************************************************************************//**
 * @brief IPC callback - copies sensor data from shared memory and wakes handler thread
 **********************************************************************************************************************/
void ipc01_callback(ipc_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case IPC_EVENT_IRQ0:
            /* Release semaphore when CPU1 acknowledges */
            R_BSP_IpcSemaphoreGive(&g_ipc_semaphore);
            break;
        case IPC_EVENT_IRQ1:
            /* Copy sensor data and wake handler thread */
            g_gui_state.time = share_memory->current_time;
            g_gui_state.temp_c = ADCTEMP_AS_C(share_memory->temperature);
            tx_semaphore_put(&g_rtc_semaphore);
            break;
        default:
            break;
    }
}

