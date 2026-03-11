/*
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/**********************************************************************************************************************
 * File Name    : r_console_portable.c
 * Version      : 1.0
 * Description  : Console portable configuration implementation.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 19.02.2026 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "r_typedefs.h"
#include "r_console_portable.h"
#include "jlink_console.h"
#include "console.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Local Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/
extern int R_JLINK_InitialiseReadConsole ();

/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/

extern char_t jlink_read_byte (void);

/**********************************************************************************************************************
 * Function Name: R_Wait_For_Input
 * Description  : Blocking console wait function.
 * Argument     :
 * Return Value : .
 *********************************************************************************************************************/
void R_Wait_For_Input()
{
    R_JLINK_InitialiseReadConsole();

    while (!R_Is_Input_Ready())
    {
        __asm volatile ("nop");
    }
}
/**********************************************************************************************************************
 End of function R_Wait_For_Input
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_Wait_For_Any_Key
 * Description  : Helper function used for blocking wait used when no intention to parse input is indicated.
 * Argument     :
 * Return Value : .
 *********************************************************************************************************************/
void R_Wait_For_Any_Key()
{
    R_Empty_Input_Buffer();

    R_Wait_For_Input();
}
/**********************************************************************************************************************
 End of function R_Wait_For_Any_Key
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function Name: R_Empty_Input_Buffer
 * Description  : Helper function to Empty input buffers, add implementation if required.
 * Argument     :
 * Return Value : .
 *********************************************************************************************************************/
void R_Empty_Input_Buffer()
{
    return;

}
/**********************************************************************************************************************
 End of function R_Empty_Input_Buffer
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function Name: R_Read_Byte
 * Description  : Read single byte into (buffer (supplied).
 * Argument     : read_byte_buffer
 * Return Value : err (to be supported in future implementation).
 *********************************************************************************************************************/
int_t R_Read_Byte(char_t *read_byte_buffer)
{
    int_t err = 0;

    R_JLINK_InitialiseReadConsole();

    R_Wait_For_Input();

    *read_byte_buffer = jlink_read_byte();

    return (err);
}
/**********************************************************************************************************************
 End of function R_Read_Byte
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_Read_String
 * Description  : Reads multiple bytes of data into buffer (supplied).
 * Arguments    : buffer - store for resulting string (egual to or exceeding max_length)
 *              : max_length - limit of string read size.
 * Return Value : length - number of bytes successfully read.
 *********************************************************************************************************************/
uint16_t R_Read_String(char *buffer, size_t max_length)
{
    volatile uint16_t length = 0;
    char_t key;

    char_t local_str[24] = "";

    length = 0;

    /* string entry, terminated by the enter key */
    do
    {
        R_Read_Byte(&key);

        switch (key)
        {
            case '\r': case '\n':
            {
                break;
            }

            case ' ':   // space used as delete, rtt doesn't decode 'del' key
            {
                if (length > 0)
                {
                    length--;
                    local_str[length] = '\0';
                    printf_colour("\r%s  \r %s", local_str, local_str);
                }

                break;
            }

            default:
            {
                if (length < (max_length - 1))
                {
                    local_str[length++] = key;
                    printf_colour("%s", local_str);
                    printf_colour("\n");
                }

                break;
            }
        }

    }
    while (('\r' != key) && ('\n' != key));

    if (strlen(local_str) > 0)
    {
        printf_colour("USER IP address entered [%s]:%d\r\n", local_str, strlen(local_str));
    }

    if (strlen(local_str) > 0)
    {
        memcpy(buffer, &local_str[0], strlen(local_str));
    }

    return (length);
}
/**********************************************************************************************************************
 End of function R_Read_String
 *********************************************************************************************************************/

