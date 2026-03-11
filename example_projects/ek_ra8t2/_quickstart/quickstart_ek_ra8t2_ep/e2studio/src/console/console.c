/*
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/**********************************************************************************************************************
 * File Name    : console.c
 * Version      : 1.0
 * Description  : Console implementation software layer.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 19.02.2026 1.00     First Release
 *********************************************************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "r_typedefs.h"
#include "console.h"
#include "jlink_console.h"

#define MINIMUM_TOKEN_LENGTH                        (5)               /* minimum length of a colour token */
#define MAXIMUM_TOKEN_LENGTH                        (13)              /* maximum length of a colour token */


/**********************************************************************************************************************
 * Function Name: console_write
 * Description  : wrapper for serial console output, limited to 300 characters in one call.
 * Argument     : buffer
 * Return Value : .
 *********************************************************************************************************************/
void console_write(const char *buffer)
{
    uint32_t length = strlen(buffer);

    if (length > 300)
    {
        length = 300;
    }

    /* Cast away const qualifier as print_to_console() does not take a const parameter.
     * The function does not modify the buffer contents despite the non-const signature. */
    print_to_console((char *)buffer);
}
/**********************************************************************************************************************
 End of function console_write
 *********************************************************************************************************************/


/*****************************************************************************
 * Function Name: get_colour
 *                Get the escape code string for the supplied colour tag
 * @param[in] char *string : the escape code
 * @param[out] bool_t *found : true if the tag was found, false if not
 * @retval the escape code for the colour tag, or the original string if there
 *         was no match
 ******************************************************************************/
static const char *get_colour(const char *string, bool *found)
{
    #define COLOURS     (13)
    const char_t * p_colour_codes[] = {
                                "[BLACK]", "\x1B[30m", "[RED]", "\x1B[91m", "[GREEN]", "\x1B[92m", "[YELLOW]",
                                "\x1B[93m","[BLUE]", "\x1B[94m", "[MAGENTA]", "\x1B[95m", "[CYAN]", "\x1B[96m",
                                "[WHITE]", "\x1B[97m","[ORANGE]", "\x1B[38;5;208m", "[PINK]", "\x1B[38;5;212m",
                                "[BROWN]", "\x1B[38;5;94m","[PURPLE]", "\x1B[35m", "[DARKYELLOW]", "\x1B[38;5;220m"
    };

    uint8_t i;

    for (i = 0; i < COLOURS; i++)
    {
        if (0 == strcmp(string, p_colour_codes[i << 1]))
        {
            *found = true;
            return (p_colour_codes[(i << 1) + 1]);
        }
    }

    *found = false;
    return (string);
}
/******************************************************************************
 * End of function get_colour
 ******************************************************************************/

/* ************************************************************************** */
/* Function Name: detokenise                                                  */
/*                Replace colour tokens with terminal colour escape codes     */
/* @param[in] input : input string possibly containing colour tokens          */
/* @param[out] output : string with colour tokens replaced with escape codes  */
/* @retval none                                                               */
/* ************************************************************************** */
static void detokenise(const char * input, char *output)
{
    int16_t  start_bracket_index;
    int16_t  end_bracket_index;
    int16_t  start_bracket_output_index;
    uint16_t i;
    uint16_t o;
    char_t   token[MAXIMUM_TOKEN_LENGTH + 1];
    uint8_t  token_length;
    bool     token_found;
    bool     token_replaced;

    const char_t * p_colour_code;

    start_bracket_index        = -1;
    end_bracket_index          = -1;
    start_bracket_output_index = 0;

    o = 0;

    /* scan the input string */
    for (i = 0; '\0' != input[i]; i++)
    {
        token_replaced = false;

        /* token start? */
        if ('[' == input[i])
        {
            /* Cast from uint16_t to int16_t as start_bracket_index is signed
             * to allow -1 as a 'not set' sentinel value. The cast is safe as
             * the maximum input length constraint ensures 'i' will not exceed
             * INT16_MAX (32767). */
            start_bracket_index        = (int16_t) i;
            start_bracket_output_index = (int16_t) o;
        }

        /* token end? */
        if (']' == input[i])
        {
            /* Cast from uint16_t to int16_t. The variable is signed to allow
             * -1 as a 'not set' sentinel value. The cast is safe as the
             * maximum input length constraint ensures 'i' will not exceed
             * INT16_MAX (32767). */
            end_bracket_index = (int16_t) i;

            /* check to see if we have a token */
            if (start_bracket_index >= 0)
            {
                /* Cast to uint8_t as the token length (including brackets)
                 * is bounded by MAXIMUM_TOKEN_LENGTH which is guaranteed to
                 * be within uint8_t range (0-255). */
                token_length = (uint8_t) ((end_bracket_index - start_bracket_index) + 1);

                if ((token_length >= MINIMUM_TOKEN_LENGTH) && (token_length <= MAXIMUM_TOKEN_LENGTH))
                {
                    /* copy the token to a buffer */
                    strncpy(token, &input[start_bracket_index], token_length);
                    token[token_length] = '\0';

                    /* check for a valid token */
                    p_colour_code = get_colour(token, &token_found);

                    /* if we have a colour token, then replace it in the output with the associated escape code */
                    if (token_found)
                    {
                        strcpy(&output[start_bracket_output_index], p_colour_code);

                        /* Cast strlen() return value to int16_t as the colour
                         * code string is short enough to fit within int16_t range.
                         * Cast the resulting sum to uint16_t to match the type of 'o';
                         * the result is guaranteed to be non-negative and within
                         * uint16_t range due to output buffer size constraints. */
                        o = (uint16_t)(start_bracket_output_index + (int16_t)strlen(p_colour_code));

                        token_replaced = true;
                    }
                }
            }

            /* reset and start looking for another token */
            start_bracket_index = -1;
            end_bracket_index   = -1;
        }

        /* if we didn't replace a token, then carry on copying input to output */
        if (!token_replaced)
        {
            output[o] = input[i];
            o++;
        }
    }

    /* terminate the output string */
    output[o] = '\0';
}
/******************************************************************************
 * End of function detokenise
 ******************************************************************************/

/*****************************************************************************
 * Function Name: printf_colour
 *                As printf_colour, but replaces colour tags with escape codes
 * @param[in] char *format : the format string
 * @param[in] ... : argument list, 0 or more parameters
 * @retval None
 ******************************************************************************/
void printf_colour(const char *format, ...)
{
    va_list     arglist;

    static char_t s_colour_format[256];
    static char_t s_final_buffer[256];

    va_start(arglist, format);

    /* replace colour tokens with terminal colour escape codes */
    detokenise(format, s_colour_format);

    vsprintf(s_final_buffer, s_colour_format, arglist);
    va_end(arglist);

    console_write(s_final_buffer);
}
/******************************************************************************
 * End of function printf_colour
 ******************************************************************************/

/******************************************************************************

 * Function Name: clear_console
 * Description  : wrapper function to clear the serial console screen.
 * Return Value : .
*******************************************************************************/
void clear_console(void)
{
    console_write("\x1b[2J\x1b[H");
}
/******************************************************************************
 * End of function clear_console
 ******************************************************************************/
