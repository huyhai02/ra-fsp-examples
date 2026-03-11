/**********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions
**********************************************************************************************************************/

1. Project Overview:
	This example project demonstrates basic functionalities of LittleFS for external flash (e.g., SPI, OSPI, QSPI)
	on RA MCU based on FSP. The project will perform various file system operations based on the user's selection
	from menu options, such as erase entire flash, erase 32 KB flash, format file system, directory management
	(create, get properties, delete), and file management (create, write, read, delete). The terminal application
	is used as the user interface. The menu options and system messages (errors and information messages)
	will be printed on the terminal application during the execution of the project.

	Note:
	- To display information, users can select between the SEGGER J-Link RTT Viewer and the serial terminal (UART)
	  with J-Link OB VCOM. It is important to note that the user should only operate a single terminal application
	  at a time to avoid conflicts or data inconsistencies. For instructions on how to switch between these options,
	  please refer the first step in the "5. Verifying Operation" section of this file.
	- By default, EP information is printed to the host PC using the serial terminal for boards that support J-Link
	  OB VCOM. Vice versa, for the RA boards that do not support J-Link OB VCOM, EP uses the SEGGER J-Link RTT
	  Viewer by default instead.
	- RA boards supported for J-Link OB VCOM: EK-RA8D1, EK-RA8T2.

2. Hardware Requirements:
	Supported RA board: EK-RA8D1, EK-RA8T2.
	1 x Renesas RA board.
	1 x USB cable for programming and debugging.

3. Hardware Connections:
	Connect the USB Debug port on the RA board to the host PC via a USB cable.

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

4. Software Configurations:
	By default, to create an empty file or open an existed file with malloc
		LittleFS | Use Malloc		: Enabled
		BSP 	 | Heap size (bytes) 	: 0x1000

	To create an empty file or open an existed file without malloc
		LittleFS | Use Malloc		: Disabled
		BSP 	 | Heap size (bytes) 	: 0

5. Verifying Operation:
	1. Import the example project.
		* By default, the EP supports serial terminal for RA boards that support J-link OB VCOM
		Define USE_VIRTUAL_COM=1 macro in
			Project Properties -> C/C++ Build -> Settings -> Tool Settings -> Compiler -> Includes > Macro Defines (-D)
	
		* To use SEGGER J-Link RTT Viewer, please follow the instructions as below:
		Define USE_VIRTUAL_COM=0 macro in
			Project Properties -> C/C++ Build -> Settings -> Tool Settings -> Compiler -> Includes > Macro Defines (-D)
	2. Generate, build the example project.
	3. Connect the RA MCU debug port to the host PC via a USB cable.
	4. Open a serial terminal application on the host PC and connect to the COM Port provided by the J-Link
	   on-board or open J-Link RTT Viewer (In case user selected SEGGER J-Link RTT Viewer or RA boards do not
	   support J-Link OB VCOM).
	   Note:
		For using the serial terminal application:
		a. Please ensure that the connection to the SEGGER J-Link RTT Viewer has been terminated.
	   	b. To echo back what was typed in Tera Term, the user needs to enable it through:
			[Setup] -> [Terminal...] -> Check [Local echo].
		c. The configuration parameters of the serial port on the terminal application are as follows:
			COM port is a port provided by the J-Link on-board.
			Baud rate: 115200 bps
			Data length: 8 bit  
			Parity: none
			Stop bit: 1 bit
			Flow control: none

		For using SEGGER J-Link RTT Viewer:
		If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
		_SEGGER_RTT) in .map file generated in the project folder (e2studio\Debug or e2studio\Release).
	5. Debug or flash the example project to the RA board.
	6. After the main menu is displayed on the terminal application, the user selects options to perform file system
	   management as desired.
		Type '1' and enter to select Flash Operations Menu.
			Type '1' and enter to erase the entire flash.
			Type '2' and enter to erase the first 32 KB of the flash.
			Type '3' and enter to format the file system.
			Type '4' and enter to go back to the Main Menu.
			Note: After erasing the entire flash or 32 KB flash, the user must format the file system.

		Type '2' and enter to select Directory Operations Menu.
			Type '1' and enter to create a new directory.
			Type '2' and enter to get the root directory properties.
			Type '3' and enter to delete a directory.
			Type '4' and enter to go back to the Main Menu.

		Type '3' and enter to select File Operations Menu.
			Type '1' and enter to create an empty file or open an existed file.
			Type '2' and enter to write a fixed content into a file.
			Type '3' and enter to read the entire file and display the first 1 KB of its content.
			Type '4' and enter to delete a file.
			Type '5' and enter to go back to the Main Menu.
