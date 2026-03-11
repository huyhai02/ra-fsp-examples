/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains common utility functions and definitions.
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ***********************************************************************************************************************/

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include "system_config.h"

/* Global variables - extern declarations */
extern system_state_t g_gui_state;

typedef struct {
    USHORT id;
    GX_WIDGET *widget;
} screen_entry_t;

#define APP_ERR_TRAP(err)           if(err) { __asm("BKPT #0\n");}

/* Common utility functions */
void send_hmi_message(gx_event_message_t event);
fsp_err_t safe_semaphore_take(const bsp_ipc_semaphore_handle_t *semaphore);
void adjust_time(rtc_time_t * p_time);

#endif /* COMMON_UTILS_H */
