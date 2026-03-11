/*
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/
/**********************************************************************************************************************
 * File Name    : menu_ext.c
 * Version      : 1.0
 * Description  : External memory demo implementation.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 19.02.2026 1.00     First Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include "menu_ext.h"
#include "hal_data.h"
#include "board_hw_cfg.h"
#include "console.h"
#include "jlink_console.h"

#include "ospi_b_ep.h"

#define FLASH_DEVICE_SIZE_BYTES     (1024 * 1024 * 64)          /* 64 MB flash device */
#define BLOCK_LIMIT_KB              (64)                        /* 64 kB maximum block size for the test */
#define MAX_BLOCK_SIZE              (BLOCK_LIMIT_KB * 1024)     /* maximum block size in bytes*/
#define GPT_CLOCK_MHZ               (250)                       /* the GPT clock speed in MHz */
#define WRITE_BLOCK_SIZE            (64)                        /* the size of a block when writing to the flash*/
#define TEST_BLOCKS                 (64)


#define WAITING_MESSAGE             ("Waiting Results")
#undef MODULE_NAME
#define MODULE_NAME                 "\r\n%d. OCTO-SPI SPEED DEMONSTRATION\r\n"

#define SUB_OPTIONS                 "\r\nCompares the write and read times to and from external Octo-SPI" \
                                    "\r\nflash memory\r\n" \
                                    "\r\n> Enter the text block size " \
                                    "\r\n(in multiples of 2 KB, max 64 KB) and press tab to continue : "

/* The RA8T2 OSPI flash is ISSI IS25LX512M-JHLE */
/* The device is 512M bit (64M Byte) */

static fsp_err_t ospi_flash_open (void);
static fsp_err_t write_test_opi (uint32_t num_blocks);
static fsp_err_t read_test_opi (uint32_t num_blocks);

static uint32_t validate_user_input (char_t * p_input);
static uint32_t value_validation (uint32_t value);

static void ospi_performance_test (uint32_t data_size,
        uint32_t * ospi_performance_write_result, uint32_t * ospi_performance_read_result);

static void reset_ospi_device (void);
static void write_en (bool is_dopi);
static void transition_to_dopi (void);
static void wait_for_write (void);
static void ospi_b_change_omclk (bsp_clocks_octaclk_div_t divider);
static void set_clock_speed_66mhz (void);
static void set_clock_speed_133mhz (void);
static void ospi_read_write_print (uint32_t block_size_actual);
static void ospi_menu_print (void);

static bool_t check_input_value (uint32_t value);

/* Cast to uint8_t *: 0x78000000 is the fixed memory-mapped base address of the OSPI CS1
 * peripheral region as defined in the hardware memory map. This address is guaranteed
 * to be a valid aligned memory-mapped address for the OSPI flash device */
static uint8_t * const sp_gp_ospi_cs1 = (uint8_t *) 0x78000000;

static char_t   s_print_buffer[BUFFER_LINE_LENGTH];
static uint8_t  s_read_block[MAX_BLOCK_SIZE];
static uint8_t  s_g_test_data[OSPI_TEST_DATA_LENGTH];
static uint32_t s_test_remain           = WRITE_BLOCK_SIZE;
static uint32_t s_test_current_block    = 0;
static uint32_t s_test_current_sub      = 0;
static size_t   s_data_size             = TEST_BLOCKS * WRITE_BLOCK_SIZE;
static uint32_t s_g_counter             = 0;
static char_t   s_block_sz_str[INPUT_BUFFER];

/**********************************************************************************************************************
 * Function Name: ospi_b_change_omclk
 * Description  : Changes the OCTACLK divider; assumes PLL2 is being used as the source for OCTACLK
 *              : divider New clock divider to switch to
 * Return Value : None
 *********************************************************************************************************************/
static void ospi_b_change_omclk(bsp_clocks_octaclk_div_t divider)
{
    /* ensure clock source (PLL2 in this example) is running before changing the OCTACLK frequency */
    /* PLL2P should be set up to give a clock speed of 266.67 MHz */
    bsp_octaclk_settings_t octaclk_settings;

    octaclk_settings.source_clock = BSP_CLOCKS_SOURCE_CLOCK_PLL2P;
    octaclk_settings.divider      = divider;
    R_BSP_OctaclkUpdate(&octaclk_settings);
}
/**********************************************************************************************************************
 End of function ospi_b_change_omclk
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: set_clock_speed_66mhz
 * Description  : set the OSPI flash bit clock to 66 MHz (133 MHz OCTACLK)
 * Return Value : None
 *********************************************************************************************************************/
