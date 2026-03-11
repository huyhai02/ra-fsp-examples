/***********************************************************************************************************************
 * File Name    : comms.c
 * Description  : Contains downloader communication related functions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/
#include "comms.h"
#include "app_common.h"

extern usb_instance_ctrl_t g_basic0_ctrl;

static volatile bool g_tx_flag  = false;
static volatile bool g_rx_flag  = false;
static volatile bool g_err_flag = false;
static volatile bool g_operation_completed_flag = false;

fsp_err_t comms_open(void)
{
    fsp_err_t err = FSP_SUCCESS;

    err = RM_COMMS_USB_PCDC_Open(&g_comms_usb_pcdc0_ctrl, &g_comms_usb_pcdc0_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return err;
}

fsp_err_t comms_wait_connection(void)
{

    return (g_comms_usb_pcdc0_ctrl.ctrl_line_state.bdtr == 1) ? FSP_SUCCESS : FSP_ERR_USB_FAILED;
}

fsp_err_t comms_send(uint8_t *p_src, uint32_t len)
{
    fsp_err_t err =  FSP_SUCCESS;

    g_tx_flag  = false;
    g_err_flag = false;
    err = RM_COMMS_USB_PCDC_Write(&g_comms_usb_pcdc0_ctrl, p_src, (uint32_t)len);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Wait for the USB Write to complete */
    while (!g_tx_flag)
    {
        /* Wait for callback */
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function read the USB data with timeout control
 * @param[IN]   p_dest                  destination data buffer
 * @param[IN]   len                     length of the data
 * @param[IN]   timeout_milliseconds    timeout in milliseconds
 * @retval      FSP_SUCCESS             Upon successful FLash_HP data flash operations.
 * @retval      Any Other Error code    Upon unsuccessful Flash_HP data flash operations.
 **********************************************************************************************************************/
fsp_err_t comms_read(uint8_t *p_dest, uint32_t *len, uint32_t timeout_milliseconds)
{
    fsp_err_t err = FSP_SUCCESS;

    uint32_t tick_timeout = sys_tick_get();

    g_rx_flag = false;
    err = RM_COMMS_USB_PCDC_Read(&g_comms_usb_pcdc0_ctrl, p_dest, *len);
    if (FSP_SUCCESS != err)
    {
        return FSP_ERR_USB_FAILED;
    }

    while (!g_rx_flag)
    {
        if (sys_tick_get() - tick_timeout >= timeout_milliseconds)
        {
            g_comms_usb_pcdc0_ctrl.p_extend->p_usb->p_api->stop(&g_basic0_ctrl, USB_TRANSFER_READ, USB_CLASS_PCDC);
            return FSP_ERR_TIMEOUT;
        }
    }

    return err;
}

/***********************************************************************************************************************
 * @brief  Callback function for USB PCDC communication events.
 *         Handles transmission, reception, and error events.
 *
 * @param[in] p_args    Pointer to the callback argument structure containing the event type.
 **********************************************************************************************************************/
void rm_comms_usb_pcdc_callback (rm_comms_callback_args_t * p_args)
{
    if (NULL == p_args)
    {
        return;
    }

    switch (p_args->event)
    {
        case RM_COMMS_EVENT_OPERATION_COMPLETE:
            g_operation_completed_flag = true;
            break;

        case RM_COMMS_EVENT_TX_OPERATION_COMPLETE:
            g_tx_flag = true;
            break;

        case RM_COMMS_EVENT_RX_OPERATION_COMPLETE:
            g_rx_flag = true;
            break;

        case RM_COMMS_EVENT_ERROR:
        default:
            g_err_flag = true;
            break;
    }
}
