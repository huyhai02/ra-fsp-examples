/***********************************************************************************************************************
 * File Name    : touch_thread_entry.c
 * Description  : Contains entry function of touch panel control.
 **********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#include "common_utils.h"
#include "touch_thread.h"
#include "r_ipc.h"
#include <stdint.h>
#include "hal_data.h"
#include "bsp_api.h"

static touch_data_t touch_data;

/* Touch Thread entry function */
void touch_thread_entry(void)
{
    UINT status = 0;
    fsp_err_t err = FSP_SUCCESS;
    
    /* Initialize FT5x06 touch controller and IRQ */
    ft5x06_init(&g_i2c_touch, &g_i2c_semaphore, GLCDC_RESET_L);
    R_ICU_ExternalIrqOpen(g_touch_irq.p_ctrl, g_touch_irq.p_cfg);
    R_ICU_ExternalIrqEnable(g_touch_irq.p_ctrl);

    /* Open IPC0 channel to CPU0 */
    err = R_IPC_Open(&g_ipc0_ctrl, &g_ipc0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }
    while (1)
    {
        /* Wait for touch interrupt */
        status = tx_semaphore_get(&g_touch_semaphore, TX_WAIT_FOREVER);
        if (TX_SUCCESS != status)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }

        /* Read and encode touch data for IPC transmission */
        ft5x06_payload_get(&touch_data);
        uint32_t touch_message = 0;
        if (1 == touch_data.num_points)
        {
            touch_message = (uint32_t)touch_data.point[0].x |
                           ((uint32_t)touch_data.point[0].y << 16);
        }
        else
        {
            touch_message = 0xFFFFFFFF;
        }

        /* Send touch data to CPU0 */
        R_IPC_MessageSend(&g_ipc0_ctrl, touch_message);

        tx_thread_sleep(1);
    }
}

/*******************************************************************************************************************//**
 * @brief Touch IRQ callback - wakes touch thread when FT5x06 interrupt occurs
 **********************************************************************************************************************/
void touch_irq_cb(external_irq_callback_args_t * p_args)
{
    if (19 == p_args->channel)
    {
        /* Wake touch thread immediately */
        tx_semaphore_ceiling_put(&g_touch_semaphore, 1);
    }
}