static void set_clock_speed_66mhz(void)
{
    /* change the OCTACLK to 133 MHz (to give a device clock of 66 MHz) */
    /* OCTACLK source is configured with 266 MHz input */
    ospi_b_change_omclk(BSP_CLOCKS_OCTACLK_DIV_2);
}
/**********************************************************************************************************************
 End of function set_clock_speed_66mhz
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: set_clock_speed_133mhz
 * Description  : set the OSPI flash bit clock to 133 MHz (266 MHz OCTACLK)
 * Return Value : None
 *********************************************************************************************************************/
static void set_clock_speed_133mhz(void)
{
    /* change the OCTACLK to 266 MHz (to give a device clock of 133 MHz) */
    /* OCTACLK source is configured with 266 MHz input */
    ospi_b_change_omclk(BSP_CLOCKS_OCTACLK_DIV_1);
}
/**********************************************************************************************************************
 End of function set_clock_speed_133mhz
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: ext_display_menu
 * Description  : Implements the external flash memory menu
 * Return Value : 0
 *********************************************************************************************************************/
test_fn ext_display_menu(void)
{
    bool_t   valid_block_size  = false;
    int32_t  c                 = -1;
    int32_t  block_sz_ndx      = 0;
    int32_t  block_sz_limit    = (INPUT_BUFFER-2);    /* Allowing for TAB and End of Message */
    uint32_t block_size_actual = 0;
    uint32_t value             = 0;
    ospi_menu_print();

    /* Keep trying to read a valid text block size
     * complete the loop in one of two ways:
     * [1] Valid block size is entered (2K boundary range 2-64K) followed by TAB
     * [2] Space Bar is pressed at any stage
     */
while (!valid_block_size)
    {
        /* reset input state */
        c            = -1;
        block_sz_ndx = 0;
        memset (&s_block_sz_str, 0, INPUT_BUFFER);
        while ((CONNECTION_ABORT_CRTL != c))
        {
            c = input_from_console();
            if ((block_sz_ndx < block_sz_limit) && (BACK_SPACE != c) && (BACK_SPACE_PUTTY != c) && (CARRAGE_RETURN != c)
                    && (MENU_ENTER_RESPONSE_CRTL != c))
            {
                /* cast to req type */
                s_block_sz_str[block_sz_ndx] = (char_t) c;
                block_sz_ndx++;
            }
            else if (((BACK_SPACE == c) || (BACK_SPACE_PUTTY == c)) && (block_sz_ndx > 0))
            {
                s_block_sz_str[block_sz_ndx - 1] = '\0';
                block_sz_ndx--;
                print_to_console("\b \b");
            }
            else if ((0 == block_sz_ndx) || (CARRAGE_RETURN == c))
            {
                /* No action required: either buffer is empty or carriage return
                             * received with no new data to process */
                ;
            }
            else
            {
                /* maximum block size exceeded (4 digits / characters entered) */
                s_block_sz_str[block_sz_ndx] = MENU_ENTER_RESPONSE_CRTL;
                c                            = MENU_ENTER_RESPONSE_CRTL;
                break;
            }
            if (MENU_EXIT_CRTL == c)
            {
                /* abort the test */
                valid_block_size  = true;
                block_size_actual = 0;
                break;
            }
            if ((MENU_ENTER_RESPONSE_CRTL == c) || (CARRAGE_RETURN == c))
            {
                break;
            }
            if ((CARRAGE_RETURN != c) && (BACK_SPACE != c))
            {
                /* Cast to char_t: value of c is a valid printable ASCII character
                 * as control characters (CR, backspace) are excluded by the
                 * preceding conditional check, and the value fits within char_t range */
                sprintf (s_print_buffer, "%c", (char_t)c);
                print_to_console(s_print_buffer);
            }
        }

        /* If the input was terminated with a TAB then attempt to process it */
        if ((MENU_ENTER_RESPONSE_CRTL == c) || (CARRAGE_RETURN == c))
        {
            value = validate_user_input (&s_block_sz_str[0]);
        }
        vTaskDelay (10);

        /* Exit the while loop when received space bar pressed */
        if (MENU_EXIT_CRTL == c)
        {
            break;
        }
        value            = value_validation (value);
        valid_block_size = check_input_value (value);
    }
    block_size_actual = value;
    if ((MENU_ENTER_RESPONSE_CRTL == c) && (0 != block_size_actual))
    {
        ospi_read_write_print(value);
    }
    while ((CONNECTION_ABORT_CRTL != c))
    {
        if ((MENU_EXIT_CRTL == c) || (0x00 == c))
        {
            break;
        }
        c = input_from_console ();
    }
    return (0);
}
/**********************************************************************************************************************
 End of function ext_display_menu
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: validate_user_input
 * Description  : parse the user input and convert to integer value
 * Argument     : p_input
 * Return Value : Integer value entered, or 0 if the menu exit character was entered
 *********************************************************************************************************************/
