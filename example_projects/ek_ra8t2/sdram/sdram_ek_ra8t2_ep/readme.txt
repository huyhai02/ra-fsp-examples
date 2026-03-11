/**********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions
**********************************************************************************************************************/

1. Project Overview:
	This project demonstrates the basic functionalities of SDRAM on Renesas RA MCUs based on Renesas FSP.
	The example project writes data to the entire SDRAM using DMAC. It demonstrates SDRAM settings and data
	retention by enabling self-refresh in low power mode. When the Ultra-Low-Power Timer (ULPT) expires,
	the MCU returns to normal mode and reads the SDRAM to ensure the data is still retained. The EP status
	information and error messages are displayed on the host PC. LED1 is turned ON for 1 second before the MCU
	enters low power mode, then turned OFF. Additionally, LED3 is turned ON if an error occurs.

	Note:
	- To display information, the user can choose between the SEGGER J-Link RTT Viewer and the serial terminal (UART)
	  with J-Link OB VCOM. It is important to note that the user should only operate a single terminal application at
	  a time to avoid conflicts or data inconsistencies.
	- For instructions on how to switch between these options, please refer to the "Verifying Operation" section in
	  sdram_notes.md file.
	- By default, the EP information is printed to the host PC using the serial terminal for boards that support
	  J-Link OB VCOM. For the RA boards that do not support J-Link OB VCOM, the EP uses the SEGGER J-Link RTT Viewer
	  by default instead.
	- RA boards that support J-Link OB VCOM: EK-RA8D1, EK-RA8E2, EK-RA8P1, EK-RA8D2, EK-RA8T2.

2. Hardware Requirements:
	Supported RA Boards: EK-RA8D1, EK-RA8E2, EK-RA8P1, EK-RA8D2, EK-RA8T2.
	1 x Host PC.
	1 x Renesas RA board.
	1 x Type-C USB cable for programming and debugging.

3. Hardware Connections:
	Connect the RA board to the host PC via a Type-C USB cable for programming and debugging.

	For EK-RA8D1:
		Set the configuration switches (SW1) as below.
		+-------------+-------------+--------------+------------+------------+------------+-------------+-----------+
		| SW1-1 PMOD1 | SW1-2 TRACE | SW1-3 CAMERA | SW1-4 ETHA | SW1-5 ETHB | SW1-6 GLCD | SW1-7 SDRAM | SW1-8 I3C |
		+-------------+-------------+--------------+------------+------------+------------+-------------+-----------+
		|     OFF     |     OFF     |      OFF     |     OFF    |     OFF    |     OFF    |     ON      |    OFF    |
		+-------------+-------------+--------------+------------+------------+------------+-------------+-----------+

	For EK-RA8T2:
		The user must set the configuration switches (SW6 and SW4) as below to use the on-board debug functionality.
		+-------+-------+-------+-------+-------+-------+-------+-------+-------+--------+
		| SW6-1 | SW6-2 | SW6-3 | SW6-4 | SW6-5 | SW6-6 | SW6-7 | SW6-8 | SW6-9 | SW6-10 |
		+-------+-------+-------+-------+-------+-------+-------+-------+-------+--------+
		|  OFF  |  OFF  |  OFF  |  OFF  |  ON   |  OFF  |  ON   |  ON   |  ON   |   ON   |
		+-------+-------+-------+-------+-------+-------+-------+-------+-------+--------+

		+-------+-------+-------+-------+-------+-------+-------+-------+
		| SW4-1 | SW4-2 | SW4-3 | SW4-4 | SW4-5 | SW4-6 | SW4-7 | SW4-8 |
		+-------+-------+-------+-------+-------+-------+-------+-------+
		|  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |  OFF  |
		+-------+-------+-------+-------+-------+-------+-------+-------+

Note:
1) For using the serial terminal (UART):
   The serial terminal setting parameters are as follows:
	Select the COM port provided by the J-Link on-board.
	Speed: 115200 bps
	Data: 8 bit
	Parity: none
	Stop bit: 1 bit
	Flow control: none

2) For using the J-Link RTT Viewer:
   If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the project folder (e2studio\Debug or e2studio\Release).
