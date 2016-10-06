/**\file eee_hal.c
 *	\brief <b>This file contains the Hardware Abstraction Layer (HAL) for all components used on the EEE 100BaseTX MISL Stack Layer. Contains functions for reading, writing, and erasing the Microchip 25AA1024 EEPROM and the Micrel KSZ8895MLUB Ethernet Controller. When reading or writing data larger than the size of a single sector (8-bits), it is HIGHLY RECOMMENDED that you use the functions that are labelled "BulkRead" or "BulkWrite".</b>
 *
 *  Created on: May 20, 2016 <br>
 *      Copyright (c) 2016 Christopher R. Miller, Kevin Schmidgall, William Nault, Colton Schimank, Mike Willey,
 *      Dr. Joseph Morgan and the Mobile Integrated Solutions Laboratory <br>
 *
 *      Permission is hereby granted, free of charge, to any person obtaining a
 *      copy of this software and associated documentation files (the "Software"),
 *      to deal in the Software without restriction, including without limitation
 *      the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *      and/or sell copies of the Software, and to permit persons to whom the Software
 *      is furnished to do so, subject to the following conditions:
 *
 *      The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 *      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *      WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *
 ******************************************************************************************************************************************/


#include <stdbool.h>
#include <stdint.h>
#include "eee_hal.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "gpio.h"
#include "pin_map.h"
#include "rom.h"
#include "ssi.h"
#include "sysctl.h"
#include "uart.h"
#include "uartstdio.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "event_logger.h"


//*****************************************************************************
//
//! FreeRTOS FIFO Queue. Allows read/write events to be logged to the EEPROM.
//! The only items that should be passed to this queue are of the LoggerCodes
//! enumeration found in event_logger.h.
//
//*****************************************************************************
extern xQueueHandle g_pLoggerQueue;
//*****************************************************************************
//
//! FreeRTOS mutex that forces concurrently running tasks to request access
//! to the EEPROM's SPI port before sending/reading information. This prevents
//! any two or more tasks from interfering with an ongoing transaction.
//
//*****************************************************************************
extern xSemaphoreHandle g_pSPI0Semaphore;
//*****************************************************************************
//
//! FreeRTOS mutex that forces concurrently running tasks to request access
//! to the Ethernet Controller's SPI port before sending/reading information.
//! This prevents any two or more tasks from interfering with an ongoing
//! transaction.
//
//*****************************************************************************
extern xSemaphoreHandle g_pSPI1Semaphore;


//*****************************************************************************
//
//! Pauses execution of the microcontroller for the time specified in ui32Ms in
//! milliseconds by converting ui32Ms to MCU ticks. This function blocks all
//! other running tasks for the time specified. [USE SPARINGLY].
//!
//! \param ui32Ms the number of milliseconds to pause the MCU
//
//*****************************************************************************
void delayMs(uint32_t ui32Ms) {

	// 1 clock cycle = 1 / SysCtlClockGet() second
	// 1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
	// 1 second = SysCtlClockGet() / 3
	// 0.001 second = 1 ms = SysCtlClockGet() / 3 / 1000

	ROM_SysCtlDelay(ui32Ms * (SysCtlClockGet() / 3 / 1000));
}
//*****************************************************************************
//
//! Pauses execution of the microcontroller for the time specified in ui32Us in
//! microseconds by converting ui32Us to MCU ticks. This function blocks all
//! other running tasks for the time specified. [USE SPARINGLY].
//!
//! \param ui32Us the number of microseconds to pause the MCU
//
//*****************************************************************************
void delayUs(uint32_t ui32Us) {
	ROM_SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}

//************EEPROM READ/WRITE FUNCTIONS FOR MICREL 25AA1024****************************
//
// The Micrel 25AA1024 is a 17 bit addressable (2^17 = 131,072) serial EEPROM that uses
// an 8-bit instruction register to perform various operations on the memory
// array housed inside it. Each address in EEPROM holds a single byte.
// The following function are a wrapper around the TivaWare SSI libraries and allow for
// easy read/write operations using this device
//
//***************************************************************************************


