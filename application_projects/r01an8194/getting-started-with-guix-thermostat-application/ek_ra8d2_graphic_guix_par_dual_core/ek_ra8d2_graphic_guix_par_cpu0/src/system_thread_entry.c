/*******************************************************************************************************
 * File Name    : system_thread_entry.c
 * Description  : Contains entry function of system control.
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include <guix_gen/thermostat_resources.h>
#include <guix_gen/thermostat_specifications.h>
#include "ipc_event_handler_thread.h"
#include "system_thread.h"
#include "common_utils.h"
#include "brightness.h"
#include "ospi_b_control.h"
#include "bsp_linker_info.h"

/* Root Window and screen pointers */
GX_WINDOW*      p_splash_screen;
GX_WINDOW*      p_settings_screen;
GX_WINDOW*      p_thermostat_screen;
GX_WINDOW*      p_mainpage_screen;
GX_WINDOW*      p_help_screen;
GX_WINDOW_ROOT* p_root;

/***********************************************************************************************************************
 Private functions
 ***********************************************************************************************************************/
static void system_state_init(void);
static fsp_err_t gpt_timer_PWM_setup(void);


static volatile shared_data_t *share_memory = (shared_data_t *) BSP_PARTITION_SHARED_MEM_START;

/* System Thread entry function */
void system_thread_entry(void)
{
    fsp_err_t     err            = FSP_SUCCESS;
    UINT          status         = TX_SUCCESS;
    UINT          gx_err         = GX_SUCCESS;
    /* block allocation will be done and address based message exchange happens */
    system_msg_t *p_message = NULL;
    system_msg_t hd_message;
    p_message = &hd_message;

    /* Initialize OSPI flash driver */
    err = ospi_b_init();
    if (err != FSP_SUCCESS)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Set OSPI flash to Octal-SPI mode for high-speed access */
    err = ospi_b_set_protocol_to_opi();
    if (err != FSP_SUCCESS)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Enable XIP mode (execute code directly from OSPI flash) */
    err = ospi_b_xip_enter();
    if (err != FSP_SUCCESS)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Initialize system state variables (time, mode, temperature) */
    system_state_init();

    /* Initialize GUIX graphics library */
    gx_err = gx_system_initialize();
    if (TX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Configure main display and create root window */
    gx_err = gx_studio_display_configure(DISPLAY,
                                rm_guix_port_hw_initialize,
                                LANGUAGE_ENGLISH,
                                DISPLAY_THEME_1,
                                &p_root);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Assign canvas memory buffer for rendering */
    gx_err = gx_canvas_memory_define(p_root->gx_window_root_canvas,
                            rm_guix_port_canvas,
                            p_root->gx_window_root_canvas->gx_canvas_memory_size);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Create Splash screen widget and attach to root window */
    gx_err = gx_studio_named_widget_create("Splash", (GX_WIDGET *)p_root, (GX_WIDGET **)&p_splash_screen);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    gx_err = gx_studio_named_widget_create("Settings", GX_NULL, (GX_WIDGET **)&p_settings_screen);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }
    
    gx_err = gx_studio_named_widget_create("MainPage", GX_NULL, (GX_WIDGET **)&p_mainpage_screen);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    gx_err = gx_studio_named_widget_create("Thermostat", GX_NULL, (GX_WIDGET **)&p_thermostat_screen);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    gx_err = gx_studio_named_widget_create("Help", GX_NULL, (GX_WIDGET **)&p_help_screen);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Make root window visible on display */
    gx_err = gx_widget_show(p_root);
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Start CPU1 (secondary core) for sensor and touch processing */
    R_BSP_SecondaryCoreStart();

    /* Start GUIX system (begins event processing) */
    gx_err = gx_system_start();
    if (GX_SUCCESS != gx_err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    /* Setup GPT timer in PWM mode for LCD backlight brightness control */
    err = gpt_timer_PWM_setup();
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(FSP_ERR_ASSERTION);
    }

    while (1)
    {
        /* Wait for messages from GUIX or other threads */
        status = tx_queue_receive((TX_QUEUE*)&system_msg_queue, (VOID*)p_message, TX_WAIT_FOREVER);
        if (TX_SUCCESS != status)
        {
            APP_ERR_TRAP(FSP_ERR_ASSERTION);
        }
        
        /* Process message based on class code */
        switch (p_message->msg_id.event_b.class_code)
        {
            case SF_MESSAGE_EVENT_CLASS_TIME:
            {
                switch (p_message->msg_id.event_b.code)
                {
                case SF_MESSAGE_EVENT_HOUR_MODE_TOGGLE:
                    /* Toggle between 12-hour and 24-hour time format */
                    g_gui_state.hour_mode = (system_hour_mode_t)(!g_gui_state.hour_mode);
                    tx_mutex_get(&g_sys_mutex, TX_WAIT_FOREVER);
                    g_gui_state.time = share_memory->current_time;

                    /* Send GUIX event to update time */
                    send_hmi_message(GXEVENT_MSG_TIME_UPDATE);
                    tx_mutex_put(&g_sys_mutex);
                    break;
                case SF_MESSAGE_EVENT_SET_TIME:
                case SF_MESSAGE_EVENT_SET_DATE:
                {
                    /* Lock shared memory and update time/date on CPU1 */
                    err = safe_semaphore_take(&g_ipc_semaphore);
                    adjust_time(&p_message->msg_payload.time_payload.time);

                    /* Get date, time and Send GUIX event to update time */
                    tx_mutex_get(&g_sys_mutex, TX_WAIT_FOREVER);
                    g_gui_state.time = share_memory->current_time;
                    send_hmi_message(GXEVENT_MSG_TIME_UPDATE);
                    tx_mutex_put(&g_sys_mutex);

                    /* Signal CPU1 to update its RTC time */
                    R_IPC_EventGenerate(&g_ipc1_ctrl, IPC_GENERATE_EVENT_IRQ0);
                    break;
                }
                case SF_MESSAGE_EVENT_AM_PM_TOGGLE:
                {
                    rtc_time_t time = {0};
                    /* Toggle between AM and PM (add/subtract 12 hours) */
                    if (g_gui_state.time.tm_hour < 12)
                    {
                        time.tm_hour = 12;
                    }
                    else
                    {
                        time.tm_hour = -12;
                    }

                    /* Lock shared memory and apply time adjustment */
                    safe_semaphore_take(&g_ipc_semaphore);
                    adjust_time(&time);

                    /* Lock access to resource */
                    tx_mutex_get(&g_sys_mutex, TX_WAIT_FOREVER);
                    g_gui_state.time = share_memory->current_time;
                    send_hmi_message(GXEVENT_MSG_AM_PM_TOGGLE);
                    tx_mutex_put(&g_sys_mutex);
                    
                    /* Signal CPU1 to update its RTC time */
                    R_IPC_EventGenerate(&g_ipc1_ctrl, IPC_GENERATE_EVENT_IRQ0);
                    break;
                }
                default:
                    break;
                }
            }

        tx_thread_sleep(1);
        }
    }
}

