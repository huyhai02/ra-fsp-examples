/**********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions
**********************************************************************************************************************/

1. Project Overview:
	This project demonstrates the basic functionalities of USB Peripheral Vendor (USB PVND) class driver on Renesas
	RA MCUs based on Renesas FSP. In this example, the application will configure the MCU as a Vendor Peripheral
	device. This Peripheral device will be connected to the USB Host Vendor (USB HVND) device which is another RA
	board. After the enumeration is completed, the Vendor Peripheral board will read 15 Bytes of data from the Vendor
	Host board and write back the same data to the Vendor Host board. The Vendor Host and Vendor Peripheral
	applications are designed to continuously transfer data between both the boards. User will be able to see all
	the operation sequence and status on J-Link RTT Viewer.

2. Supported Boards:
	Supported RA boards: EK-RA2A1, EK-RA4M1, EK-RA4M2, EK-RA4M3, EK-RA6M1, EK-RA6M2, EK-RA6M3, EK-RA6M3G, EK-RA6M4,
			     EK-RA6M5.

	Supported USB modes (PVND/HVND) depend on the specific board, as shown in the table below:
		+-------------+-------------+--------------+
		| Board       | USB PVND    | USB HNVD     |
		+-------------+-------------+--------------+
		| EK-RA2A1    | Support     | Not support  |
		+-------------+-------------+--------------+
		| EK-RA4M1    | Support     | Not support  |
		+-------------+-------------+--------------+
		| EK-RA4M2    | Support     | Support      |
		+-------------+-------------+--------------+
		| EK-RA4M3    | Support     | Support      |
		+-------------+-------------+--------------+
		| EK-RA6M1    | Support     | Not support  |
		+-------------+-------------+--------------+
		| EK-RA6M2    | Support     | Not support  |
		+-------------+-------------+--------------+
		| EK-RA6M3    | Support     | Support      |
		+-------------+-------------+--------------+
		| EK-RA6M3G   | Support     | Support      |
		+-------------+-------------+--------------+
		| EK-RA6M4    | Support     | Support      |
		+-------------+-------------+--------------+
		| EK-RA6M5    | Support     | Support      |
		+-------------+-------------+--------------+

3. Hardware Requirements:
	2 x Supported RA boards:
		1 x RA board (e.g., EK-RA4M1) runs the USB PVND example project.
		1 x RA board (e.g., EK-RA4M2) runs the USB HVND example project.
	1 x USB OTG cable.
	3 x USB cables.
		2 x USB cables for programming and debugging.
		1 x USB cable used to connect the RA board 1 to the RA board 2 through the USB OTG cable.

4. Configuration changes to switch Speed in configurator:
    i.  Full-Speed: (EK-RA2A1, EK-RA4M1, EK-RA4M2, EK-RA4M3, EK-RA6M1, EK-RA6M2, EK-RA6M4, EK-RA6M5)
        USB Speed                     :    Full Speed
        USB Module Number             :    USB_IP0 Port

    ii. High-Speed: (EK-RA6M3, EK-RA6M3G)
        USB Speed                     :    Hi Speed
        USB Module Number             :    USB_IP1 Port

5. Hardware Connections:
	- Connect RA board 1 running the USB PVND example project to RA board 2 running the USB HVND example project
	  using a USB OTG cable.
	- Connect USB debug ports of the two RA boards to USB ports of the host PC using two USB cables.

	For EK-RA2A1, EK-RA4M1, EK-RA6M1, EK-RA6M2 (Full-Speed):
		- Connect micro-AB USB Full-Speed port (J9) of the Board1 to Board2 via a micro USB cable
		  through a USB OTG cable.

	For EK-RA4M2, EK-RA4M3, EK-RA6M4, EK-RA6M5 (Full-Speed):
		- Jumper J12 placement is pins 2-3.
		- Connect jumper J15 pins.
		- Connect micro-AB USB Full-Speed port (J11) of the Board1 to Board2 via a micro USB cable
		  through a USB OTG cable.

	For EK-RA6M3, EK-RA6M3G (High-Speed):
		- Jumper J7 placement is pins 2-3.
		- Connect jumper J17 pins.
		- Connect micro-AB USB High-Speed port (J6) of the Board1 to Board2 via a micro USB cable
		  through OTG cable.