//*****************************************************************************
//
//! Writes a single 8 bit value to a register within the EEPROM at the specified
//! address. This function does not handle page operations.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 17-bit address in EEPROM to write to
//! \param data the 8-bit data to save to EEPROM
//!
//! This function takes the address provided by the user and accompanying data
//! and performs the appropriate timing to not only store the information, but
//! also read back the information to ensure that the value has been properly
//! saved.
//!
//! \note Only the last 24 bits of information in the address variable are kept
//! for use with this chip. Of those 24, only 17 can be used to reference a
//! memory address as this chip only has a 131,072 sectors. Each sector stores
//! a single byte of information.
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMSingleWrite(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t address, uint8_t data)
{
    uint32_t WRITE_ENABLE 	= 0x00000006;
    uint32_t WRITE_COMMAND 	= 0x00000002;
    uint32_t READ_COMMAND 	= 0x00000003;
    uint32_t ACTIVE_LOW 	= 0x00000000;
    uint32_t READ_DATA;

	LogItemEEPROM(EEPROMWriteOP);

	xSemaphoreTake(g_pSPI0Semaphore,0);
    //Invert data to write, zeros in EEPROM indicate set bits
    data = ~data;

    //Set Write Enable Latch
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(3);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, WRITE_ENABLE);
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Wait for the SSI Port to finish all communication before bringing CS high
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(1);


    //Write data to the appropriate register in the EEPROM
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, WRITE_COMMAND);
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Shift data by 16 bits and mask everything but the last octet
    SSIDataPut(SSI_BASE, ((address >> 16) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Shift data by 8 bits and mask everything but the last octet
    SSIDataPut(SSI_BASE, ((address >> 8) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Mask everything but the last octet
    SSIDataPut(SSI_BASE, ((address) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Send our 8-bit data out the SSITX Pin
    SSIDataPut(SSI_BASE, data);
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Wait for the SSI Port to finish all communication before bringing CS high
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    //25AA1024 requires a finite amount of time to peform an automated erase of the sector
    delayMs(8);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(3);
    //Read our data back from the same address and verify it matches
    SSIDataPut(SSI_BASE, READ_COMMAND);
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, ((address >> 16) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, ((address >> 8) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, ((address) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, 0x00000000);
    //Wait for the SSI Port to finish all communication before bringing CS high
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    SSIDataGet(SSI_BASE, &READ_DATA);

    if (data == READ_DATA)
    {
    	xSemaphoreGive(g_pSPI0Semaphore);
    	return true;
    }
    else {
    	xSemaphoreGive(g_pSPI0Semaphore);

    	LogItemEEPROM(EEPROMIOException);

    	return false;
    }
}

//*****************************************************************************
//
//! Erases the 25AA1024 memory area completely.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMChipErase(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN)
{
    uint32_t WRITE_ENABLE 			= 0x00000006;

    uint32_t ERASE_COMMAND 			= 0x000000C7;
    uint32_t ACTIVE_LOW 			= 0x00000000;
    uint32_t READ_DATA;
    //Set Write Enable Latch

	xSemaphoreTake(g_pSPI0Semaphore,0);

    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(3);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, WRITE_ENABLE);
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Wait for the SSI Port to finish all communication before bringing CS high
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(1);

    //Write chip erase command to the EEPROM
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, ERASE_COMMAND);
    SSIDataGet(SSI_BASE, &READ_DATA);
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    //25AA1024 requires a finite amount of time to peform an automated erase of the sector
    delayMs(8);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(3);

	xSemaphoreGive(g_pSPI0Semaphore);
    return true;
}
//*****************************************************************************
//
//! Erases a page on the 25AA1024 (256 bytes)
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the page [(0x00 - 0xFFFFF) % 256] to erase. Pages are 256 bytes!
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMPageErase(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t address)
{
    uint32_t WRITE_ENABLE 			= 0x6;
    uint32_t READ_STATUS			= 0x5;
    uint32_t ERASE_COMMAND 			= 0x42;
    uint32_t ACTIVE_LOW 			= 0x0;
    uint32_t DUMMY_DATA				= 0x0;
    uint32_t READ_DATA;
    //Set Write Enable Latch

	xSemaphoreTake(g_pSPI0Semaphore,0);

    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(3);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, WRITE_ENABLE);
    SSIDataGet(SSI_BASE, &READ_DATA);
    //Wait for the SSI Port to finish all communication before bringing CS high
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(1);

    //Write chip erase command to the EEPROM
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, ERASE_COMMAND);
    SSIDataGet(SSI_BASE, &READ_DATA);
    while(SSIBusy(SSI_BASE));
    SSIDataPut(SSI_BASE, address);
    SSIDataGet(SSI_BASE, &READ_DATA);
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    //25AA1024 requires a finite amount of time to peform an automated erase of the sector
    delayMs(10);

    READ_DATA = 0x01;

    while ((READ_DATA & 0x01) != 0x00) {
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(1);
    SSIDataPut(SSI_BASE, READ_STATUS);
    SSIDataGet(SSI_BASE, &READ_DATA);
    while(SSIBusy(SSI_BASE));
    SSIDataPut(SSI_BASE, DUMMY_DATA);
    SSIDataGet(SSI_BASE, &READ_DATA);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    }

	xSemaphoreGive(g_pSPI0Semaphore);
    return true;
}
//*****************************************************************************
//
//! Reads a single 8 bit value to a register within the EEPROM at the specified
//! address. This function does not handle page operations.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 17-bit address in EEPROM to write to
//!
//! This function obtains the 8 bits held at the address specified by the user
//! from the 25AA1024 EEPROM.
//!
//! \note Only the last 24 bits of information in the address variable are kept
//! for use with this chip. Of those 24, only 17 can be used to reference a
//! memory address as this chip only has a 131,072 sectors. Each sector stores
//! a single byte of information.
//!
//! \return Returns the value read from EEPROM as a unsigned integer
//
//*****************************************************************************
uint8_t EEPROMSingleRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t address)
{
    uint32_t READ_COMMAND 	= 0x00000003;
    uint32_t ACTIVE_LOW 	= 0x00000000;
    uint32_t READ_DATA;

	LogItemEEPROM(EEPROMReadOP);

	xSemaphoreTake(g_pSPI0Semaphore,0);

    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    delayUs(3);
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, ACTIVE_LOW);
    delayUs(3);
    //Read our data back from the same address and verify it matches
    SSIDataPut(SSI_BASE, READ_COMMAND);
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, ((address >> 16) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, ((address >> 8) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, ((address) & 0x000000FF));
    SSIDataGet(SSI_BASE, &READ_DATA);
    SSIDataPut(SSI_BASE, 0x00000000);
    //Wait for the SSI Port to finish all communication before bringing CS high
    while(SSIBusy(SSI_BASE));
    GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
    SSIDataGet(SSI_BASE, &READ_DATA);

	xSemaphoreGive(g_pSPI0Semaphore);

    return (~READ_DATA);
}

//*****************************************************************************
//
//! Writes an array of 8-bit values to the EEPROM chip and handles overflow
//! to another page.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param start_address the 17-bit starting address in EEPROM to write to
//! \param data pointer-to-array of 8-bit values to write sequentially
//! \param array_length number of values held in the array. This value should be non-zero.
//!
//! This function takes the address provided by the user and accompanying data
//! and performs write operations for each 8-bit value in the given array. If
//! the next address were to overflow to another page, this function provides
//! safe methods for dealing with this.
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMBulkWrite(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t start_address, uint8_t *data, uint32_t array_length)
{
	int pos = 0;

	LogItemEEPROM(EEPROMWriteOP);

	xSemaphoreTake(g_pSPI0Semaphore,0);

	if (start_address >= 131072) {
		//User provided value outside range of addressable memory
		LogItemEEPROM(EEPROMIOException);
		return false;
	}

	for (pos = 0; pos < array_length; pos++) {
		if (start_address >= 131072) {
			//Iterated beyond addressable memory
			xSemaphoreGive(g_pSPI0Semaphore);

			LogItemEEPROM(EEPROMIOException);

			return false;
		}
		if (EEPROMSingleWrite(SSI_BASE, CS_PORT_BASE, CS_PIN, start_address, (data[pos])) == 0) {
			xSemaphoreGive(g_pSPI0Semaphore);

			LogItemEEPROM(EEPROMIOException);

			while (1) {
				// We encountered a bad write, notify the developer.
			}
		}
		start_address++;
	}

	xSemaphoreGive(g_pSPI0Semaphore);

	return true;
}

//*****************************************************************************
//
//! Reads an array of 8-bit values to the EEPROM chip and handles overflow
//! to another page.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param start_address the 17-bit starting address in EEPROM to write to
//! \param output pointer-to-array of 8-bit values to save results into
//! \param array_length number of values held in the array. This value should be non-zero.
//!
//! This function takes the address provided by the user and accompanying data
//! and performs read operations for each address up to the array size. If
//! the next address were to overflow to another page, this function provides
//! safe methods for dealing with this.
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMBulkRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t start_address, uint8_t *output, uint32_t array_length)
{
	int pos = 0;

	LogItemEEPROM(EEPROMReadOP);

	xSemaphoreTake(g_pSPI0Semaphore,0);

	if (start_address >= 131072) {
		//User provided value outside range of addressable memory
		xSemaphoreGive(g_pSPI0Semaphore);

		LogItemEEPROM(EEPROMIOException);

		return false;
	}

	for (pos = 0; pos < array_length; pos++) {
		if (start_address >= 131072) {
			//Iterated beyond addressable memory
			xSemaphoreGive(g_pSPI0Semaphore);

			LogItemEEPROM(EEPROMIOException);

			return false;
		}
		output[pos] = EEPROMSingleRead(SSI_BASE, CS_PORT_BASE, CS_PIN, start_address);
		start_address++;
	}

	xSemaphoreGive(g_pSPI0Semaphore);
	return true;
}

//*****************************************************************************
//
//! Reads a status register from the Ethernet Controller over SPI.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 8-bit address in the Ethernet Controller to read from
//!
//! This function takes the address provided by the user and performs the timing
//! needed to aquire the status information inside the specified register on a
//! MICREL KSZ8895MQX Ethernet Controller.
//!
//! \return Returns the data held at the specified register as an unsigned integer
//
//*****************************************************************************
uint32_t EthoControllerSingleRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint8_t address)
{
	uint32_t READ_DATA;
	uint32_t READCOMMAND = 0x03;
	uint32_t DUMMYDATA = 0x00;

	LogItemEEPROM(EthoControllerReadOP);

	xSemaphoreTake(g_pSPI1Semaphore,0);

	//Pull status information for port 3 from register 0x39
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
	delayUs(3);
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, DUMMYDATA);
	delayUs(3);
	//Read our data back from the same address and verify it matches
	SSIDataPut(SSI_BASE, READCOMMAND);
	SSIDataGet(SSI_BASE, &READ_DATA);
	SSIDataPut(SSI_BASE, address);
	SSIDataGet(SSI_BASE, &READ_DATA);
	SSIDataPut(SSI_BASE, DUMMYDATA);
	//Wait for the SSI Port to finish all communication before bringing CS high
	while(SSIBusy(SSI_BASE));
	delayUs(3);
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
	SSIDataGet(SSI_BASE, &READ_DATA);

	xSemaphoreGive(g_pSPI1Semaphore);

	return READ_DATA;
}

