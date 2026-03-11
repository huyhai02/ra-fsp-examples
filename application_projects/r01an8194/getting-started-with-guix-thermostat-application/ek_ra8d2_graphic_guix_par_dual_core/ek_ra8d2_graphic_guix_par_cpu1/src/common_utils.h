/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains data structures and functions used in common_utils.c.
 **********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2023 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <Touch_FT5x06/touch_ft5x06.h>
#include "r_rtc_api.h"
#include "bsp_api.h"

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef struct {
    rtc_time_t     current_time;
    uint16_t       temperature;
} shared_data_t;
/***********************************************************************************************************************
Function prototypes
***********************************************************************************************************************/
void system_time_init(void);
fsp_err_t safe_semaphore_take(const bsp_ipc_semaphore_handle_t *semaphore);
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define APP_ERR_TRAP(err)           if(err) { __asm("BKPT #0\n");} /* trap upon the error  */ // @suppress("3.9b, 3.9f, 8.4b Macro expansion")

/* Default Time: Thursday 9:30:00 20 March 2025, daylight saving time */
#define TM_SEC                   (0U)
#define TM_MIN                   (30U)
#define TM_HOUR                  (9U)
#define TM_MDAY                  (20U)
#define TM_MON                   (2U)
#define TM_YEAR                  (125U)
#define TM_WDAY                  (4U)
#define TM_YDAY                  (78U)
#define TM_ISDST                 (1U)

#endif /* COMMON_UTILS_H */
