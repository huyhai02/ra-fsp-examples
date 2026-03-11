/***********************************************************************************************************************
 * File Name    : common_utils.c
 * Description  : Contains common utility functions used across the application.
 **********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "common_data.h"
#include "system_services_thread.h"

/*******************************************************************************************************************//**
 * @brief Take IPC semaphore with retry logic for shared memory protection
 **********************************************************************************************************************/
fsp_err_t safe_semaphore_take(const bsp_ipc_semaphore_handle_t *semaphore)
{
    fsp_err_t err = FSP_SUCCESS;
    
    /* Try to acquire semaphore, retry if in use */
    err = R_BSP_IpcSemaphoreTake(semaphore);
    while(FSP_SUCCESS != err)
    {
        if(FSP_ERR_IN_USE == err)
        {
            err = R_BSP_IpcSemaphoreTake(semaphore);
        }
        else if (FSP_ERR_IP_CHANNEL_NOT_PRESENT == err)
        {
            break;
        }
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief Initialize RTC with default time and enable 1-second periodic interrupt
 **********************************************************************************************************************/
void system_time_init(void)
{
    fsp_err_t err = FSP_SUCCESS;
    
    /* Default time: 12:00:00 01/01/2024 */
    rtc_time_t default_time = {
        .tm_sec   = TM_SEC,
        .tm_min   = TM_MIN,
        .tm_hour  = TM_HOUR,
        .tm_mday  = TM_MDAY,
        .tm_mon   = TM_MON,
        .tm_year  = TM_YEAR,
        .tm_wday  = TM_WDAY,
        .tm_yday  = TM_YDAY,
        .tm_isdst = TM_ISDST
    };

    /* Open RTC and configure periodic interrupt */
    err = R_RTC_Open(&g_rtc_ctrl, &g_rtc_cfg);
    if (FSP_SUCCESS == err)
    {
        err = R_RTC_PeriodicIrqRateSet(&g_rtc_ctrl, RTC_PERIODIC_IRQ_SELECT_1_SECOND);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }

        err = R_RTC_CalendarTimeSet(&g_rtc_ctrl, &default_time);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }
    }
    else
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }
}
