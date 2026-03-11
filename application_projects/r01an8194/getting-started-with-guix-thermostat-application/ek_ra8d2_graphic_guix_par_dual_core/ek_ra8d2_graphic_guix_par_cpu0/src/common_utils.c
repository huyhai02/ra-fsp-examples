/***********************************************************************************************************************
 * File Name    : common_utils.c
 * Description  : Contains common utility functions and definitions.
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ***********************************************************************************************************************/

#include "common_utils.h"
#include "bsp_linker_info.h"

/* Global variable definition */
/** Stored GUI state */
/* Place g_gui_state in uncached memory region to avoid cache coherency issues */
BSP_PLACE_IN_SECTION(".ram_noinit_nocache") system_state_t g_gui_state;
GX_EVENT gxe = {0};


/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static const uint8_t month_table[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
static const uint8_t ly_month_table[12] = {6, 2, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5};
static const uint8_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/***********************************************************************************************************************
 * Private functions
 **********************************************************************************************************************/
static int weekday_get(rtc_time_t * p_time);
static void time_validate(int * time, int max, bool day_flag);
/*******************************************************************************************************************//**
 * @brief Send event message to GUIX event queue with current system state
 **********************************************************************************************************************/
void send_hmi_message(gx_event_message_t event)
{
    UINT gx_err = GX_SUCCESS;
    system_payload_t message_to_gx = {0};

    /* Build message with event type and system state */
    message_to_gx.gx_event.gx_event_type = event;
    message_to_gx.state = g_gui_state;
    message_to_gx.gx_event.gx_event_target = GX_NULL;
    message_to_gx.gx_event.gx_event_payload.gx_event_ulongdata = (ULONG)&message_to_gx;

    /* Send to GUIX event queue */
    gx_err = gx_system_event_send(&message_to_gx.gx_event);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }
}

/*******************************************************************************************************************//**
 * @brief Take IPC semaphore with retry logic for shared memory protection
 **********************************************************************************************************************/
fsp_err_t safe_semaphore_take(const bsp_ipc_semaphore_handle_t *semaphore)
{
    fsp_err_t err = FSP_SUCCESS;
    
    /* Try to acquire semaphore, retry if in use */
    err = R_BSP_IpcSemaphoreTake(semaphore);
    while (FSP_SUCCESS != err)
    {
        if (FSP_ERR_IN_USE == err)
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
 * @brief Validate and wrap time values (0-based for time units, 1-based for days)
 **********************************************************************************************************************/
static void time_validate(int * p_time, int max, bool day_flag)
{
    if (day_flag == false)
    {
        /* Time units: wrap 0 to max-1 */
        if (*p_time < 0)
        {
            *p_time = max - 1;
        }
        if (*p_time >= max)
        {
            *p_time = 0;
        }
    }
    else
    {
        /* Days: wrap 1 to max-1 */
        if (*p_time <= 0)
        {
            *p_time = max - 1;
        }
        if (*p_time >= max)
        {
            *p_time = 1;
        }
    }
}

/*******************************************************************************************************************//**
 * @brief Adjust RTC time in shared memory by specified deltas with leap year handling
 **********************************************************************************************************************/
void adjust_time(rtc_time_t * p_time)
{
    rtc_time_t t;
    shared_data_t *share_memory = (shared_data_t *) BSP_PARTITION_SHARED_MEM_START;

    /* Read current time from shared memory */
    memcpy(&t, (void *)&share_memory->current_time, sizeof(rtc_time_t));

    /* Apply time adjustments */
    t.tm_sec += p_time->tm_sec;
    t.tm_min += p_time->tm_min;
    t.tm_hour += p_time->tm_hour;
    t.tm_mday += p_time->tm_mday;
    t.tm_mon += p_time->tm_mon;
    t.tm_year += p_time->tm_year;

    /* Check for leap year */
    bool leap = false;
    if (0 == t.tm_year % 4)
    {
        leap = true;
        if (0 == t.tm_year % 100)
        {
            leap = false;
            if (0 != t.tm_year % 400)
            {
                leap = true;
            }
        }
    }

    /* Validate time fields with wrap-around */
    time_validate(&t.tm_sec, 60, false);
    time_validate(&t.tm_min, 60, false);
    time_validate(&t.tm_hour, 24, false);
    time_validate(&t.tm_mon, 12, false);

    /* Validate days based on current month and leap year */
    int32_t temp_month = (t.tm_mon + 12) % 12;
    uint8_t days_in_current_month = days_in_month[temp_month];
    if (leap && (1 == temp_month))
    {
        days_in_current_month++;
    }
    time_validate(&t.tm_mday, days_in_current_month+1, true);
    
    /* Recalculate weekday */
    t.tm_wday = weekday_get(&t);

    /* Write adjusted time back to shared memory */
    memcpy((void *)&share_memory->current_time,&t,sizeof(rtc_time_t));
}

/*******************************************************************************************************************//**
 * @brief Calculate weekday using Zeller's algorithm with leap year adjustment
 **********************************************************************************************************************/
static int weekday_get(rtc_time_t * p_time)
{
    int32_t day = p_time->tm_mday;
    
    /* Check for leap year */
    bool leap = false;
    if (0 == p_time->tm_year % 4)
    {
        leap = true;
        if (0 == p_time->tm_year % 100)
        {
            leap = false;
            if (0 != p_time->tm_year % 400)
            {
                leap = true;
            }
        }
    }
    
    /* Calculate weekday using lookup tables and offsets */
    int32_t month_offset;
    if (leap)
    {
        month_offset = ly_month_table[p_time->tm_mon];
    }
    else
    {
        month_offset = month_table[p_time->tm_mon];
    }
    int32_t year_offset = p_time->tm_year % 100;
    int32_t century_offset = (3 - ((p_time->tm_year / 100) % 4)) * 2;
    int32_t offset = day + month_offset + year_offset + year_offset / 4 + century_offset;
    int32_t index = offset % 7;

    return (index);
}
