/***********************************************************************************************************************
 * File Name    : xmodem.h
 * Description  : Contains XModem protocol macro definitions and function definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/

#include "xmodem.h"
#include "comms.h"
#include "info_header.h"

#define MAX_RETRIES             (10)

#define XMODEM_PACKET_SIZE      (132) /* <SOH><blk #><255-blk #><--128 data bytes--><checksum> */
#define XMODEM_DATA_SIZE        (128)
#define XMODEM_TIMEOUT_MS       (1000)

#define HEADER_INDEX            (0)
#define CHECKSUM_INDEX          (131)
#define DATA_INDEX              (3)

static uint8_t rx_byte[XMODEM_PACKET_SIZE] BSP_ALIGN_VARIABLE(4);
static uint32_t rx_len = 0;

static xmodem_config_t g_xmodem_cfg;

/***********************************************************************************************************************
* Function Name: xmodem_send_cmd
* Description  : Sends a single XMODEM control command (such as NAK, ACK, or CAN) to the transmitter
*                over the communication interface.
* Arguments    : cmd -  The XMODEM command byte to be sent.
* Return Value : xmodem_status_t:
*                   XMODEM_SUCCESS   - Command sent successfully.
*                   XMODEM_ERROR     - Failed to send command.
***********************************************************************************************************************/
static xmodem_status_t xmodem_send_cmd(uint8_t cmd)
{
    fsp_err_t ret = FSP_SUCCESS;
    ret = g_xmodem_cfg.xmodem_comm_send(&cmd, 1);
    if (ret != FSP_SUCCESS)
    {
        return XMODEM_ERROR;
    }

    return XMODEM_SUCCESS;
}

/***********************************************************************************************************************
* Function Name: xmodem_read_data
* Description  : Reads a block of data from the communication interface with a specified timeout.
* Arguments    : data           - Pointer to the buffer where received data will be stored.
*                len            - Pointer to a variable that holds the expected length of data to read.
*                                 On return, it contains the actual number of bytes received.
*                timeout_ms     - Timeout duration for the read operation, in milliseconds.
* Return Value : xmodem_status_t:
*                   XMODEM_SUCCESS        - Data was successfully read.
*                   XMODEM_TIMEOUT_ERROR  - Timeout occurred before data was received.
*                   XMODEM_ERROR          - A communication or hardware error occurred.
***********************************************************************************************************************/
static xmodem_status_t xmodem_read_data(uint8_t *data, uint32_t *len, uint32_t timeout_ms)
{
    fsp_err_t err = FSP_SUCCESS;

    err = g_xmodem_cfg.xmodem_comm_read(data, len, timeout_ms);
    if (err == FSP_SUCCESS)
    {
        return XMODEM_SUCCESS;
    }
    else if (err == FSP_ERR_TIMEOUT)
    {
        return XMODEM_TIMEOUT_ERROR;
    }
    else
    {
        return XMODEM_ERROR;
    }

    return XMODEM_SUCCESS;
}

/***********************************************************************************************************************
* Function Name: xmodem_check_package_num
* Description  : Validates the packet number and its complement in a received XMODEM packet.
*                This function ensures that the packet sequence is correct and that the packet
*                is neither corrupted nor duplicated.
* Arguments    : packet_data            - Pointer to the received XMODEM packet buffer.
*                expected_packet_num    - Expected packet number in the current XMODEM transfer sequence.
* Return Value : xmodem_status_t:
*                   XMODEM_SUCCESS          - Packet number and complement are valid.
*                   XMODEM_DUPLICATE_PACKET - Received packet is a duplicate of the previous one.
*                   XMODEM_SEQUENCE_ERROR   - Packet header, number, or complement is invalid.
***********************************************************************************************************************/
static xmodem_status_t xmodem_check_package_num(uint8_t *packet_data, uint8_t expected_packet_num)
{
    uint8_t pkt_num = packet_data[1];
    uint8_t pkt_num_complement = packet_data[2];

    if (packet_data[HEADER_INDEX] != SOH)
    {
        return XMODEM_SEQUENCE_ERROR;
    }

    if ((uint8_t)(pkt_num + pkt_num_complement) != 0xFF)
    {
        return XMODEM_SEQUENCE_ERROR;
    }

    if (pkt_num == (uint8_t)(expected_packet_num - 1))
    {
        return XMODEM_DUPLICATE_PACKET;
    }

    if ((pkt_num != (uint8_t)expected_packet_num))
    {
        return XMODEM_SEQUENCE_ERROR;
    }

    return XMODEM_SUCCESS;
}