//*****************************************************************************
//
//! Reads multiple status register from the Ethernet Controller over SPI.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param start_address the 8-bit address in the Ethernet Controller to begin reading from
//! \param count the number of registers from the starting address to read.
//! \param output pointer-to-array value that feeds the result of the function back to the calling thread
//!
//! This function takes the address provided by the user and performs the timing
//! needed to aquire the status information inside the specified register on a
//! MICREL KSZ8895MQX Ethernet Controller. It then proceeds to get all successive registers
//! from the device until the count parameter has been reached. This information is then returned
//! as a pointer-to-array of uint32_t values.
//!
//! \note Ensure that the array passed to the "output" parameter is defined as having at least the size of count
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Success)
//
//*****************************************************************************
bool EthoControllerBulkRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint8_t start_address, uint8_t count, uint32_t *output)
{
	uint32_t READ_DATA;
	uint32_t READCOMMAND = 0x03;
	uint32_t DUMMYDATA = 0x00;
	int i = 0;

	LogItemEEPROM(EthoControllerReadOP);

	xSemaphoreTake(g_pSPI1Semaphore,0);

	if ((start_address + count) > 255) {
		//User tried to request more results than are held in this 8-bit device.
		return false;
	}
	for (i = 0; i < count; i++) {
	//Pull status information for port 3 from register 0x39
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
	delayUs(3);
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, DUMMYDATA);
	delayUs(3);
	//Read our data back from the same address and verify it matches
	SSIDataPut(SSI_BASE, READCOMMAND);
	SSIDataGet(SSI_BASE, &READ_DATA);
	SSIDataPut(SSI_BASE, (start_address + i));
	SSIDataGet(SSI_BASE, &READ_DATA);
	SSIDataPut(SSI_BASE, DUMMYDATA);
	//Wait for the SSI Port to finish all communication before bringing CS high
	while(SSIBusy(SSI_BASE));
	delayUs(3);
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
	SSIDataGet(SSI_BASE, &READ_DATA);

	output[i] = READ_DATA;

	}

	xSemaphoreGive(g_pSPI1Semaphore);

	return true;
}

