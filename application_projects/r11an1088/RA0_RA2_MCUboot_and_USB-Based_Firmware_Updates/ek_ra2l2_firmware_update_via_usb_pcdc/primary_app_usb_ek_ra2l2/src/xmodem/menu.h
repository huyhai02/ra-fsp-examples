/***********************************************************************************************************************
 * File Name    : menu.h
 * Description  : Contains menu implementation macro definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/

#include <stdio.h>

#ifndef MENU_H_
#define MENU_H_

#include "hal_data.h"

typedef struct {
    fsp_err_t       ret;
    uint8_t         data;
} app_menu_info_t;

app_menu_info_t menu_process(void);
void menu_display(void);

#endif /* MENU_H_ */
