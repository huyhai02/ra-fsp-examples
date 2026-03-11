/***********************************************************************************************************************
 * File Name    : terminal_thread_entry.c
 * Description  : Contains functions from the terminal thread
 ***********************************************************************************************************************/
/**********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
**********************************************************************************************************************/
#include <common_app.h>
#include "terminal_thread.h"
#include "filex.h"

/***********************************************************************************************************************
 * Private functions
 **********************************************************************************************************************/
UINT terminal_output_handle(void);
UINT terminal_input_handle(void);
static void conv_ul64_to_str(ULONG64 value, CHAR * p_buf, UINT len);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static CHAR *gp_menu_content [MENU_MAX] =
{
     [MENU_MAIN] = MAIN_MENU,
     [MENU_FLASH_MEDIA] = FLASH_MEDIA_MENU,
     [MENU_DIR] = DIRECTORY_MENU,
     [MENU_FILE] = FILE_MENU,
	 [MENU_SECTOR] = SECTOR_MENU
};

/***********************************************************************************************************************
 *  Function Name: terminal_thread_entry
 *  Description  : This function performs Terminal Thread entry operation.
 *  Arguments    : None
 *  Return Value : None
 **********************************************************************************************************************/
void terminal_thread_entry(void)
{
    UINT status = TX_SUCCESS;

#if (USE_VIRTUAL_COM == 1)
    status = serial_init();
    if (FSP_SUCCESS != status)
    {
        /* Error trap */
        ERROR_TRAP;
    }
#endif /* USE_VIRTUAL_COM */

    status = term_framework_init();
    if (TX_SUCCESS != status)
    {
    	APP_PRINT("\r\nERROR: terminal_framework_init Failed\r\n");
    	APP_PRINT("\r\nReturned Error Code: 0x%x  \r\n", status);
        ERROR_TRAP;
    }

    /* Send a start event to the Console thread */
    status = (ULONG)tx_event_flags_set (&g_request_event,
                                           REQUEST_START_CONSOLE,
                                           TX_OR);
    if (TX_SUCCESS != status)
    {
    	APP_PRINT("\r\nERROR: tx_event_flags_set failed\r\n");
    	ERROR_TRAP;
    }

    tx_thread_sleep (TERMINAL_SLEEP_TICK);

    while (true)
    {
        /* Send a output queue to the Terminal thread to display menu */

        /* Handle user input from terminal  */
        status = terminal_input_handle();
        if (TX_SUCCESS != status)
        {
            APP_PRINT("\r\nERROR: terminal_input_hanlde Failed\r\n");
            APP_PRINT("\r\nReturned Error Code: 0x%x  \r\n", status);
            ERROR_TRAP;
        }

        /* Handle other threads that need output to the terminal application via terminal interface */
        status = terminal_output_handle();
        if (TX_SUCCESS != status)
        {
            APP_PRINT("\r\nERROR: terminal_output_hanlde Failed\r\n");
            APP_PRINT("\r\nReturned Error Code: 0x%x  \r\n", status);
            ERROR_TRAP;
        }

        tx_thread_sleep (TERMINAL_SLEEP_TICK);
    }
}

/***********************************************************************************************************************
 *  Function Name: terminal_output_handle
 *  Description  : This function transfers a message to the PC based on the terminal frame taken from the terminal output queue.
 *  Arguments    : None
 *  Return Value : TX_SUCCESS    Upon successful operation
 *                 Any Other Error code apart from TX_SUCCESS
 **********************************************************************************************************************/
