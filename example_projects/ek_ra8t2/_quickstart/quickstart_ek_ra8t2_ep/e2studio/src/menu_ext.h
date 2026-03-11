/*
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/***********************************************************************************************************************
 * File Name    : menu_ext.h
 * Description  : External memory demo interface file.
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 19.02.2026 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "r_typedefs.h"

#ifndef OSPI_FUNCTIONS_FLASH_TEST_H_
#define OSPI_FUNCTIONS_FLASH_TEST_H_


/* These commands and registers are for the ISSI IS25LX512M and may not match other targets */
#define READ_SFDP_COMMAND                              (0x5AU)
#define READ_SFDP_COMMAND_OPI                          (0x5AU)
#define READ_REGISTER_COMMAND                          (0x71U)
#define READ_REGISTER_COMMAND_OPI                      (0x71U)

#define WRITE_REGISTER_COMMAND                         (0x81U)
#define WRITE_REGISTER_COMMAND_OPI                     (0x81U)
#define WRITE_VOLATILE_CONFIGURATION_REGISTER          (0x81u)
#define WRITE_VOLATILE_CONFIGURATION_REGISTER_OPI      (0x8181u)
#define WRITE_ENABLE_COMMAND                           (0x06U)
#define WRITE_ENABLE_COMMAND_OPI                       (0x0606U)
#define READ_STATUS_COMMAND                            (0x05U)
#define READ_STATUS_COMMAND_OPI                        (0x0505U)
#define READ_FLAG_STATUS_COMMAND_OPI                   (0x7070U)
#define READ_VOLATILE_CONFIGURATION_REGISTER           (0x85u)
#define READ_VOLATILE_CONFIGURATION_REGISTER_OPI       (0x8585u)

#define CFR2V_REGISTER_ADDRESS                      (0x800003U)
#define CFR3V_REGISTER_ADDRESS                      (0x800004U)
#define DEFDOPI_REGISTER_ADDRESS                    (0x40000000U)
#define READ_CONFIGURATION_REGISTER_2               (0x00000000U)

#define CFR2V_MEMLAT_POS                               (0U)
#define CFR2V_MEMLAT_MSK                               (0x0FU)

#define CFR3V_VRGLAT_POS                               (6U)
#define CFR3V_VRGLAT_MSK                               (0xC0U)
#define CFR3V_UNHYSA_POS                               (3U)
#define CFR3V_UNHYSA_MSK                               (0x08U)

#define CFR5V_SDRDDR_POS                               (1U)
#define CFR5V_SDRDDR_MSK                               (0x02U)
#define CFR5V_OPIIT_POS                                (0U)
#define DOPI_MSK                                       (0x10U)
#define SPI_MSK                                        (0x00U)

#define OSPI_MODE_DOPI                                 (DOPI_MSK)
#define OSPI_MODE_SPI                                  (SPI_MSK)

#define REG_LATENCY_CODE_SPI   (0x00U)
#define REG_LATENCY_CODE_OPI   (0x03U)

#define MEM_LATENCY_CODE_SPI   (0x08U)
#define MEM_LATENCY_CODE_OPI   (0x0AU)

#define REG_DUMMY_CYCLES_SPI           (0U)
#define READ_STATUS_DUMMY_CYCLES_OPI   (16U)
#define REG_DUMMY_CYCLES_OPI           (16U)


#define WRITE_ENABLE_MASK      (0x02U)

#define READ_SFDP_DUMMY_CYCLES (8U)
#define SFDP_SIGNATURE         (0x50444653U)

/* Cast: 48-bit value requires uint64_t to avoid truncation */
#define DEVICE_ID_OPI          ((uint64_t)(0x3A3A8686C2C2))

#define OSPI_TEST_DATA_LENGTH  (0x40U)
#define OSPI_SECTOR_SIZE       (4096U)
#define OSPI_BLOCK_SIZE        (262144U)

#define OSPI_RESET_PIN         (BSP_IO_PORT_12_PIN_07)
#define OSPI_RESET_DELAY       (50U)


#define EXTENDED_SPI_MODE                            (0xFFu)
#define EXTENDED_SPI_MODE_NO_DQS                     (0xDFu)
#define OCTAL_DDR_MODE                               (0xE7u)
#define OCTAL_DDR_MODE_NO_DQS                        (0xC7u)

/**********************************************************************************************************************
 * Function Name: ext_display_menu
 * Description  : .
 * Return Value : .
 *********************************************************************************************************************/
test_fn ext_display_menu (void);

#endif /* OSPI_TEST_H_ */
