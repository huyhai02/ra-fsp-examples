/***********************************************************************************************************************
 * File Name    : system_services_thread_entry.c
 * Description  : Contains entry function of system services thread for Core 1.
 **********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "hal_data.h"
#include <string.h>
#include "system_services_thread.h"
#include "bsp_linker_info.h"

static fsp_err_t ADC_Setup(void);

volatile shared_data_t share_memory BSP_PLACE_IN_SECTION(".shared_mem") = {0};

/* system_services_thread_entry function */
void system_services_thread_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint16_t adc_temp_data = 0;
    rtc_time_t time;
    UINT status = TX_SUCCESS;

    /* Initialize RTC, ADC and IPC1 communication */
    system_time_init();
    err = R_IPC_Open(&g_ipc1_ctrl, &g_ipc1_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }
    err = ADC_Setup();
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    while (1)
    {
        /* Wait for RTC 1-second interrupt */
        status = tx_semaphore_get(&g_timer_semaphore, TX_WAIT_FOREVER);
        if (TX_SUCCESS != status)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }

        /* Update shared memory with RTC time and temperature data */
        R_RTC_CalendarTimeGet(&g_rtc_ctrl, &time);
        safe_semaphore_take(&g_ipc_semaphore);
        memcpy( (void*)&share_memory.current_time, &time, sizeof(rtc_time_t));
        err = R_ADC_B_Read(&g_adc_ctrl, ADC_CHANNEL_TEMPERATURE, &adc_temp_data);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_TRAP(err);
        }
        share_memory.temperature = adc_temp_data;

        /* Signal CPU0 that sensor data is ready */
        R_IPC_EventGenerate(&g_ipc1_ctrl, IPC_GENERATE_EVENT_IRQ1);

        tx_thread_sleep(1);
    }
}

/*******************************************************************************************************************//**
 * @brief IPC callback - handles RTC time updates and shared memory synchronization with CPU0
 **********************************************************************************************************************/
void ipc11_callback(ipc_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case IPC_EVENT_IRQ0:
            /* Update RTC time and acknowledge */
            R_RTC_CalendarTimeSet(&g_rtc_ctrl, (rtc_time_t*)&share_memory.current_time);
            R_IPC_EventGenerate(&g_ipc1_ctrl, IPC_GENERATE_EVENT_IRQ0);
            break;
        case IPC_EVENT_IRQ1:
            /* Release shared memory semaphore */
            R_BSP_IpcSemaphoreGive(&g_ipc_semaphore);
            break;
        default:
            break;
    }
}

/*******************************************************************************************************************//**
 * @brief RTC periodic interrupt callback - wakes system services thread every second
 **********************************************************************************************************************/
void time_update_callback(rtc_callback_args_t * p_context)
{
    if (RTC_EVENT_PERIODIC_IRQ == p_context->event)
    {
        tx_semaphore_put(&g_timer_semaphore);
    }
}

/*******************************************************************************************************************//**
 * @brief Setup ADC for temperature sensor - open, configure, calibrate and start scanning
 **********************************************************************************************************************/
static fsp_err_t ADC_Setup(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Open and configure ADC for temperature channel */
    fsp_err = R_ADC_B_Open(&g_adc_ctrl, &g_adc_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }
    fsp_err = R_ADC_B_ScanCfg(&g_adc_ctrl, &g_adc_scan_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    /* Calibrate and wait for completion */
    fsp_err = R_ADC_B_Calibrate(&g_adc_ctrl, NULL);
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }
    adc_status_t adc_status = {.state = ADC_STATE_CALIBRATION_IN_PROGRESS};
    while ((ADC_STATE_IDLE != adc_status.state) && (FSP_SUCCESS == fsp_err))
    {
        R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
        fsp_err = R_ADC_B_StatusGet(&g_adc_ctrl, &adc_status);
    }

    /* Start continuous scanning */
    fsp_err = R_ADC_B_ScanStart(&g_adc_ctrl);
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    return fsp_err;
}
