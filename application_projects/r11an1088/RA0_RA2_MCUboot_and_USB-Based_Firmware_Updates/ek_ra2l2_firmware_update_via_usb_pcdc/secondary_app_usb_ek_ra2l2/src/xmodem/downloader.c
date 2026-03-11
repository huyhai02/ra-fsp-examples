/***********************************************************************************************************************
 * File Name    : downloader.c
 * Description  : Contains menu item implementations
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/


#include "downloader.h"
#include "info_header.h"
#include "xmodem.h"
#include "comms.h"
#include "stdio.h"
#include "menu.h"
#include "hal_data.h"

static bool is_usb_connected = false;

void downloader_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open the comms driver */
    err = comms_open();
    if (FSP_SUCCESS != err)
    {
        /* Stop as comms open failure */
        while (1)
        {
          ;
        }
    }

    /* Open the flash driver */
    err = R_FLASH_LP_Open(&g_flash0_ctrl, &g_flash0_cfg);
    if (FSP_SUCCESS != err)
    {
        /* Stop as Internal flash open failure */
        while (1)
        {
          ;
        }
    }

    /* Initializes the XMODEM protocol */
    xmodem_config_t xmodem_cfg = {
      .xmodem_comm_read = comms_read,
      .xmodem_comm_send = comms_send
    };
    xmodem_init(&xmodem_cfg);
}

fsp_err_t downloader_entry_process(void)
{
    fsp_err_t ret = FSP_SUCCESS;

    if (comms_wait_connection() == FSP_SUCCESS)
    {
        if (!is_usb_connected)
        {
            is_usb_connected = true;
            menu_display();
        }

        app_menu_info_t app_menu_info = menu_process();

        if (app_menu_info.data != '2')
        {
            menu_display();
            return ret;
        }
    }
    else
    {
        is_usb_connected = false;
    }

    return ret;
}

void display_image_slot_info(void)
{
    uint8_t str[100];

    struct image_version {
        uint8_t iv_major;
        uint8_t iv_minor;
        uint16_t iv_revision;
        uint32_t iv_build_num;
    };

    struct image_header {
        uint32_t ih_magic;
        uint32_t ih_load_addr;
        uint16_t ih_hdr_size;            /* Size of image header (bytes). */
        uint16_t ih_protect_tlv_size;    /* Size of protected TLV area (bytes). */
        uint32_t ih_img_size;            /* Does not include header. */
        uint32_t ih_flags;               /* IMAGE_F_[...]. */
        struct image_version ih_ver;
        uint32_t _pad1;
    };

    struct image_header * p_img_header;

    /* Primary NS Image Slot */
    p_img_header = (struct image_header *)PRIMARY_IMAGE_START_ADDRESS;
    snprintf((char *)str, sizeof(str), "**********************\r\n");
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "* Primary Image Slot *\r\n");
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "**********************\r\n");
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Image version: \t\t%02d.%02d (Rev: %d, Build: %d)\r\n", p_img_header->ih_ver.iv_major,
                                                                                             p_img_header->ih_ver.iv_minor,
                                                                                             p_img_header->ih_ver.iv_revision,
                                                                                             p_img_header->ih_ver.iv_build_num);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Load address: \t\t0x%08X\r\n", (unsigned int)PRIMARY_IMAGE_START_ADDRESS);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Header size: \t\t0x%04X (%d bytes)\r\n", p_img_header->ih_hdr_size, p_img_header->ih_hdr_size);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Protected TLV size: \t0x%04X (%d bytes)\r\n", p_img_header->ih_protect_tlv_size, p_img_header->ih_protect_tlv_size);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Image size: \t\t0x%08X (%d bytes)\r\n", (unsigned int)p_img_header->ih_img_size, p_img_header->ih_img_size);
    comms_send(str, strlen((char *)str));

    /* Secondary NS Image Slot */
    p_img_header = (struct image_header *)SECONDARY_IMAGE_START_ADDRESS;
    snprintf((char *)str, sizeof(str), "\r\n***************************\r\n");
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "* Secondary Image Slot *\r\n");
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "************************\r\n");
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Image version: \t\t%02d.%02d (Rev: %d, Build: %d)\r\n", p_img_header->ih_ver.iv_major,
                                                                                             p_img_header->ih_ver.iv_minor,
                                                                                             p_img_header->ih_ver.iv_revision,
                                                                                             p_img_header->ih_ver.iv_build_num);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Load address: \t\t0x%08X\r\n", (unsigned int)SECONDARY_IMAGE_START_ADDRESS);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Header size: \t\t0x%04X (%d bytes)\r\n", p_img_header->ih_hdr_size, p_img_header->ih_hdr_size);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Protected TLV size: \t0x%04X (%d bytes)\r\n", p_img_header->ih_protect_tlv_size, p_img_header->ih_protect_tlv_size);
    comms_send(str, strlen((char *)str));
    snprintf((char *)str, sizeof(str), "Image size: \t\t0x%08X (%d bytes)\r\n", (unsigned int)p_img_header->ih_img_size, p_img_header->ih_img_size);
    comms_send(str, strlen((char *)str));
}