static uint32_t validate_user_input(char_t *p_input)
{
    uint32_t result = INVALID_CHARACTER;
    uint32_t value  = 0;
    uint32_t t      = 0;
    uint32_t c      = 0;

    while (true)
    {
        /* Cast to req type */
        c = (uint32_t) (*p_input);

        p_input++;

        if ((c >= '0') && (c <= '9'))
        {
            /* Cast as compiler will interpret result as int */
            value = (uint32_t) (value * 10);

            /* Cast to req type */
            t = (uint32_t) (c - '0');

            /* Cast as compiler will interpret result as int */
            value = (uint32_t) (value + t);
        }
        else
        {
            if (MENU_ENTER_RESPONSE_CRTL == c)
            {
                result = value;
            }

            if (MENU_EXIT_CRTL == c)
            {
                result = 0;
            }

            break;
        }
    }

    return (result);
}
/**********************************************************************************************************************
 End of function validate_user_input
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: ospi_performance_test
 * Description  : write data to the OSPI flash, then read it. Use a timer to measure the durations
 * Argument     : data_size -  the data size to write and read
 * Argument     : ospi_performance_write_result - time taken for the write
 * Argument     : ospi_performance_read_result - time taken for the read
 * Return Value : None
 *********************************************************************************************************************/
static void ospi_performance_test(uint32_t data_size,
        uint32_t *ospi_performance_write_result, uint32_t *ospi_performance_read_result)
{
    fsp_err_t err;

    uint32_t  erase_size = OSPI_SECTOR_SIZE;
    uint32_t  erase_blocks;

    set_clock_speed_66mhz();

    err = ospi_b_init();

    err = ospi_flash_open();

    /* calculate the number of blocks to erase */
    erase_blocks = data_size / erase_size;

    if ((erase_blocks * erase_size) < data_size)
    {
        erase_blocks++;
    }

    /* erase the flash data area before we write to it */
    for (uint32_t block = 0; block < erase_blocks; block++)
    {
        /* Cast to uint8_t *: sp_gp_ospi_cs1 is the base address of the OSPI memory region
          * and the offset (erase_size * block) calculates the start address of each erase block.
          * The resulting address is guaranteed to be within the valid OSPI CS1 memory-mapped
          * region as erase_blocks is derived from data_size which does not exceed the
          * available OSPI memory space */
        err = g_ospi1.p_api->erase (g_ospi1.p_ctrl, (uint8_t *) (sp_gp_ospi_cs1 + (erase_size * block)), erase_size);
        assert(FSP_SUCCESS == err);
        wait_for_write();
    }

    transition_to_dopi();

    set_clock_speed_133mhz();

    printf_colour("\r\nWriting the text block to external Octo-SPI flash memory ...\r\n");

    write_test_opi(data_size / WRITE_BLOCK_SIZE);

    *ospi_performance_write_result = s_g_counter;

    printf_colour("\r\nWriting to flash completed\r\n");

    printf_colour("\r\nReading the text block from external Octo-SPI flash memory ...\r\n");

    read_test_opi(data_size / WRITE_BLOCK_SIZE);

    *ospi_performance_read_result = s_g_counter;

    printf_colour("\r\nReading from flash completed\r\n");

    R_OSPI_B_Close (g_ospi1.p_ctrl);
}
/**********************************************************************************************************************
 End of function ospi_performance_test
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: write_test_opi
 * Description  : write data to the OSPI flash and use a timer to time the write duration
 * Argument     : num_blocks - the number of blocks
 * Return Value : FSP_SUCCESS on success or an error code on failure
 *********************************************************************************************************************/