/***********************************************************************************************************************
* Function Name: xmodem_checksum
* Description  : Verifies the checksum of a received XMODEM packet to ensure data integrity.
*                The checksum is calculated by summing all data bytes in the packet and comparing
*                the result with the transmitted checksum byte.
* Arguments    : packet_data    - Pointer to the received XMODEM packet buffer.
* Return Value : xmodem_status_t:
*                   XMODEM_SUCCESS        - Packet checksum is valid.
*                   XMODEM_CHECKSUM_ERROR - Packet checksum mismatch detected.
***********************************************************************************************************************/
static xmodem_status_t xmodem_checksum(uint8_t *packet_data)
{
    uint8_t checksum = 0;
    xmodem_status_t ret = XMODEM_SUCCESS;

    for (int index = 0; index < XMODEM_DATA_SIZE; index++)
    {
        checksum = checksum + packet_data[index + DATA_INDEX];
    }

    if (checksum != packet_data[CHECKSUM_INDEX])
    {
        ret = XMODEM_CHECKSUM_ERROR;
    }
    else
    {
        ret = XMODEM_SUCCESS;
    }

    return ret;
}

/***********************************************************************************************************************
* Function Name: xmodem_receive_packet
* Description  : Receives a single XMODEM packet from the communication interface, validates its integrity,
*                and handles retry logic for transmission errors or timeouts.
*                The function waits for a packet, checks sequence number, verifies checksum,
*                and acknowledges the sender accordingly.
* Arguments    : packet_data          - Pointer to the buffer for storing the received packet.
*                expected_packet_num  - Expected sequence number of the next packet to be received.
* Return Value : xmodem_status_t:
*                   XMODEM_PACKET_RECEIVED  - Valid packet received successfully.
*                   XMODEM_DUPLICATE_PACKET - Duplicate packet detected (previous ACK missed).
*                   XMODEM_EOT_RECEIVED     - End of transmission received (EOT).
*                   XMODEM_TIMEOUT_ERROR    - Transmission timed out after maximum retries.
*                   XMODEM_SEQUENCE_ERROR   - Invalid or out-of-sequence packet received.
*                   XMODEM_CHECKSUM_ERROR   - Data integrity check failed.
*                   XMODEM_ERROR            - General communication or unexpected error.
***********************************************************************************************************************/
static xmodem_status_t xmodem_receive_packet(uint8_t *packet_data, uint8_t expected_packet_num)
{
    xmodem_status_t ret = XMODEM_SUCCESS;

    uint8_t retries = MAX_RETRIES;

    bool is_start_transfer = false;

    while (retries > 0)
    {
        rx_len = XMODEM_PACKET_SIZE;
        memset(packet_data, 0, XMODEM_PACKET_SIZE);

        ret = xmodem_read_data(packet_data, &rx_len, XMODEM_TIMEOUT_MS);
        if (ret == XMODEM_TIMEOUT_ERROR)
        {
            if (!is_start_transfer)
            {
                xmodem_send_cmd(NAK);
                continue;
            }
            else
            {
                if (--retries == 0)
                {
                    return XMODEM_TIMEOUT_ERROR;
                }
                xmodem_send_cmd(NAK);
                continue;
            }
        }
        if (ret == XMODEM_SUCCESS)
        {
            is_start_transfer = true;

            if (packet_data[0] == EOT)
            {
                xmodem_send_cmd(ACK);
                return XMODEM_EOT_RECEIVED;
            }

            ret = xmodem_check_package_num(packet_data, expected_packet_num);
            if (ret == XMODEM_DUPLICATE_PACKET)
            {
                xmodem_send_cmd(ACK);
                continue;
            }
            else if (ret == XMODEM_SEQUENCE_ERROR)
            {
                if (--retries == 0)
                {
                   return XMODEM_SEQUENCE_ERROR;
                }
                xmodem_send_cmd(NAK);
                continue;
            }

            if (xmodem_checksum(packet_data) != XMODEM_SUCCESS)
            {
                if (--retries == 0)
                {
                   return XMODEM_CHECKSUM_ERROR;
                }
                xmodem_send_cmd(NAK);
                continue;
            }

            /* Packet received successfully */
            xmodem_send_cmd(ACK);

            return XMODEM_PACKET_RECEIVED;
        }
        else
        {
            return XMODEM_ERROR;
        }
    }

    return ret;
}

