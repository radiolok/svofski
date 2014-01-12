/*
 * @brief USB Mass Storage Device Example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * Copyright(C) Dean Camera, 2011, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "MassStorage.h"

#include "uartbridge.h"
#include "xprintf.h"
#include "cartucho.h"
#include "DataRam.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/** nxpUSBlib Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
static USB_ClassInfo_MS_Device_t Disk_MS_Interface = {
	.Config = {
		.InterfaceNumber           = 0,

		.DataINEndpointNumber      = MASS_STORAGE_IN_EPNUM,
		.DataINEndpointSize        = MASS_STORAGE_IO_EPSIZE,
		.DataINEndpointDoubleBank  = false,

		.DataOUTEndpointNumber     = MASS_STORAGE_OUT_EPNUM,
		.DataOUTEndpointSize       = MASS_STORAGE_IO_EPSIZE,
		.DataOUTEndpointDoubleBank = false,

		.TotalLUNs                 = TOTAL_LUNS,
		.PortNumber = 0,
	},
};


/* HW set up function */
static void SetupHardware(void)
{
	Board_Init();

    UART0_Init(230400);
    NVIC_EnableIRQ(UART0_IRQn);
	xprintf("\n\n\nVectrexador\n");
	xprintf("\n\n\nVectrexador2\n");

#ifndef SERIALDEBUG	
	dflihdfgkjl
    NVIC_DisableIRQ(UART0_IRQn);
	UART0_UnInit();
	VectrexBusInit();
#endif	

	USB_Init(Disk_MS_Interface.Config.PortNumber, USB_MODE_Device);
}

volatile int USBUnplugged = 0;

volatile uint8_t* ROMBase;
extern const uint8_t Berzerk_vec[];
extern const int Berzerk_size;
extern const uint8_t BNZ17_BIN[];
extern const int BNZ17_BIN_size;
extern const uint8_t Mail_Plane_bin[];
extern const int Mail_Plane_bin_size;
extern const uint8_t cubedemo_vec[];
extern const int cubedemo_vec_size;
extern const uint8_t demo184_bin[];


void copyrom() {
	//ROMBase = (uint8_t *) &BNZ17_BIN[0];
	//ROMBase = (uint8_t *) &demo184_bin[0];
	//ROMBase = DataRam_GetDataPtr();
	//memcpy(DataRam_GetDataPtr(), Berzerk_vec, Berzerk_size);
	//memcpy(DataRam_GetDataPtr(), BNZ17_BIN, BNZ17_BIN_size);
	//memcpy(DataRam_GetDataPtr(), Mail_Plane_bin, Mail_Plane_bin_size);
	//memcpy(DataRam_GetDataPtr(), cubedemo_vec, cubedemo_vec_size);
}

#define WITH_USB

/**
 *  @brief  Main program entry point
 *  @return Will never return
 *  @note   This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

#ifdef WITH_USB	

	xprintf("\nLet's see some usb stuff");

	for(;;) {
		// initially expect USB connection
		for (; !USBUnplugged; ) {
			#if !defined(USB_DEVICE_ROM_DRIVER)
			MS_Device_USBTask(&Disk_MS_Interface);
			USB_USBTask(Disk_MS_Interface.Config.PortNumber, USB_MODE_Device);
			#endif
		}

		// after USB is unplugged, serve the vectrex bus
		VectrexBusSlave();

		// forever, but who knows?
	}
#endif
	VectrexBusInit();
	copyrom();
	//for(;;);
	VectrexBusSlave();	
}

/**
 * @brief  Event handler for the library USB Connection event
 * @return Nothing
 */
void EVENT_USB_Device_Connect(void)
{}

/**
 * @brief Event handler for the library USB Disconnection event
 * @return Nothing
 */
void EVENT_USB_Device_Disconnect(void)
{
	USBUnplugged = 1;
}

/**
 * @brief Event handler for the library USB Configuration Changed event
 * @return Nothing
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);
}

/**
 * @brief Event handler for the library USB Control Request reception event
 * @return	Nothing
 */
void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
}

/**
 * @brief Mass Storage class driver callback function
 * @return Nothing
 * @note   The reception of SCSI commands from the host, which must be processed
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t *const MSInterfaceInfo)
{
	bool CommandSuccess;

	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);
	return CommandSuccess;
}