static fsp_err_t write_test_opi(uint32_t num_blocks)
{
    fsp_err_t err;

    fsp_err_t retval = FSP_SUCCESS;

    timer_status_t timer_status;

    srand(0);

    /* Cast to uint8_t: rand() returns int per C standard but only the least significant
     * byte (0-255) is required for the byte-wise pseudo-random test pattern. The cast
     * safely truncates to the lower 8 bits which is the intended behaviour for
     * generating a repeatable single-byte data pattern from the seeded sequence */
    volatile uint8_t data = (uint8_t) rand();

    R_GPT_Open(g_memory_performance.p_ctrl, g_memory_performance.p_cfg);

    /* use critical section so that interrupts don't affect the time measurements */
    FSP_CRITICAL_SECTION_DEFINE;
    FSP_CRITICAL_SECTION_ENTER;

    /* write the test data in OPI mode */
    s_g_counter = 0;

    for (s_test_current_block = 0; s_test_current_block < num_blocks; s_test_current_block++)
    {
        s_test_remain      = WRITE_BLOCK_SIZE;
        s_test_current_sub = 0;

        while (s_test_remain > 0)
        {
            s_test_remain--;

            /* Cast to uint8_t: rand() returns int but only the least significant byte
                     * is needed to match the byte-wise pseudo-random pattern used during write */
            data = (uint8_t)rand();

            s_g_test_data[s_test_current_sub++] = data;
        }

        /* only time the write function that includes the wait_for_write */
        R_GPT_Start(g_memory_performance.p_ctrl);


        /* Cast to uint8_t *: sp_gp_ospi_cs1 is the base address of the OSPI memory-mapped region
         * and the offset (s_test_current_block * WRITE_BLOCK_SIZE) calculates the destination
         * address for the current block. The resulting address is guaranteed to be within the
         * valid OSPI CS1 memory-mapped region as s_test_current_block is bounded by the
         * total number of test blocks and WRITE_BLOCK_SIZE is a fixed block size constant */
        err = g_ospi1.p_api->write (g_ospi1.p_ctrl, (s_g_test_data),
                                    (uint8_t *) (sp_gp_ospi_cs1 + (s_test_current_block * WRITE_BLOCK_SIZE)),
                                    WRITE_BLOCK_SIZE);

        wait_for_write();

        R_GPT_Stop(g_memory_performance.p_ctrl);
        R_GPT_StatusGet(g_memory_performance.p_ctrl, &timer_status);

        if (FSP_SUCCESS != err)
        {
            retval = err;
        }
    }

    FSP_CRITICAL_SECTION_EXIT;

    R_GPT_StatusGet(g_memory_performance.p_ctrl, &timer_status);
    s_g_counter = timer_status.counter;

    R_GPT_Close(g_memory_performance.p_ctrl);

    if (FSP_SUCCESS != retval)
    {
        print_to_console("Write failed");
    }

    return (retval);
}
/**********************************************************************************************************************
 End of function write_test_opi
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: read_test_opi
 * Description  : read data from the OSPI flash and use a timer to time the read duration
 * Argument     : block_size - the size of the block to read
 * Return Value : FSP_SUCCESS on success or an error code on failure
 *********************************************************************************************************************/
