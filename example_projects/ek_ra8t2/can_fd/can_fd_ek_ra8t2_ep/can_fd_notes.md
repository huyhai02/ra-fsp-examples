# CAN FD Communication Example on RA Boards

## Table of Contents
1. [Introduction](#introduction)
2. [Required Resources](#required-resources)
   1. [Hardware Requirements](#hardware-requirements)
      1. [Required Boards](#required-boards)
	  2. [Supported Boards](#supported-boards)
      3. [Additional Hardware](#additional-hardware)
      4. [Hardware Connections](#hardware-connections)
   2. [Software Requirements](#software-requirements)

3. [Verifying the application](#verifying-the-application)
4. [Project Notes](#project-notes)
   1. [System Level Block Diagram](#system-level-block-diagram)
   2. [FSP Modules Used](#fsp-modules-used)
   3. [Module Configuration Notes:](#module-configuration-notes)
   4. [API Usage](#api-usage)
   5. [Memory Usage](#memory-usage)
   6. [Clock Configuration](#clock-configuration)
   7. [Application Execution Flow](#application-execution-flow)
   8. [Troubleshooting Tips](#troubleshooting-tips)
   9. [Known Limitations](#known-limitations)
5. [Special Topics](#special-topics)
6. [Conclusion and Next Steps](#conclusion-and-next-steps)
7. [References](#references)
8. [Notice](#notice)

# Introduction 
This sample project demonstrates CAN FD communication using Renesas RA microcontrollers across two RA boards. During runtime, users can set the nominal rate, FD data rate, and sample point, then begin transmission using the chosen settings. To halt transmission and adjust the baud rate, users simply press Enter without inputting a value, which returns them to the configuration menu.

In operation, data is initially sent from Board 1 to Board 2. Upon receiving the data, Board 2 displays it on the terminal and responds with updated data. Board 1 then receives this response, converts the CAN frame into a CAN FD frame, and sends it back to Board 2. After receiving the CAN FD frame, Board 2 updates the data once more and transmits it to Board 1. Finally, Board 1 displays the received data on the terminal.

Please refer to the [Example Project Usage Guide](https://github.com/renesas/ra-fsp-examples/blob/master/example_projects/Example%20Project%20Usage%20Guide.pdf) 
for general information on example projects.

# Required Resources 
To build and run the CAN FD example project successfully, specific hardware, software, and supporting resources are essential. These components facilitate proper configuration, compilation, and execution on Renesas RA MCUs. Typically, the required items include development boards, communication interfaces, development tools, and relevant documentation. A detailed list of resources for the CAN FD example project is provided in the following sections.

**Note: Some RA boards come with an on-board CAN FD transceiver, while others do not. However, boards without a built-in transceiver still support CAN FD and can be used with an off-the-shelf supplemental CAN FD transceiver.**

## Hardware Requirements 

### Required Boards 
* 2 x RA boards.

### Supported Boards 
* EK-RA6M5, EK-RA8T2.

### Additional Hardware 
* 2 x Type-C USB cables for programming and debugging.
* For using on-board CAN Transceivers: 3 x Jumper wires with both ends female.
* For using external CAN-FD Transceiver Board (ex.OM13099):
    * 1 x OM13099 (CAN-FD Transceiver Board):
        * Link product: [OM13099 (CAN-FD Transceiver Board)](https://www.nxp.com/products/interfaces/can-transceivers/can-with-flexible-data-rate/dual-can-fd-transceiver-shield:OM13099).
        * Manufacturer Product Number: OM13099.
    * 9 x Jumper wires with both ends female.

### Hardware Connections 
* Note:
    * Requires 2 RA boards to run the project.
    * The boards can be of the same type (E.g. EK-RA8M1 <--> EK-RA8M1) or different type (E.g. EK-RA8M1 <--> EK-RA6T3). 
    * In case of the same type of boards, the on-board CAN transceivers mounted (if populated) can be used for testing by connecting the CANH, CANL and GND. 
    * In case of the same type of boards (E.g. EK-RA8D1 <--> EK-RA8D1), (if on-board CAN transceivers not populated) external OM13099 board can be used to test the application by connecting the required wires as below.
    * In case of the different types of boards (EK-RA8D1 <--> EK-RA6T3), which doesn't contain on-board CAN transceivers, OM13099 can be used as externally to test the application by connecting the required wires as described in the optional sections, for the particular boards.

* For EK-RA6M5:
    * For using on-board CAN Transceivers:
        - Short E33, E34 & E35.
        - J33:1 of Board 1 to J33:1 of Board 2.
        - J33:2 of Board 1 to J33:2 of Board 2.
        - J33:3 of Board 1 to J33:3 of Board 2.

    * (Optional) For using external CAN-FD Transceiver Board:
    	- Note: To use the external CAN-FD transceiver, the user must either cut E33, E34, and E35 to prevent conflicts with the on-board CAN transceiver or configure alternative CAN CTX and CRX pins instead of cutting E33, E34, and E35.

        |     Board 1      | OM13099 (CAN-FD Transceiver Board) |     Board 2      |
		|------------------|------------------------------------|------------------|
		| CTX P609 (J3:12) |           CAN0 TX (J3:4)           |        ---       |
		| CRX P610 (J3:13) |           CAN0 RX (J3:2)           |        ---       |
		|   VCC (J18:4)    |            3V3 (J3:12)             |        ---       |
		|   5V0 (J18:5)    |            5V0 (J3:14)             |        ---       |
		|   GND (J18:6)    |            GND (J3:16)             |        ---       |
		|        ---       |           CAN1 TX (J1:4)           | CTX P609 (J3:12) |
		|        ---       |           CAN1 RX (J1:2)           | CRX P610 (J3:13) |
		|        ---       |           P3:2 <-> P3:13           |        ---       |
		|        ---       |           P3:7 <-> P3:18           |        ---       |

* For EK-RA8T2: 
	* The user must set the configuration switches (SW6 and SW4) as below to use the on-board debug functionality and CANFD channel 1.

    | SW6-1 | SW6-2 | SW6-3 | SW6-4 | SW6-5 | SW6-6 | SW6-7 | SW6-8 | SW6-9 | SW6-10 |
	|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:------:|
	|  OFF  |  OFF  |  OFF  |  OFF  |  ON   |  OFF  |  ON   |  ON   |  ON   |  ON    |

    | SW4-1 | SW4-2 | SW4-3 | SW4-4 | SW4-5 | SW4-6 | SW4-7 | SW4-8 |
	|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|:-----:|
	|  OFF  |  OFF  |  OFF  |  ON   |  OFF  |  OFF  |  OFF  |  OFF  |

    * Use on-board CAN Transceiver:
        - J38:1 of Board 1 to J38:1 of Board 2.
        - J38:2 of Board 1 to J38:2 of Board 2.
        - J38:3 of Board 1 to J38:3 of Board 2.

## Software Requirements
* Renesas Flexible Software Package (FSP): Version 6.4.0
* e2 studio: Version 2025-12
* SEGGER J-Link RTT Viewer: Version 9.14a
* LLVM Embedded Toolchain for ARM: Version 21.1.1
* Terminal Console Application: Tera Term or a similar application

# Verifying the application 
**Note: In this example project, the same code runs on both boards. The board from which the user initiates the transmission becomes the Board 1 and board which receives the data becomes Board 2.**
**Note: Users are required to make sure 2 instances of e2studio, terminal are required to run and debug the application.**
	
1. Import the example project.
    By default, the EP supports serial terminal for RA boards that support J-link OB VCOM
    * Define USE_VIRTUAL_COM = 1 macro in Project Properties -> C/C++ Build -> Settings -> Tool Settings -> Compiler -> Includes -> Macro Defines (-D)

    To use SEGGER J-Link RTT Viewer, please follow the instructions below:
    * Define USE_VIRTUAL_COM = 0 macro in Project Properties -> C/C++ Build -> Settings -> Tool Settings -> Compiler -> Includes -> Macro Defines (-D)
2. Generate, and build the example project.
3. Before running the example project, make sure hardware connections are completed.
4. Connect the RA MCU debug port to the host PC via a Type-C USB cable.
5. For using the serial terminal (UART):  
    * To echo back what was typed in Tera Term, the user needs to enable it through: [Setup] -> [Terminal...] -> Check [Local echo].
    * The configuration parameters of the serial port on the terminal application are as follows:
        * COM port is a port provided by the J-Link on-board.
        * Speed: 115200 bps
	    * Data: 8 bit  
	    * Parity: none
	    * Stop bits: 1 bit
	    * Flow control: none
6. Debug or flash the EP to the RA board to view output log.

The images below showcase the output on the serial terminal application (Tera Term):

![EP_Info](images/canfd_ep_info.png "EP Info")

Menu option:

![Menu_option](images/canfd_menu_option.png "Menu option")

Change baudrate runtime (make sure the configured baud rate of 2 Boards are same):

![Change_nominal_rate](images/change_nominal_rate.png "Change nominal rate with suggestion value")

![Change_nominal_rate_custom](images/change_nominal_rate_custom.png "Change nominal rate with custom value")

![Change_data_rate](images/change_data_rate.png "Change data rate with suggestion value")

![Change_data_rate_custom](images/change_data_rate_custom.png "Change data rate with custom value")

![Change_sample_point](images/change_sample_point.png "Change sample point value")

Data Transmission from Board 1 to Board 2

Board 1 log:

![canfd_terminal_output](images/B1_to_B2_B1_log.png "Board 1 log")

Board 2 log:

![canfd_terminal_output](images/B1_to_B2_B2_log.png "Board 2 log")

# Project Notes 

This section provides a system-level block diagram of the CAN FD EP that visually represents the overall architecture, highlighting how different modules interact and how data flows through the system. 
It shows FSP modules CAN, and GPT, which are essential for the application's functionality. Module configuration details are generally covered in the FSP User Manual (UM), with additional notes provided only when specific configurations deviate from the standard setup and require user attention. API usage is documented with references to the FSP UM, and the actual implementation of these APIs is illustrated in the application flow diagram.

Memory usage is outlined, including RAM and Flash consumption, along with the initial FSP version used during the release of the example project, categorized by MCU group and compiler. Lastly, any non-default clock configurations and special considerations for clock setup are clearly documented to ensure proper system operation.

## System-Level Block Diagram 

![can_fd](images/can_fd-hld.drawio.png "CAN FD Block Diagram")

## FSP Modules Used 

List all the various modules that are used in this example project. Refer to the FSP User Manual for further details on each module listed below.

| Module Name | Usage  | Searchable Keyword|
|-------------|-----------------------------------------------|-----------------------------------------------|
| CANFD | CANFD module is used to support CAN with both flexible data rate and classic frame, to support multiple channel operation and gateway function. | CAN |
| GPT | GPT is used to measure execution time of CANFD module operations. | r_gpt |

## Module Configuration Notes 

This section describes FSP Configurator properties which are important or different from those selected by default. 

**Note**: Following is the formula for data and nominal rate calculation.

**baud rate = canfd_clock_hz / ((time_segment_1 + time_segment_2 + 1) * prescaler)**

Refer [FSP User Manual on GitHub](https://renesas.github.io/fsp/group___c_a_n_f_d.html) for more details like min, max values of time_segment_1, time_segment_2 and prescaler.

These values are chosen by taking reference of ek-ra8m2 hardware user manual from the **Section 41.4.1**

**Configuration Properties for CANFD instance**
|   Module Property Path and Identifier   |   Default Value   |   Used Value   |   Reason   |
| :-------------------------------------: | :---------------: | :------------: | :--------: |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Common > Reception > Acceptance Filtering > Channel 1 Rule Count | 16 | 16 | Number of acceptance filter list rules dedicated to Channel 1. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Bitrate > Manual > Use manual settings | No | Yes | Use manual baud rate values so that automatic baud rate values are overwritten and user configured baud rate values are used. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Transmit Interrupts > TXMB0 | ☐ | ☑ | Relevant Tx buffer is enabled to trigger interrupt after transmission is complete. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Channel Error Interrupts > Error Warning | ☐ | ☑ | Select which channel error interrupt sources to enable. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Channel Error Interrupts > Error Passive | ☐ | ☑ | Select which channel error interrupt sources to enable. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Channel Error Interrupts > Bus-Off Entry | ☐ | ☑ | Select which channel error interrupt sources to enable. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Channel Error Interrupts > Bus-Off Recovery | ☐ | ☑ | Select which channel error interrupt sources to enable. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Channel Error Interrupts > Overload | ☐ | ☑ | Select which channel error interrupt sources to enable. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Global Error Interrupt > Sources > DLC Check | ☐ | ☑ | When enabled received messages will be rejected if their DLC field is less than the value configured in the associated AFL rule. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Global Error Interrupt > Sources > Message Lost | ☐ | ☑ | As there is no interrupt for message buffer reception it is recommended to use RX FIFOs instead. Set this value to 0 to disable RX Message Buffers. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Global Error Interrupt > Sources > FD Payload Overflow | ☐ | ☑ | Configure whether received messages larger than the destination buffer should be truncated or rejected. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > Message Buffers > Payload Size | 8 bytes | 64 bytes | Maximum data payload size of 64 is specified here for the RX message buffer to accommodate data length in FD frame. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > FIFOs > FIFO 0 > Enable | Disabled | Enabled | Enable RX FIFO 0. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > FIFOs > FIFO 0 > Payload Size | 8 bytes | 64 bytes | Select the message payload size for RX FIFO 0. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > FIFOs > FIFO 0 > Depth | 16 stages | 8 stages | Select the number of stages for RX FIFO 0. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > FIFOs > FIFO 1 > Enable | Disabled | Enabled | Enable RX FIFO 1. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > FIFOs > FIFO 1 > Payload Size | 8 bytes | 64 bytes | Select the message payload size for RX FIFO 1. |
| configuration.xml > Stacks > g_canfd0 CAN FD Lite (r_canfdlite) > Properties > Settings > Property > Module g_canfd0 CAN FD Lite (r_canfdlite) > Reception > FIFOs > FIFO 1 > Depth | 16 stages | 8 stages | Select the number of stages for RX FIFO 1. |

**Configuration Properties for GPT instance**
|   Module Property Path and Identifier   |   Default Value   |   Used Value   |   Reason   |
| :-------------------------------------: | :---------------: | :------------: | :--------: |
| configuration.xml > g_timer Timer, General PWM (r_gpt) > Properties > Settings > Property > Module g_timer Timer, General PWM (r_gpt) > General > Channel | 0 | 0 | Use GPT Channel 0 to measure execution time of CANFD operations. |
| configuration.xml > g_timer Timer, General PWM (r_gpt) > Properties > Settings > Property > Module g_timer Timer, General PWM (r_gpt) > General > Period | 0x10000 | 0x100000000 | Set the period to 0x100000000 raw count. |

## API Usage 
The table below link directs you to the CAN FD API used at the application layer by this example project.
- [CAN FD Module APIs on FSP User Manual on GitHub](https://renesas.github.io/fsp/group___c_a_n_f_d.html)
- [Timer, General PWM Module APIs on FSP User Manual on GitHub](https://renesas.github.io/fsp/group___g_p_t.html)

## Memory Usage 
**Memory Usage of can_fd Example Project in Bytes**
This section outlines the code and data memory consumption in the example project, encompassing both the HAL driver and application code. It provides an estimate of the memory requirements for the module and the application.

|   Compiler                              |   text	        |   data         |   .bss           |
| :-------------------------------------: | :-------------: | :------------: | :--------------: |
|   LLVM                                  |   35394 Bytes   |   136 Bytes    |    3768 Bytes    |

**Note: For users seeking detailed insights into the memory usage of individual modules and functions, the following third-party tool can be used to analyze memory consumption at a granular level.**
- [Memory usage tool](https://www.sikorskiy.net/info/prj/amap/)

## Clock Configuration 

If the clock configuration deviates from the default or requires special handling for specific EPs, those details will be documented here to support EP demonstration. However, for the CANFD EP, no special clock adjustments are necessary.

## Application Execution Flow 
This section describes the sequence of events and usage of API during the execution flow of the application.
The diagram shows the overall CAN FD communication flow: the application initializes the CAN FD module, the user sets baud rates and sample points, and transmission begins. The system then sends and receives CAN FD and ACK frames via the HAL, compares transmitted and received data for verification, and finally confirms successful CAN operation to the user.

![can_fd](images/can_fd-sequence.drawio.png "CAN FD Sequence Diagram")

## Troubleshooting Tips 
Note:
1. Ensure the configured bitrate of 2 Boards are same.
2. In this example project, the same code runs on both boards. The board from which the user initiates the transmission becomes the Board 1 and board which receives the data becomes Board 2.
3. The user is expected to enter data not exceeding 15 bytes in size.
4. For OM13099 (CAN Transceiver Board):  
    a. Connect P3 loopback connection on CAN Transceiver Board with jumper cables instead of a db9 serial cable.

    b. Connect jumpers J3, J4, J5 and J6 on OM13099 (CAN Transceiver Board) to establish connection to CAN termination resistors.

5. For using the SEGGER J-Link RTT Viewer: If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) in .map file generated in the project folder (e2studio\Debug or e2studio\Release).

## Known Limitations

No known limitations are seen in this EP.

# Special Topics 

LED behavior as below:  
* LED1 will be turned ON when CAN transmission operation is in progress.  
* LED2 will be turned ON when CAN transmission operation is successful.  
* LED3 will be turned ON when an error occurs.  

# Conclusion and Next Steps 
This example project provides a practical demonstration of CAN FD communication between two Renesas RA microcontroller boards. It highlights dynamic runtime configuration of nominal and data baud rates, sample points, and showcases bidirectional data exchange using both standard CAN and CAN FD frames. The use of identical application code across both boards streamlines deployment, while the hardware setup supports both on-board and external CAN FD transceivers for added flexibility.

Through the transmission and response sequence, users gain valuable hands-on experience in configuring and running CAN FD application on RA devices.

To further explore CAN FD implementation on Renesas RA MCUs:

Review the project source code located in the src directory.

Refer to the HAL driver and its documentation in the FSP User Manual for deeper technical insights.

Visit renesas.com for additional CAN FD resources, application notes, and documentation related to RA devices.

# References 
The following documents can be referred to for enhancing your understanding of the operation of this example project:
- [FSP User Manual on GitHub](https://renesas.github.io/fsp/)
- [FSP Known Issues](https://github.com/renesas/fsp/issues)
- [OM13099 (CAN-FD Transceiver Board)](https://www.nxp.com/products/interfaces/can-transceivers/can-with-flexible-data-rate/dual-can-fd-transceiver-shield:OM13099).
- [RA8M2 - (1GHz Arm Cortex-M85 and Cortex-M33 Dual-Core High-Performance MCU)](https://www.renesas.com/en/products/ra8m2)

# Notice

1. Descriptions of circuits, software and other related
information in this document are provided only to illustrate the
operation of semiconductor products and application examples. You are
fully responsible for the incorporation or any other use of the
circuits, software, and information in the design of your product or
system. Renesas Electronics disclaims any and all liability for any
losses and damages incurred by you or third parties arising from the use
of these circuits, software, or information. 

2. Renesas Electronics
hereby expressly disclaims any warranties against and liability for
infringement or any other claims involving patents, copyrights, or other
intellectual property rights of third parties, by or arising from the
use of Renesas Electronics products or technical information described
in this document, including but not limited to, the product data,
drawings, charts, programs, algorithms, and application examples. 

3. No license, express, implied or otherwise, is granted hereby under any
patents, copyrights or other intellectual property rights of Renesas
Electronics or others. 

4. You shall be responsible for determining what
licenses are required from any third parties, and obtaining such
licenses for the lawful import, export, manufacture, sales, utilization,
distribution or other disposal of any products incorporating Renesas
Electronics products, if required. 

5. You shall not alter, modify, copy,
or reverse engineer any Renesas Electronics product, whether in whole or
in part. Renesas Electronics disclaims any and all liability for any
losses or damages incurred by you or third parties arising from such
alteration, modification, copying or reverse engineering. 

6. Renesas Electronics products are classified according to the following two
quality grades: "Standard" and "High Quality". The intended applications
for each Renesas Electronics product depends on the product's quality
grade, as indicated below. "Standard": Computers; office equipment;
communications equipment; test and measurement equipment; audio and
visual equipment; home electronic appliances; machine tools; personal
electronic equipment; industrial robots; etc. "High Quality":
Transportation equipment (automobiles, trains, ships, etc.); traffic
control (traffic lights); large-scale communication equipment; key
financial terminal systems; safety control equipment; etc. Unless
expressly designated as a high reliability product or a product for
harsh environments in a Renesas Electronics data sheet or other Renesas
Electronics document, Renesas Electronics products are not intended or
authorized for use in products or systems that may pose a direct threat
to human life or bodily injury (artificial life support devices or
systems; surgical implantations; etc.), or may cause serious property
damage (space system; undersea repeaters; nuclear power control systems;
aircraft control systems; key plant systems; military equipment; etc.).
Renesas Electronics disclaims any and all liability for any damages or
losses incurred by you or any third parties arising from the use of any
Renesas Electronics product that is inconsistent with any Renesas
Electronics data sheet, user's manual or other Renesas Electronics
document. 

7. No semiconductor product is absolutely secure. Notwithstanding any security measures or features that may be implemented in Renesas Electronics hardware or software products, Renesas Electronics shall have absolutely no liability arising out of
any vulnerability or security breach, including but not limited to any unauthorized access to or use of a Renesas Electronics product or a system that uses a Renesas Electronics product. RENESAS ELECTRONICS DOES NOT WARRANT OR GUARANTEE THAT RENESAS ELECTRONICS PRODUCTS, OR ANY
SYSTEMS CREATED USING RENESAS ELECTRONICS PRODUCTS WILL BE INVULNERABLE OR FREE FROM CORRUPTION, ATTACK, VIRUSES, INTERFERENCE, HACKING, DATA LOSS OR THEFT, OR OTHER SECURITY INTRUSION ("Vulnerability Issues"). RENESAS ELECTRONICS DISCLAIMS ANY AND ALL RESPONSIBILITY OR LIABILITY
ARISING FROM OR RELATED TO ANY VULNERABILITY ISSUES. FURTHERMORE, TO THE EXTENT PERMITTED BY APPLICABLE LAW, RENESAS ELECTRONICS DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, WITH RESPECT TO THIS DOCUMENT
AND ANY RELATED OR ACCOMPANYING SOFTWARE OR HARDWARE, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 

8. When using Renesas Electronics products, refer to the latest product information (data sheets, user's manuals, application notes, "General Notes for Handling and Using Semiconductor Devices" in
the reliability handbook, etc.), and ensure that usage conditions are within the ranges specified by Renesas Electronics with respect to
maximum ratings, operating power supply voltage range, heat dissipation characteristics, installation, etc. Renesas Electronics disclaims any
and all liability for any malfunctions, failure or accident arising out of the use of Renesas Electronics products outside of such specified
ranges. 

9. Although Renesas Electronics endeavors to improve the quality and reliability of Renesas Electronics products, semiconductor products
have specific characteristics, such as the occurrence of failure at a certain rate and malfunctions under certain use conditions. Unless
designated as a high reliability product or a product for harsh environments in a Renesas Electronics data sheet or other Renesas
Electronics document, Renesas Electronics products are not subject to radiation resistance design. You are responsible for implementing safety
measures to guard against the possibility of bodily injury, injury or damage caused by fire, and/or danger to the public in the event of a
failure or malfunction of Renesas Electronics products, such as safety design for hardware and software, including but not limited to
redundancy, fire control and malfunction prevention, appropriate treatment for aging degradation or any other appropriate measures.
Because the evaluation of microcomputer software alone is very difficult and impractical, you are responsible for evaluating the safety of the
final products or systems manufactured by you. 

10. Please contact a
Renesas Electronics sales office for details as to environmental matters such as the environmental compatibility of each Renesas Electronics
product. You are responsible for carefully and sufficiently investigating applicable laws and regulations that regulate the
inclusion or use of controlled substances, including without limitation, the EU RoHS Directive, and using Renesas Electronics products in
compliance with all these applicable laws and regulations. Renesas Electronics disclaims any and all liability for damages or losses
occurring as a result of your noncompliance with applicable laws and regulations. 

11. Renesas Electronics products and technologies shall not be used for or incorporated into any products or systems whose
manufacture, use, or sale is prohibited under any applicable domestic or foreign laws or regulations. You shall comply with any applicable export
control laws and regulations promulgated and administered by the governments of any countries asserting jurisdiction over the parties or
transactions. 

12. It is the responsibility of the buyer or distributor of Renesas Electronics products, or any other party who distributes,
disposes of, or otherwise sells or transfers the product to a third party, to notify such third party in advance of the contents and
conditions set forth in this document. 

13. This document shall not be
reprinted, reproduced or duplicated in any form, in whole or in part, without prior written consent of Renesas Electronics. 

14. Please contact a Renesas Electronics sales office if you have any questions regarding the information contained in this document or Renesas Electronics
products. (Note1) "Renesas Electronics" as used in this document means Renesas Electronics Corporation and also includes its directly or
indirectly controlled subsidiaries. (Note2) "Renesas Electronics product(s)" means any product developed or manufactured by or for
Renesas Electronics.

                                                                                   (Rev.5.0-1 October 2020)
## Corporate Headquarters 

Contact information TOYOSU FORESIA, 3-2-24

Toyosu, Koto-ku, Tokyo 135-0061, Japan 

www.renesas.com 

## Contact information 

For further information on a product, technology, the most up-to-date version of a
document, or your nearest sales office, please visit:
www.renesas.com/contact/. 

## Trademarks 
Renesas and the Renesas logo are trademarks of Renesas Electronics Corporation. All trademarks and
registered trademarks are the property of their respective owners.

							© 2026 Renesas Electronics Corporation. All rights reserved
