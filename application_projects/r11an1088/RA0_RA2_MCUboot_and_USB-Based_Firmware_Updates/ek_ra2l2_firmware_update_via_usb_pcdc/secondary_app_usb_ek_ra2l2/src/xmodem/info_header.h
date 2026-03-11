/***********************************************************************************************************************
 * File Name    : info_header.h
 * Description  : Contains macros for image slots, enum and function prototypes
 ***********************************************************************************************************************/

/***********************************************************************************************************************
* Copyright (c) 2020 - 2025 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
************************************************************************************************************************/

#ifndef HEADER_H_
#define HEADER_H_

#define PRIMARY_IMAGE_START_ADDRESS         0x3800
#define PRIMARY_IMAGE_END_ADDRESS           0x117FF
#define SECONDARY_IMAGE_START_ADDRESS       0x11800
#define SECONDARY_IMAGE_END_ADDRESS         0x1F7FF
#define FLASH_BLOCK_SIZE                    (2 * 1024)
#define SECONDARY_IMAGE_NUM_BLOCKS          ((SECONDARY_IMAGE_END_ADDRESS - SECONDARY_IMAGE_START_ADDRESS + 1U) / FLASH_BLOCK_SIZE)

typedef enum
{
    DISABLE = 0,
    RE_ENABLE
} enable_disable_t;

void display_image_slot_info(void);

void interrupt_control(enable_disable_t enable_disable);

#endif /* HEADER_H_ */