static fsp_err_t read_test_opi(uint32_t block_size)
{
    fsp_err_t err = FSP_SUCCESS;

    timer_status_t timer_status;

    uint32_t  failing_index = 0;
    uint8_t * p_ptr;

    srand(0);

    /* Cast to uint8_t: rand() returns int but only the least significant byte
             * is needed to match the byte-wise pseudo-random pattern used during write */
    volatile uint8_t  data   = (uint8_t) rand();
    volatile uint32_t errors = 0;

    /* scale block_size to bytes by multiplying by the block size */
    block_size *= WRITE_BLOCK_SIZE;

    s_data_size = TEST_BLOCKS * WRITE_BLOCK_SIZE;

    err = R_GPT_Open(g_memory_performance.p_ctrl, g_memory_performance.p_cfg);

    memset(s_read_block, 0, block_size);

    FSP_CRITICAL_SECTION_DEFINE;
    FSP_CRITICAL_SECTION_ENTER;

    R_GPT_Reset(g_memory_performance.p_ctrl);

    /* Read the data and time the read operation */
    p_ptr = sp_gp_ospi_cs1;

    R_GPT_Start(g_memory_performance.p_ctrl);

    memcpy(s_read_block, p_ptr, block_size);

    R_GPT_StatusGet(g_memory_performance.p_ctrl, &timer_status);

    s_g_counter = timer_status.counter;
    FSP_CRITICAL_SECTION_EXIT;

    R_GPT_Close(g_memory_performance.p_ctrl);

    p_ptr = s_read_block;

    /* Verify data */
    for (uint32_t i = 0; i < block_size; i++)
    {
        /* Cast to uint8_t: rand() returns int but only the least significant byte
                 * is needed to match the byte-wise pseudo-random pattern used during write */
        data = (uint8_t)rand();

        if ((*p_ptr) != data)
        {
            if (0 == errors)
            {
                failing_index = i;
            }

            err = FSP_ERR_INVALID_DATA;
            errors++;
        }

        p_ptr++;
    }

    if (errors > 0)
    {
        printf_colour("Failed to verify %lu bytes. First verification failure index %lu\r\n", errors, failing_index);
    }

    return (err);
}
/**********************************************************************************************************************
 End of function read_test_opi
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: reset_ospi_device
 * Description  : reset the OSPI flash device
 * Return Value : None
 *********************************************************************************************************************/
