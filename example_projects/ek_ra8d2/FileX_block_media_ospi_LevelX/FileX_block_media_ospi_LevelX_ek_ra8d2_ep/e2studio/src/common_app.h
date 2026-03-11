/***********************************************************************************************************************
 * File Name    : common_app.h
 * Description  : Contains data structures and functions used in the EP.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef COMMON_APP_H_
#define COMMON_APP_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "common_utils.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define ONE_SLEEP_TICK              (1U)
#define TERMINAL_SLEEP_TICK         (1U)

/* Macros for the terminal framework */
#define TERMINAL_BYTE_POOL_NAME     "terminal_byte_pool_name"
#define TERMINAL_BYTE_POOL_SIZE     (4096U)

/* Macros for the SCI UART handle */
#define TX_MAX_SIZE                 (2048U)
#define RX_MAX_SIZE                 (16U)
#define CHAR_CR                     (0x0D)

/* Macros for converter size of bytes to string */
#define THOUSAND                    (1000UL)
#define MILLION                     (1000000UL)
#define BILLION                     (1000000000UL)
#define STR_UL64_MAX_LEN            (16U)

#define FILE_SYSTEM_REQUEST_MASK    (0x0000FFFC)

/* Macro for the menus */
#define EP_VERSION              	("1.0")
#define MODULE_NAME             	"FileX block media ospi LevelX"

#define BANNER_INFO     "\r\n********************************************************************************"\
						"\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module       *"\
						"\r\n*   Example Project Version %s                                                *"\
						"\r\n*   Flex Software Pack Version %d.%d.%d                                           *"\
						"\r\n********************************************************************************"\
						"\r\nRefer to readme.txt file for more details on Example Project and"\
						"\r\nFSP User's Manual for more information about "MODULE_NAME" module\r\n"

#define EP_INFO         "\r\nThis example project demonstrates the operation of the FileX file system, incorporating"\
						"\r\nLevelX wear leveling features on the OSPI block media of the RA MCU. The project will"\
						"\r\nperform various FileX file system operations based on the user's selection from menu"\
						"\r\noption, such as erase OSPI Flash, media management (open, format), directory management"\
						"\r\n(create, get properties, delete), file management (create, write, read, delete), and"\
						"\r\nsector level operation (defragment, write, read, erase)."\
						"\r\nThe terminal application is used as the user interface. The menu options and system"\
						"\r\nmessages (errors and information messages) will be printed on the terminal application"\
						"\r\nduring the execution of the project.\r\n\r\n"

#define MAIN_MENU                   "\r\n\r\nMain Menu:"\
                                    "\r\n  1. Flash Media operation."\
                                    "\r\n  2. Directory operation."\
                                    "\r\n  3. File operation."\
									"\r\n  4. Sector operation."\
                                    "\r\n  User Input:  "

#define FLASH_MEDIA_MENU            "\r\n\r\nMain Menu >"\
									"\r\n  Flash Media option:"\
                                    "\r\n    1. Erase entire Flash."\
                                    "\r\n    2. Format Media."\
                                    "\r\n    3. Open Media."\
									"\r\n    4. Defragment the Flash."\
									"\r\n    5. Go back main menu"\
                                    "\r\n    User Input:  "

#define DIRECTORY_MENU              "\r\n\r\nMain Menu >"\
                                    "\r\n  Directory operation:"\
                                    "\r\n    1. Create Directory."\
                                    "\r\n    2. Get Directory Properties."\
                                    "\r\n    3. Delete Directory."\
                                    "\r\n    4. Go back Main Menu."\
                                    "\r\n    User Input:  "

#define FILE_MENU                   "\r\n\r\nMain Menu >"\
                                    "\r\n  File operation:"\
                                    "\r\n    1. Create Empty File."\
                                    "\r\n    2. Write To File."\
                                    "\r\n    3. Read From File."\
                                    "\r\n    4. Delete File."\
                                    "\r\n    5. Go back Main Menu."\
                                    "\r\n    User Input:  "

#define SECTOR_MENU  			   "\r\n\r\nMain Menu >"\
								    "\r\n  Sector operation:"\
								    "\r\n    1. Write to Sector."\
								    "\r\n    2. Read from Sector."\
								    "\r\n    3. Erase Sector."\
								    "\r\n    4. Go back Main Menu."\
								    "\r\n    User Input:  "

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/* Enumerate for the terminal event */
typedef enum terminal_event
{
    TERMINAL_INPUT_MESSAGE,
    TERMINAL_OUTPUT_EP_BANNER,
    TERMINAL_OUTPUT_EP_INFO,
    TERMINAL_OUTPUT_APP_INFO_STR,
    TERMINAL_OUTPUT_APP_ERR_STR,
    TERMINAL_OUTPUT_APP_ERR_TRAP,
    TERMINAL_OUTPUT_APP_MENU,
    TERMINAL_OUTPUT_APP_MEDIA_PROPERTY,
    TERMINAL_OUTPUT_SECTOR_INFO,
    TERMINAL_OUTPUT_APP_MEDIA_VOLUME_INFO,
    TERMINAL_OUTPUT_APP_ENTRY_INFO,
    TERMINAL_OUTPUT_APP_DIR_PROPERTY
} terminal_event_t;

/* Enumerate for the Menu screen */
typedef enum e_menu
{
    MENU_MAIN = 0,
    MENU_FLASH_MEDIA,
    MENU_DIR,
    MENU_FILE,
	MENU_SECTOR,
    MENU_MAX
} menu_t;

/* Enumerate for month values */
typedef enum e_month
{
    JANUARY   = 1,
    FEBRUARY  = 2,
    MARCH     = 3,
    APRIL     = 4,
    MAY       = 5,
    JUNE      = 6,
    JULY      = 7,
    AUGUST    = 8,
    SEPTEMBER = 9,
    OCTOBER   = 10,
    NOVEMBER  = 11,
    DECEMBER  = 12,
} month_t;

/* Structure used to exchange information between application thread and terminal thread */
typedef struct terminal_msg
{
    terminal_event_t id;
    uint32_t size;
    uint32_t time;
    CHAR msg[];
} terminal_msg_t;

/* Structure to store date and time */
typedef struct st_time
{
    UINT month;
    UINT date;
    UINT year;
    UINT hour;
    UINT min;
    UINT sec;
} time_t;

/* Structure to store entry details */
typedef struct st_entry_info
{
    CHAR name[FX_MAX_LONG_NAME_LEN];
    UINT attr;
    ULONG size;
    time_t time;
} entry_info_t;

/* Structure to store directory property details */
typedef struct st_dir_property
{
    UINT file;
    UINT subdir;
    ULONG size;
} dir_property_t;

/* Structure to store media property details */
typedef struct st_media_property
{
    ULONG64 phy_size;
    ULONG64 total_size;
    ULONG64 free_size;
    CHAR *  format_type;
} media_property_t;

/* Structure to store sector info details */
typedef struct st_sector_info
{
    ULONG *sector_map_entry;
    ULONG *sector_address;
    ULONG logical_sector;
} sector_info_t;

/***********************************************************************************************************************
 * Public function prototypes
 **********************************************************************************************************************/
UINT terminal_input_handle(void);
UINT terminal_output_handle(void);
UINT terminal_thread_init_check(void);

#endif /* COMMON_APP_H_ */