//*****************************************************************************
//
//! Writes a status register from the Ethernet Controller over SPI.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 8-bit address in the Ethernet Controller to read from
//! \param data the 8-bit data to save to the Ethernet Controller
//!
//! This function takes the address provided by the user and performs the timing
//! needed to write the status information inside the specified register on a
//! MICREL KSZ8895MQX Ethernet Controller.
//!
//! \return Returns the data held at the specified register as an unsigned integer
//
//*****************************************************************************
bool EthoControllerSingleWrite(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint8_t address, uint32_t data)
{
	uint32_t READ_DATA;
	uint32_t WRITECOMMAND = 0x02;
	uint32_t DUMMYDATA = 0x00;

	LogItemEEPROM(EthoControllerWriteOP);

	xSemaphoreTake(g_pSPI1Semaphore,0);

	//Pull status information for port 3 from register 0x39
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
	delayUs(3);
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, DUMMYDATA);
	delayUs(3);
	//Read our data back from the same address and verify it matches
	SSIDataPut(SSI_BASE, WRITECOMMAND);
	SSIDataGet(SSI_BASE, &READ_DATA);
	SSIDataPut(SSI_BASE, address);
	SSIDataGet(SSI_BASE, &READ_DATA);
	SSIDataPut(SSI_BASE, data);
	//Wait for the SSI Port to finish all communication before bringing CS high
	while(SSIBusy(SSI_BASE));
	delayUs(3);
	GPIOPinWrite(CS_PORT_BASE, CS_PIN, CS_PIN);
	SSIDataGet(SSI_BASE, &READ_DATA);

	xSemaphoreGive(g_pSPI1Semaphore);

	return true;
}