static void reset_ospi_device(void)
{
    /* configure the pin as GPIO */
    R_IOPORT_PinCfg(&g_ioport_ctrl, OSPI_RESET_PIN, (uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT |\
                    (uint32_t) IOPORT_CFG_PORT_OUTPUT_LOW);

    /* generate reset pulse */
    R_IOPORT_PinWrite(g_ioport.p_ctrl, OSPI_RESET_PIN, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(OSPI_RESET_DELAY, BSP_DELAY_UNITS_MILLISECONDS);
    R_IOPORT_PinWrite(g_ioport.p_ctrl, OSPI_RESET_PIN, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(OSPI_RESET_DELAY, BSP_DELAY_UNITS_MILLISECONDS);

    /* set to OSPI peripheral mode */
    R_IOPORT_PinCfg(&g_ioport_ctrl, OSPI_RESET_PIN, (uint32_t) IOPORT_CFG_DRIVE_HIGH | \
            (uint32_t) IOPORT_CFG_PERIPHERAL_PIN | (uint32_t) IOPORT_PERIPHERAL_OSPI);

    /* Reset flash device */
    R_XSPI0->LIOCTL_b.RSTCS0 = 0;
    R_BSP_SoftwareDelay(OSPI_RESET_DELAY, BSP_DELAY_UNITS_MICROSECONDS);
    R_XSPI0->LIOCTL_b.RSTCS0 = 1;
    R_BSP_SoftwareDelay(OSPI_RESET_DELAY, BSP_DELAY_UNITS_MICROSECONDS);
}
/**********************************************************************************************************************
 End of function reset_ospi_device
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: write_en
 * Description  : send a write enable commend to the OSPI flash device
 * Return Value : None
 *********************************************************************************************************************/
static void write_en(bool is_dopi)
{
    spi_flash_direct_transfer_t tfr =
    {
        .command = is_dopi ? WRITE_ENABLE_COMMAND_OPI : WRITE_ENABLE_COMMAND,
        .command_length = is_dopi ? 2U : 1U,
        .address_length = 0U,
        .data_length = 0U,
        .dummy_cycles = 0U
    };

    fsp_err_t err = g_ospi1.p_api->directTransfer(g_ospi1.p_ctrl, &tfr, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    assert(FSP_SUCCESS == err);

    tfr = (spi_flash_direct_transfer_t)
    {
        .command        = is_dopi ? READ_STATUS_COMMAND_OPI : READ_STATUS_COMMAND,
        .command_length = is_dopi ? 2U : 1U,
        .address_length = 0U,
        .data_length    = 1U,
        .dummy_cycles   = is_dopi ? READ_STATUS_DUMMY_CYCLES_OPI : REG_DUMMY_CYCLES_SPI,
    };

    err = g_ospi1.p_api->directTransfer(g_ospi1.p_ctrl, &tfr, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    assert(FSP_SUCCESS == err);

    if (0 == (tfr.data & WRITE_ENABLE_MASK))
    {
        __BKPT(0);
    }
}
/**********************************************************************************************************************
 End of function write_en
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: transition_to_dopi
 * Description  : transition the device from SPI mode to DOPI mode
 * Return Value : None
 *********************************************************************************************************************/
static void transition_to_dopi(void)
{
    fsp_err_t err = FSP_SUCCESS;

    spi_flash_direct_transfer_t tfr;
    spi_flash_direct_transfer_t tfr_read;

    write_en(false);

    /* Write to Volatile Configuration Register 0 */
    tfr.command        = WRITE_VOLATILE_CONFIGURATION_REGISTER;
    tfr.command_length = 1U;
    tfr.address        = 0;
    tfr.address_length = 3U;
    tfr.data_length    = 1U;
    tfr.data           = OCTAL_DDR_MODE;
    tfr.dummy_cycles   = 0U;
    err                = g_ospi1.p_api->directTransfer(g_ospi1.p_ctrl, &tfr, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);

    assert(FSP_SUCCESS == err);

    /* Device should now be in DOPI mode */

    /* Change the protocol mode of the driver */
    /* DS will auto-calibrate in this call */
    err = g_ospi1.p_api->spiProtocolSet(g_ospi1.p_ctrl, SPI_FLASH_PROTOCOL_8D_8D_8D);
    assert(FSP_SUCCESS == err);

    tfr_read.command        = READ_VOLATILE_CONFIGURATION_REGISTER;
    tfr_read.command_length = 1U;
    tfr_read.address        = 0;
    tfr_read.address_length = 4U;
    tfr_read.data           = 0U;
    tfr_read.data_length    = 1U;
    tfr_read.dummy_cycles   = 8u;

    err = g_ospi1.p_api->directTransfer(g_ospi1.p_ctrl, &tfr_read, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    assert(FSP_SUCCESS == err);

}
/**********************************************************************************************************************
 End of function transition_to_dopi
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: ospi_flash_open
 * Description  : open the OSPI driver and read the OSPI flash SFDP signature
 * Return Value : FSP_SUCCESS on success or an error code on failure
 *********************************************************************************************************************/
static fsp_err_t ospi_flash_open(void)
{
    fsp_err_t err = FSP_SUCCESS;

    reset_ospi_device();

    spi_flash_direct_transfer_t test_tfr =
    {
    .command = READ_SFDP_COMMAND,
    .command_length = 1,
    .address = 0,
    .address_length = 3,
    .data = 0,
    .data_length = 4,
    .dummy_cycles = READ_SFDP_DUMMY_CYCLES
    };

    err = g_ospi1.p_api->open(g_ospi1.p_ctrl, g_ospi1.p_cfg);
    err = ospi_b_init();

    if (FSP_SUCCESS == err)
    {
        err = g_ospi1.p_api->directTransfer(g_ospi1.p_ctrl, &test_tfr, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);

        if (SFDP_SIGNATURE != test_tfr.data)
        {
            err = FSP_ERR_NOT_INITIALIZED;
            return (err);
        }
    }

    return (err);
}
/**********************************************************************************************************************
 End of function ospi_flash_open
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: wait_for_write
 * Description  : Read the status register until it indicates that the write has completed
 * Return Value : None
 *********************************************************************************************************************/
static void wait_for_write(void)
{
    spi_flash_status_t status = { 0 };

    do
    {
        assert(FSP_SUCCESS == g_ospi1.p_api->statusGet (g_ospi1.p_ctrl, &status));
    }
    while (status.write_in_progress);
}
/**********************************************************************************************************************
 End of function wait_for_write
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: ospi_read_write_print
 * Description  : .
 * Argument     : block_size_actual
 * Return Value : .
 *********************************************************************************************************************/
static void ospi_read_write_print(uint32_t block_size_actual)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    uint32_t ospi_performance_write_result = 0;
    uint32_t ospi_performance_read_result  = 0;
    uint32_t ospi_read_result              = 0;
    uint32_t ospi_write_result             = 0;

    char_t   res_str[24];

    sprintf (s_print_buffer, "\r\n\r\nGenerated a text block of %2lu KB in SRAM\r\n", block_size_actual);
    print_to_console(s_print_buffer);

    block_size_actual = block_size_actual * 1024;

    ospi_performance_test (block_size_actual, &ospi_performance_write_result, &ospi_performance_read_result);

    ospi_write_result =  ospi_performance_write_result / GPT_CLOCK_MHZ;
    ospi_read_result  =  ospi_performance_read_result / GPT_CLOCK_MHZ;

    /* Handle error */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Fatal error */
        SYSTEM_ERROR_CFG_
    }

    print_to_console("\r\n-------------------------------------------------");
    print_to_console("\r\nOperation/Flash          Octo-SPI");
    print_to_console("\r\n-------------------------------------------------");

    sprintf (s_print_buffer, "\r\nWrite                    %6ld", ospi_write_result);
    print_to_console(s_print_buffer);

    sprintf (s_print_buffer, "\r\nRead                     %6ld", ospi_read_result);
    print_to_console(s_print_buffer);

    print_to_console("\r\n-------------------------------------------------");
    print_to_console("\r\nNote: Times are in microseconds");
    print_to_console(MENU_RETURN_INFO);

    sprintf (res_str, "%6ld uS", ospi_write_result);
    sprintf (res_str, "%6ld uS", ospi_read_result);
    sprintf (res_str, "Block Size %6ld Bytes", block_size_actual);

}
/**********************************************************************************************************************
 End of function ospi_read_write_print
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: value_validation
 * Description  : Ensure a provided value is divisible by the defined BLOCK_LIMIT.
 * Argument     : value
 * Return Value : the valid value
 *********************************************************************************************************************/
static uint32_t value_validation(uint32_t value)
{
    if (value < 2)
    {
        value = INVALID_BLOCK_SIZE;
    }
    if ((value > BLOCK_LIMIT_KB) && (value < INVALID_MARKERS))
    {
        value = INVALID_BLOCK_SIZE;
    }
    if ((value > 0) && ((value % 2) != 0)  && (value < INVALID_MARKERS))
    {
        value = INVALID_BLOCK_BOUNDARY;
    }
    return value;
}
/**********************************************************************************************************************
 End of function value_validation
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: check_input_value
 * Description  : Check the value obtained is a valid block size.
 * Arguments    : value: valid_block_size
 * Return Value : true or false if the block size is valid or not
 *********************************************************************************************************************/
static bool_t check_input_value(uint32_t value)
{
    switch (value)
    {

        case INVALID_CHARACTER:
        {
            sprintf(s_print_buffer,
                    "\r\n> Invalid character in entry, enter the text block size specifying a 2K boundary (eg 24)"\
                    " and press tab : ");

            /* ignoring -Wpointer-sign is OK when treating signed char_t array as as unsigned */
            print_to_console(s_print_buffer);
            break;
        }

        case INVALID_BLOCK_SIZE:
        {
            sprintf(s_print_buffer, "\r\n> Invalid size, enter the text block size (eg 24) and press tab " \
                    ": ");

            /* ignoring -Wpointer-sign is OK when treating signed char_t array as as unsigned */
            print_to_console(s_print_buffer);
            break;
        }

        case INVALID_BLOCK_BOUNDARY:
        {
            sprintf(s_print_buffer,
                    "\r\n> Invalid boundary, enter the text block size specifying a 2K boundary (eg 24)"\
                    " and press tab : ");

            /* ignoring -Wpointer-sign is OK when treating signed char_t array as as unsigned */
            print_to_console(s_print_buffer);
            break;
        }

        default:
        {
            return (true);
        }
    }
    return (false);
}
/**********************************************************************************************************************
 End of function check_input_value
 *********************************************************************************************************************/


/**********************************************************************************************************************
 * Function Name: ospi_menu_print
 * Description  : .
 * Return Value : .
 *********************************************************************************************************************/
static void ospi_menu_print(void)
{
    clear_console();
    sprintf (s_print_buffer, MODULE_NAME, g_selected_menu);
    print_to_console(s_print_buffer);
    print_to_console(SUB_OPTIONS);
}
/**********************************************************************************************************************
 End of function ospi_menu_print
 *********************************************************************************************************************/