/***********************************************************************************************************************
* Function Name: xmodem_program_flash
* Description  : Writes a block of received XMODEM data to the specified flash memory address.
*                This function is typically called after a valid XMODEM packet is received and verified.
*                If the flash write operation fails, the function notifies the sender by transmitting a CAN command.
* Arguments    : packet_data    - Pointer to the data buffer containing the received XMODEM payload.
*                flash_address  - Destination flash memory address where the data will be written.
* Return Value : xmodem_status_t:
*                   XMODEM_SUCCESS    - Flash programming completed successfully.
*                   XMODEM_PROG_ERROR - Flash write operation failed.
***********************************************************************************************************************/
static xmodem_status_t xmodem_program_flash(uint8_t *packet_data, uint32_t flash_address)
{
    fsp_err_t err = FSP_SUCCESS;

    err = R_FLASH_LP_Write(&g_flash0_ctrl, (uint32_t)packet_data, flash_address, XMODEM_DATA_SIZE);
    if (err != FSP_SUCCESS)
    {
        xmodem_send_cmd(CAN);
        return XMODEM_PROG_ERROR;
    }

    return XMODEM_SUCCESS;
}

/***********************************************************************************************************************
* Function Name: xmodem_download_and_program_flash
* Description  : Handles the complete XMODEM download and flash programming process.
*                This function coordinates receiving XMODEM packets, verifying their integrity,
*                and writing valid data blocks to flash memory sequentially.
*                The transfer ends when the EOT (End of Transmission) character is received.
*
* Arguments    : flash_address  - Starting address in flash memory where the received data will be written.
*
* Return Value : xmodem_status_t:
*                   XMODEM_SUCCESS          - Operation completed successfully.
*                   XMODEM_COMPLETED        - Transfer ended normally after EOT received.
*                   XMODEM_TIMEOUT_ERROR    - Timeout occurred while waiting for data.
*                   XMODEM_PROG_ERROR       - Flash programming failed.
*                   XMODEM_SEQUENCE_ERROR   - Packet sequence mismatch detected.
*                   XMODEM_CHECKSUM_ERROR   - Checksum mismatch detected.
*                   XMODEM_ERROR            - General communication or protocol error.
*
* Note          :
*   - This function disables interrupts during flash write to ensure data integrity.
*   - The function sends an initial NAK to prompt the sender to start transmission.
*   - The flash is written in 128-byte blocks as per standard XMODEM packet size.
***********************************************************************************************************************/
xmodem_status_t xmodem_download_and_program_flash(uint32_t flash_address)
{
    xmodem_status_t ret = XMODEM_SUCCESS;

    uint8_t packet_num = 1;

    /* Kick off the XMODEM transfer */
    xmodem_send_cmd(NAK);

    uint32_t address = flash_address;

    while (1)
    {
        /* Receive the packet */
        ret = xmodem_receive_packet(&rx_byte[0], packet_num);
        if (ret == XMODEM_EOT_RECEIVED)
        {
            return XMODEM_COMPLETED;
        }
        else if (ret == XMODEM_PACKET_RECEIVED)
        {
            interrupt_control(DISABLE);
            ret = xmodem_program_flash(&rx_byte[DATA_INDEX], address);
            interrupt_control(RE_ENABLE);

            address += XMODEM_DATA_SIZE;
            packet_num++;
        }
        else
        {
            return ret;
        }
    }

    return ret;
}

xmodem_status_t xmodem_init(xmodem_config_t *xmodem_cfg)
{
    if (xmodem_cfg == NULL || xmodem_cfg->xmodem_comm_read == NULL || xmodem_cfg->xmodem_comm_send == NULL)
    {
        return XMODEM_ERROR;
    }

    g_xmodem_cfg = *xmodem_cfg;

   return XMODEM_SUCCESS;
}