UINT terminal_output_handle(void)
{
    UINT status = TX_SUCCESS;
    UINT err_code = RESET_VALUE;
    uint8_t menu = RESET_VALUE;
    terminal_msg_t * p_terminal_msg = NULL;
    CHAR str_size [STR_UL64_MAX_LEN] = {RESET_VALUE};
    CHAR str_size_align [STR_UL64_MAX_LEN] = {RESET_VALUE};
    fsp_pack_version_t version = {RESET_VALUE};
    entry_info_t entry = {RESET_VALUE};
    dir_property_t dir = {RESET_VALUE};
    media_property_t media = {RESET_VALUE};
    sector_info_t sector = {RESET_VALUE};

    while (true)
    {
        /* Retrieve a message from output queue */
        status = term_get_output_queue((void *)&p_terminal_msg, TX_NO_WAIT);
        if (TX_SUCCESS == status)
        {
            switch (p_terminal_msg->id)
            {
                case TERMINAL_OUTPUT_EP_BANNER:
                    /* Get FSP package information */
                    R_FSP_VersionGet(&version);

                    /* Print the EP banner to terminal application on PC */
                    APP_PRINT(BANNER_INFO, EP_VERSION,
                            version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);
                    break;

                case TERMINAL_OUTPUT_EP_INFO:
                    /* Print the EP information to terminal application on PC */
                    APP_PRINT(EP_INFO);
                    break;

                case TERMINAL_OUTPUT_APP_INFO_STR:
                    APP_PRINT("%s", p_terminal_msg->msg);
                    break;

                case TERMINAL_OUTPUT_APP_ERR_STR:
                    APP_PRINT("\r\nERROR : %s\r\n", p_terminal_msg->msg);
                    break;

                case TERMINAL_OUTPUT_APP_ERR_TRAP:
                    err_code = *(UINT*)p_terminal_msg->msg;
                    APP_PRINT("\r\nReturned Error Code: 0x%x  \r\n", err_code);
                    ERROR_TRAP;
                    break;

                case TERMINAL_OUTPUT_APP_MENU:
                    menu = *(uint8_t*)p_terminal_msg->msg;
                    APP_PRINT(gp_menu_content[menu]);
                    break;

                case TERMINAL_OUTPUT_APP_ENTRY_INFO:
                    entry = *(entry_info_t*)p_terminal_msg->msg;
                    if(FX_DIRECTORY == (entry.attr & FX_DIRECTORY))
                    {
                        /* Print directory information */
                        APP_PRINT("%.2u/%.2u/%.4u  %.2u:%.2u:%.2u    %s %s\r\n",
                                  entry.time.month, entry.time.date, entry.time.year,
                                  entry.time.hour, entry.time.min, entry.time.sec,
                                  "<DIR>          ", entry.name);
                    }
                    else if (FX_ARCHIVE == (entry.attr & FX_ARCHIVE))
                    {
                        /* Print file information */
                        conv_ul64_to_str (entry.size, str_size, STR_UL64_MAX_LEN);
                        snprintf (str_size_align, STR_UL64_MAX_LEN, "%15s", str_size);
                        APP_PRINT("%.2u/%.2u/%.4u  %.2u:%.2u:%.2u    %s %s\r\n",
                                entry.time.month, entry.time.date, entry.time.year,
                                entry.time.hour, entry.time.min, entry.time.sec,
                                str_size_align, entry.name);
                    }
                    else
                    {
                        /* Do nothing */
                    }
                    break;

                case TERMINAL_OUTPUT_APP_DIR_PROPERTY:
                    dir = *(dir_property_t*)p_terminal_msg->msg;
                    /* Print directory property */
                    conv_ul64_to_str (dir.size, str_size, STR_UL64_MAX_LEN);
                    snprintf (str_size_align, STR_UL64_MAX_LEN, "%15s", str_size);
                    APP_PRINT("         %6u File(s) %s bytes\r\n", dir.file, str_size_align);
                    APP_PRINT("         %6u Dir(s)\r\n\r\n", dir.subdir);
                    break;

                case TERMINAL_OUTPUT_APP_MEDIA_PROPERTY:
                    media = *(media_property_t*)p_terminal_msg->msg;

                    conv_ul64_to_str (media.total_size, str_size, STR_UL64_MAX_LEN);
                    APP_PRINT("Media total size is:   %15s bytes\r\n", str_size);

                    conv_ul64_to_str (media.free_size, str_size, STR_UL64_MAX_LEN);
                    APP_PRINT("Media free size is:    %15s bytes\r\n", str_size);

                    APP_PRINT("Format type of flash is:    %s", media.format_type);

                break;

                case TERMINAL_OUTPUT_SECTOR_INFO:
                    sector = *(sector_info_t*)p_terminal_msg->msg;
                    APP_PRINT("\r\nSector allocated at address:            0x%x", sector.sector_address);
                break;

                default:
                    break;
            }

            /* Release terminal message allocated block */
            status = tx_byte_release((void *)p_terminal_msg);
            if (TX_SUCCESS != status)
            {
                /* Print a error message to terminal application on PC */
                APP_PRINT("\r\nERROR : tx_byte_release failed\r\n");
                return status;
            }
        }

        /* In case queue is empty*/
        else if (TX_QUEUE_EMPTY == status)
        {
            return TX_SUCCESS;
        }

        /* In case the API is error */
        else
        {
            /* Print a error message to terminal application on PC */
            APP_PRINT("\r\nERROR : tx_queue_receive failed\r\n");
            return status;
        }
    }

    return status;
}

/*******************************************************************************************************************//**
 * @brief Handles input from the terminal.
 *
 * This function checks if there is any data input from the terminal, reads it, and sends it to the input queue.
 *
 * @retval TX_SUCCESS Always returns success.
 **********************************************************************************************************************/
UINT terminal_input_handle(void)
{
    UINT read_len = RESET_VALUE;
    CHAR read_buff [TERM_BUFFER_SIZE];

    if(TERM_HAS_DATA())
    {
        read_len = TERM_READ(read_buff, sizeof(read_buff));
        if (read_len)
        {
            term_send_input_queue(TERMINAL_INPUT_MESSAGE, read_buff, read_len);
        }
    }

    return TX_SUCCESS;
}

/***********************************************************************************************************************
 *  Function Name: conv_ul64_to_str
 *  Description  : This function converts a ULONG 64 value to a string.
 *  Arguments    : value   value is of type ULONG 64 bits
 *                 len     length of input buffer
 *                 p_buf   pointer to string buffer
 *  Return Value : None
 **********************************************************************************************************************/
static void conv_ul64_to_str(ULONG64 value, CHAR * p_buf, UINT len)
{
    UINT units     = RESET_VALUE;
    UINT thousands = RESET_VALUE;
    UINT millions  = RESET_VALUE;
    UINT billions  = RESET_VALUE;

    /* Clean output buffer */
    memset(p_buf, NULL_CHAR, len);

    /* Check the value range */
    if (BILLION <= value)
    {
        billions = (UINT)(value / BILLION);
        millions = (UINT)((value / MILLION) % THOUSAND);
        thousands = (UINT)((value / THOUSAND) % THOUSAND);
        units = (UINT)(value % THOUSAND);
        snprintf (p_buf, len, "%u,%.3u,%.3u,%.3u", billions, millions, thousands, units);
    }
    else if (MILLION <= value)
    {
        millions = (UINT)(value / MILLION);
        thousands = (UINT)((value / THOUSAND) % THOUSAND);
        units = (UINT)(value % THOUSAND);
        snprintf (p_buf, len, "%u,%.3u,%.3u", millions, thousands, units);
    }
    else if (THOUSAND <= value)
    {
        thousands = (UINT)(value / THOUSAND);
        units = (UINT)(value % THOUSAND);
        snprintf (p_buf, len, "%u,%.3u", thousands, units);
    }
    else
    {
        units = (UINT)value;
        snprintf (p_buf, len, "%u", units);
    }
}


