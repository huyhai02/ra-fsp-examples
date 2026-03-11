/*
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/**********************************************************************************************************************
 * File Name    : console.h
 * Version      : 1.0
 * Description  : console support functions interface
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 19.02.2026 1.00     First Release
 *********************************************************************************************************************/

#ifndef CONSOLE_CONSOLE_H_
#define CONSOLE_CONSOLE_H_

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/
extern void console_write (const char * buffer);
extern void printf_colour (const char * format, ...);

extern void clear_console (void);

#endif /* CONSOLE_CONSOLE_H_ */