/*******************************************************************************************************************//**
 * @brief Sets up GPT timer in PWM mode for LCD backlight brightness control.
 *        Opens GPT, enables timer, starts PWM output, and sets initial brightness.
 * @param[in] None
 * @retval FSP_SUCCESS  GPT/PWM setup completed successfully
 * @retval FSP_ERR_*    Error occurred during GPT initialization or configuration
 **********************************************************************************************************************/
static fsp_err_t gpt_timer_PWM_setup(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Open GPT module */
    err = R_GPT_Open(&g_timer_PWM_ctrl, &g_timer_PWM_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    /* Enable GPT Timer */
    err = R_GPT_Enable(&g_timer_PWM_ctrl);
    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    /* Start GPT PWM timer */
    err = R_GPT_Start(&g_timer_PWM_ctrl);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Set brightness (LCD backlight) level: 50 = (45+5) */
    g_gui_state.brightness = 45;
    brightness_up(&g_gui_state.brightness);

    return err;
}

/*******************************************************************************************************************//**
 * @brief Initializes the system state with default values for GUI operation.
 * @param[in] None
 * @retval None
 **********************************************************************************************************************/
static void system_state_init(void)
{
    /* Initialize system state */
    g_gui_state.fan_mode    = SYSTEM_FAN_MODE_AUTO;
    g_gui_state.fan_on      = false;
    g_gui_state.hour_mode   = SYSTEM_HOUR_MODE_12;
    g_gui_state.mode        = SYSTEM_MODE_OFF;
    g_gui_state.target_temp = 24;
    g_gui_state.temp_units  = SYSTEM_TEMP_UNITS_C;
}

