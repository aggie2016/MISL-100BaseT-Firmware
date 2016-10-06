/**\file command_functions.c
 * \brief <b>All of the functions called by the Command Line Interface. Additional functions for the CLI or I2C should be placed in this file.</b>
 *
 *
 *	This file contains all functions called at run-time by either CLI or I2C commands. All functions that are intended to be used by the command line
 *	adopt the format COM_<Function Name>, take an array of pointers-to-char, and return a boolean as the result of execution. This format must be
 *	followed in order to avoid incompatibility with	the UART interpreter task (described later in this document). Each function is described below
 *
 *	[1.1] -- Structure of New Commmands -- <br>
 *	If you wish provide additional functionality to the command line interface, all new functions must follow the syntax outlined below. <br>
 *	Command Line Functions: <br>
 *		Each function takes an array of pointer-to-char of size 20 (as defined by MAX_PARAMS) and returns a bool as the result of execution.
 *	\code
 *		bool COM_<FunctionName>(char *params[MAX_PARAMS])
 *		{
 *		    return true (or false);
 *		}
 *	\endcode
 *	I2C Functions: <br>
 *		Unlike functions used by the CLI, I2C commands are formatted to accept and return 8-bit values for use over SCL and SDA lines. This
 *		allows the information recieved to the easily processed since all values recieved over I2C are done so using an 8-bit packet size.
 *		The commonly used return value for successful and unsuccesful commands is the same as that defined for boolean true and false
 *		(0x01: Successful, 0x00: Failed). Any additional information that you wish to return to the I2C master must be done so within
 *		the scope of the function itself.
 *	\code
 *		uint8_t I2C_<FunctionName>(uint8_t params[MAX_PARAMS])
 *		{
 *		    return true;
 *		}
 *	\endcode
 *
 *	[1.2] -- Playing Nicely with FreeRTOS -- <br>
 *	Since all of the executed code in this firmware (after boot) utitlizes FreeRTOS task, the use of long processing loops within the command-line
 *	is highly discouraged. Instead, inside long-running processes (i.e. COM_SaveSwitchConfiguration), use the built-in FreeRTOS xTaskGetTickCount and
 *	vTaskDelayUntil functions to briefly return control back to the kernel to allow other tasks to run. The firmware provides three levels of task delay
 *	length that can be used to specify the amount of computation time that will be returned to the kernel: <br>
 *		- LONG_RUNNING_TASK_DLY (40 milliseconds) <br>
 *		- SHORT_TASK_DLY	(10 milliseconds) <br>
 *		- VERY_SHORT_TASK_DLY	(5  milliseconds) <br>
 *	<br>
 *	[1.3] -- Displaying Real-Time Progress -- <br>
 *	EEE Firmware v1.1.12 includes a brief set of functions that display a progress bar during the execution of a long-running task. To add a progress bar in
 *	lieu of standard notification text, use the CreateProgressBar function. This function will return a progress pointer that is used by the UpdateProgressBar
 *	functions. To increment the progress bar, pass the pointer along with the PBarAction parameter 'Increment'. No additional text should be printed to the command
 *	line while this bar is displaying progress. <br>
 *	<br>
 *	[1.4] -- Current CLI Functions -- <br>
 *	Commands currently supported by this version of the EEE 100BaseTX Firmware are listed below. For an expanded description of each function, refer directly
 *	to the source code or to the doxygen homepage stored under 'html/index.html'. <br>
 *		[1.4.1] COM_WriteToEEPROM <br>
 *		[1.4.2]	COM_SaveSwitchConfiguration <br>
 *		[1.4.3] COM_ReinitializeEEPROM <br>
 *		[1.4.4] COM_ReadFromEEPROM <br>
 *		[1.4.5] COM_ReadFromEthernetController <br>
 *		[1.4.6] COM_WriteToEthernetController <br>
 *		[1.4.7] COM_SetBitEthernetController <br>
 *		[1.4.8] COM_ClearBitEthernetController <br>
 *		[1.4.9] COM_SetSCBitEthernetController <br>
 *		[1.4.10] COM_RunCableDiagnostics <br>
 *		[1.4.11] COM_ShowPortStatus <br>
 *		[1.4.12] COM_ResetTivaC <br>
 *		[1.4.13] COM_ShowRunningConfig <br>
 *		[1.4.14] COM_DeleteUsersMenu <br>
 *		[1.4.15] COM_ListUsers <br>
 *		[1.4.16] COM_AddUser <br>
 *		[1.4.17] COM_EventStatus <br>
 *		[1.4.18] COM_ManageEvents <br>
 *		[1.4.19] COM_ListEvents <br>
 *		[1.4.20] COM_DeleteEvents <br>
 *		[1.4.21] COM_Logout <br>
 *		[1.4.22] COM_SetPortVLAN <br>
 *		[1.4.23] COM_EnableVLANS <br>
 *		[1.4.24] COM_DisableVLANS <br>
 *		[1.4.25] COM_DeleteConfig <br>
 *		[1.4.26] COM_SetVLANEntry <br>
 *		[1.4.27] COM_ShowVLANTable <br>
 *		[1.4.28] COM_ShowStaticMACTable <br>
 *		[1.4.29] COM_ShowDynamicMACTable <br>
 *		[1.4.30] CreateProgressBar <br>
 *		[1.4.31] UpdateProgressBar <br>
 * <br>
 *  Created on: May 20, 2016
 *      Copyright (c) 2016 Christopher R. Miller, Kevin Schmidgall, William Nault, Colton Schimank, Mike Willey,
 *      Dr. Joseph Morgan and the Mobile Integrated Solutions Laboratory
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
 ******************************************************************************************************************************************/
#include <eee_hal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "i2c.h"
#include "command_functions.h"
#include "event_logger.h"
#include "interpreter_task.h"
#include "freertos_init.h"
#include "i2c_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "string.h"

void ShowProgress(int percent);
uint32_t SetPortMembershipBits(uint32_t port_membership, uint32_t port_addr);
uint8_t AssertVLANS(uint32_t vlan_id, uint32_t port_id);
size_t FindIndex(uint8_t a[], size_t size, int value );

static bool ResetIssued = false;
extern xQueueHandle g_pLoggerQueue;

//*****************************************************************************
//
//! Write to EEEPROM (for Console Window)
//! Writes an 8-bit value to the register address specified and reports on the
//! success or failure of the operation to the user's command line interface.
//! \param params[0] 8-bit register address to write to
//! \param params[1] 8-bit register data to write
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_WriteToEEPOM(char *params[MAX_PARAMS]) {

	//Parameter 1: Register Address
	uint32_t reg_addr = (uint32_t)strtol(params[0],NULL,0);
	UARTprintf("REG ADDR: 0x%02x\n", reg_addr);
	//Parameter 2: Register Data (8-bits)
	uint32_t reg_data = (uint32_t)strtol(params[1],NULL,0);
	UARTprintf("REG DATA: 0x%02x\n", reg_data);
	UARTprintf("[RUNNING TASK]: Writing To EEPROM                                           \n");
	ShowProgress(50);
	if (EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, reg_addr, reg_data))
	{
		ShowProgress(100);
	}
	else {
		ShowProgress(-1);
	}

	UARTprintf("\033[0m");
	return true;
}

//*****************************************************************************
//
//! Read from EEEPROM (for Console Window)
//! Reads an 8-bit value from the register address specified and reports on the
//! success or failure of the operation to the user's command line interface.
//! \param params[0] 8-bit register address to read from
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ReadFromEEPROM(char *params[MAX_PARAMS])
{
	//Parameter 1: Register Address
	uint32_t reg_addr = (uint32_t)strtol(params[0],NULL,0);
	//Returned Data Stored Here
	uint32_t reg_data;
	UARTprintf("REG ADDR: 0x%08x\n", reg_addr);
	UARTprintf("[RUNNING TASK]: Reading From EEPROM                                           \n");
	ShowProgress(50);
	reg_data = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, reg_addr);
	ShowProgress(100);
	UARTprintf("\n Data Read @0x%02x: 0x%02x", reg_addr, reg_data);
	UARTprintf("\033[0m");
	return true;
}

//*****************************************************************************
//
//! Write/Read from Ethernet Controller (for I2C Commands)
//! Reads or writes an 8-bit value from the register address specified and
//! reports on the success or failure of the operation to the user over I2C.
//! \param params[0] 8-bit register address to write/read from
//! \param params[1] 8-bit register data to write (only applicable in write mode)
//! \param params[2] 1-bit boolean value (1: write operation, 0: read operation)
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_WriteReadFromEthernetController(uint8_t params[MAX_PARAMS])
{
	uint8_t reg_addr = params[0];
	uint8_t reg_data = params[1];
	bool write_op = (bool)params[2];
	if (write_op) {
		EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, reg_addr, reg_data);
	}
	else {
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, reg_addr);
	}
	return reg_data;
	//Read specified register from controller 1
}
//*****************************************************************************
//
//! Set A Bit Inside Ethernet Controller (for I2C Commands)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the I2C master.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active high
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_SetBitEthernetController(uint8_t params[MAX_PARAMS])
{
	 uint32_t reg_data = 0x00;

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)params[0];

	//Parameter 2: Register Offset Address (8-bits)
	uint32_t offset = (uint32_t)params[1];

	//Parameter 3: Bit to set (8-bits)
	uint32_t bit_to_set = (uint32_t)params[2];

	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + offset));
	reg_data |= 1 << bit_to_set;

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset), reg_data);
	return true;
}
//*****************************************************************************
//
//! Clears A Bit Inside Ethernet Controller (for I2C Commands)
//! Writes an active low value to the register address and bit specified and
//! reports on the success or failure of the operation to the I2C master.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active low
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_ClearBitEthernetController(uint8_t params[MAX_PARAMS])
{
	uint32_t reg_data = 0x00;
	portTickType ui32WakeTime;
	uint32_t retry_attempts = 0;

	//This task takes a while, so we want share nicely with other tasks instead of blocking
	uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	//Get system time in ticks
	ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)params[0];

	//Parameter 2: Register Offset Address (8-bits)
	uint32_t offset = (uint32_t)params[1];

	//Parameter 3: Bit to set (8-bits)
	uint32_t bit_to_set = (uint32_t)params[2];

	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + offset));
	reg_data &= ~(1 << bit_to_set);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset), reg_data);
	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			return false;
		}
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	return true;
}
//*****************************************************************************
//
//! Set Self-Clearning Bit Inside Ethernet Controller (for I2C Commands)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the I2C master after that
//! same bit returns to an active low state.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active low
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_SetSCEthernetController(uint8_t params[MAX_PARAMS])
{
    uint32_t reg_data = 0x00;
	 portTickType ui32WakeTime;
	 uint32_t retry_attempts = 0;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)params[0];

	//Parameter 2: Register Offset Address (8-bits)
	uint32_t offset = (uint32_t)params[1];

	//Parameter 3: Bit to set (8-bits)
	uint32_t bit_to_set = (uint32_t)params[2];

	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + offset));
	//Set bit to '1' and write
	reg_data |= 1 << bit_to_set;

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset), reg_data);
	//Reset bit to '0' and validate
	reg_data &= ~(1 << bit_to_set);

	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			return false;
		}
       vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	return true;
}
//*****************************************************************************
//
//! Run Cable Diagnostics For Specified Port (for I2C Commands)
//! Activates the Time Domain Reflectometry (TDR) function built into the ethernet
//! controller for the port specified and writes the state (Short/Open/Normal) and
//! distance to fault (in meters) back to the I2C master.

//! \param params[0] 8-bit base address of the port to test (see interpreter_task.h)
//!
//! \return Returns cable state (Short/Open/Normal) for further analysis
//
//*****************************************************************************
uint8_t I2C_RunCableDiagnostics(uint8_t params[MAX_PARAMS])
{
     uint32_t reg_data = 0, cable_state = 0;
	 portTickType ui32WakeTime;
	 uint32_t retry_attempts = 0;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)params[0];

	//Disable auto-negotiation
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX));

	//Set bit to '1' and write to disable auto-negotiation
	reg_data |= 1 << 0x07;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX), reg_data);

	//Disable auto-MDI/MDIX
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX));

	//Set bit to '1' and write to disable auto-negotiation
	reg_data |= 1 << 0x02;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX), reg_data);

	//LINKMD: Start Diagnostics
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX));

	//Set bit to '1' and write to start LinkMD
	reg_data |= 1 << 0x04;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX), reg_data);

	//Poll for completion of LinkMD process and obtain results
	reg_data &= ~(1 << 0x04);

	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			return false;
		}
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	//LinkMD finished


	//Get cable diagnostics results
	cable_state = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX));
	cable_state &= 0x60;

	//Enable auto-negotiation
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX));
	reg_data &= ~(1 << 0x07);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX), reg_data);

	//Enable AutoMDI/MDI-X
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX));
	reg_data &= ~(1 << 0x02);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX), reg_data);

	return cable_state;
}
//*****************************************************************************
//
//! Save Running Configuration to EEPROM (for I2C Commands)
//! Saves all registers from 0x00 to 0xFF inside the ethernet controller to
//! the corresponding EEPROM registers (0x100 - 0x1FF) an reports on the
//! success or failure of the operation.
//!
//! \return Returns the results of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_SaveSwitchConfiguration(uint8_t params[MAX_PARAMS])
{
	 uint32_t read_addr = 0x00;
	 uint8_t flag_data = 0x00;
	 uint32_t eeprom_eth0_addr = 0x100;
	 portTickType ui32WakeTime;
	 UARTprintf("\nI2C Requested Configuration Save\n%s>", console_hostname);
	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	 for (read_addr = 0; read_addr < 0xFF; read_addr++)
	{
		//Read data at address "read_addr" from Ethernet Controller 1
		uint8_t data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, read_addr);

		//Save this information to the next sector in EEPROM
		if (!EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, eeprom_eth0_addr, data)) {
			//We encountered a bad write cycle, report this to the user
			return false;
		}

		//Increment to the next sector in EEPROM
		eeprom_eth0_addr++;

		//Delay for 40ms to allow other tasks to run
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	flag_data = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, 0x0001E);
	flag_data |= 1 << 0x06;
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, 0x0001E, flag_data);
	return true;
}
//*****************************************************************************
//
//! Download Running Configuration (for I2C Commands)
//! Sequentially writes each value held in the Micrel KSZ8895MLUB's registers
//! 0x00 - 0xFF to the requesting I2C master.
//!
//! \return Returns the results of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_DownloadSwitchConfiguration(uint8_t params[MAX_PARAMS])
{
	 uint32_t read_addr = 0x00;
	 portTickType ui32WakeTime;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	 for (read_addr = 0; read_addr < 0xFF; read_addr++)
	{
		//Read data at address "read_addr" from Ethernet Controller 1
		uint8_t data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, read_addr);
		delayUs(I2C_SLAVE_SEND_DLY);
		//Send this information over I2C to the master
		I2CSlaveDataPut(I2C_BASE_ADDR,data);
		I2CMasterControl(I2C_BASE_ADDR, I2C_MASTER_CMD_SINGLE_RECEIVE);
		//Delay for 40ms to allow other tasks to run
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	return true;
}
//*****************************************************************************
//
//! Clear Running Configuration (for I2C Commands)
//! Sequentially writes a null (0x00) to each register in the Micrel 25AA1024
//! EEPROM from 0x100 - 0x1FF and reports the success or failure of the operation
//!
//! \return Returns the results of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_ClearSwitchConfiguration(uint8_t params[MAX_PARAMS])
{
	 uint32_t write_addr = 0x100;
	 uint8_t flag_data;
	 portTickType ui32WakeTime;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	 for (; write_addr < 0x1FF; write_addr++)
	{
		//Read data at address "read_addr" from Ethernet Controller 1
		EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, write_addr, 0x00);

		//Delay for 40ms to allow other tasks to run
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
		flag_data = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, 0x0001E);
		flag_data &= ~(1 << 0x0);
		EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, 0x0001E, flag_data);
	return true;
}
//*****************************************************************************
//
//! Read 8-bits From Ethernet Controller (for Command-Line Interface)
//! Aquires the 8-bit value held at the register address specified and returns
//! it to the user's command-line. This function is mainly used for diagnostics
//! purposes.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \param params[0] 8-bit register address inside the KSZ8895MLUB
//!
//! \return Success or failure of the operation as a boolean.
//
//*****************************************************************************
bool COM_ReadFromEthernetController(char *params[MAX_PARAMS])
{

	uint8_t reg_addr = (uint8_t)strtol(params[0],NULL,16);
	uint32_t reg_data;
	UARTprintf("REG ADDR: 0x%08x\n", reg_addr);
	UARTprintf("[RUNNING TASK]: Reading From Ethernet Controller 1 \n");
	ShowProgress(50);
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, reg_addr);
	ShowProgress(100);
	UARTprintf("\nData Read @ 0x%08x: 0x%08x", reg_addr, reg_data);
	UARTprintf("\033[0m");
	return true;
	//Read specified register from controller 1
}
//*****************************************************************************
//
//! Sets A Bit Inside Ethernet Controller (for Command-Line Interface)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the user's command-line.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active high
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetBitEthernetController(char *params[MAX_PARAMS])
{
     uint32_t reg_data = 0x00;
	 portTickType ui32WakeTime;
	 uint32_t retry_attempts = 0;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = LONG_RUNNING_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)strtol(params[0],NULL,0);

	//Parameter 2: Register Offset Address (8-bits)
	uint32_t offset = (uint32_t)strtol(params[1],NULL,0);

	//Parameter 3: Bit to set (8-bits)
	uint32_t bit_to_set = (uint32_t)strtol(params[2],NULL,0);

	//Parameter 4: Task Execution Text Printed to Command Line
	UARTprintf("[RUNNING TASK]: %s \n", params[3]);
	ShowProgress(30);

	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + offset));
	ShowProgress(60);
	reg_data |= 1 << bit_to_set;

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset), reg_data);

	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			ShowProgress(-1);
			UARTprintf("\033[0m");
			return false;
		}
		ShowProgress(40 + retry_attempts);
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	ShowProgress(100);
	UARTprintf("\033[0m");
	return true;
}
//*****************************************************************************
//
//! Clear A Bit Inside Ethernet Controller (for Command-Line Interface)
//! Writes an active low value to the register address and bit specified and
//! reports on the success or failure of the operation to the user's command-line.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active low
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ClearBitEthernetController(char *params[MAX_PARAMS])
{
	uint32_t reg_data = 0x00;
	portTickType ui32WakeTime;
	uint32_t retry_attempts = 0;

	//This task takes a while, so we want share nicely with other tasks instead of blocking
	uint32_t ui32TaskDelay = LONG_RUNNING_TASK_DLY;

	//Get system time in ticks
	ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)strtol(params[0],NULL,0);

	//Parameter 2: Register Offset Address (8-bits)
	uint32_t offset = (uint32_t)strtol(params[1],NULL,0);

	//Parameter 3: Bit to set (8-bits)
	uint32_t bit_to_set = (uint32_t)strtol(params[2],NULL,0);

	//Parameter 4: Task Execution Text Printed to Command Line
	UARTprintf("[RUNNING TASK]: %s \n", params[3]);
	ShowProgress(50);

	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + offset));

	reg_data &= ~(1 << bit_to_set);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset), reg_data);
	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			ShowProgress(-1);
			UARTprintf("\033[0m");
			return false;
		}
		ShowProgress(40 + retry_attempts);
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	ShowProgress(100);
	UARTprintf("\033[0m");
	return true;
}
//*****************************************************************************
//
//! Set Self-Clearing Bit Inside Ethernet Controller (for Command-Line Interface)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the user's command-line
//! once the specified bit has returned to its original active low state
//!
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to pend on
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetSCBitEthernetController(char *params[MAX_PARAMS])
{
     uint32_t reg_data = 0x00;
	 portTickType ui32WakeTime;
	 uint32_t retry_attempts = 0;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = LONG_RUNNING_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)strtol(params[0],NULL,0);

	//Parameter 2: Register Offset Address (8-bits)
	uint32_t offset = (uint32_t)strtol(params[1],NULL,0);

	//Parameter 3: Bit to set (8-bits)
	uint32_t bit_to_set = (uint32_t)strtol(params[2],NULL,0);

	//Parameter 4: Task Execution Text Printed to Command Line
	UARTprintf("[RUNNING TASK]: %s \n", params[3]);
	ShowProgress(30);

	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + offset));
	ShowProgress(60);

	//Set bit to '1' and write
	reg_data |= 1 << bit_to_set;

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset), reg_data);


	//Reset bit to '0' and validate
	reg_data &= ~(1 << bit_to_set);

	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + offset)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			ShowProgress(-1);
			UARTprintf("\033[0m");
			return false;
		}
		ShowProgress(40 + retry_attempts);
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	ShowProgress(100);
	UARTprintf("\033[0m");
	return true;
}
//*****************************************************************************
//
//! Run Cable Diagnostics For Specified Port (for Command-Line Interface)
//! Activates the Time Domain Reflectometry (TDR) function built into the ethernet
//! controller for the port specified and writes the state (Short/Open/Normal) and
//! distance to fault (in meters) back to the user's command-line
//!
//! \param params[0] 8-bit base address of the port to test (see interpreter_task.h)
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_RunCableDiagnostics(char *params[MAX_PARAMS]) {
     uint32_t reg_data = 0, fault_distance = 0, cable_state = 0;
	 portTickType ui32WakeTime;
	 uint32_t retry_attempts = 0;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = LONG_RUNNING_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	//Parameter 1: Register Base Address
	uint32_t reg_addr = (uint32_t)strtol(params[0],NULL,0);

	//Task Execution Text Printed to Command Line
	UARTprintf("[RUNNING TASK]: Running Link MD for selected port, please wait... \n");

	//Disable auto-negotiation
	ShowProgress(10);
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX));
	ShowProgress(15);

	//Set bit to '1' and write to disable auto-negotiation
	reg_data |= 1 << 0x07;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX), reg_data);
	ShowProgress(20);

	//Disable auto-MDI/MDIX
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX));
	ShowProgress(25);

	//Set bit to '1' and write to disable auto-negotiation
	reg_data |= 1 << 0x02;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX), reg_data);
	ShowProgress(30);

	//LINKMD: Start Diagnostics
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX));
	ShowProgress(35);

	//Set bit to '1' and write to start LinkMD
	reg_data |= 1 << 0x04;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX), reg_data);
	ShowProgress(40);

	//Poll for completion of LinkMD process and obtain results
	reg_data &= ~(1 << 0x04);

	while (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX)) != reg_data) {
		retry_attempts++;
		if (retry_attempts > 10) {
			ShowProgress(-1);
			UARTprintf("\033[0m");
			return false;
		}
		ShowProgress(40 + retry_attempts);
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	//LinkMD finished
	ShowProgress(55);

	//Get cable diagnostics results
	cable_state = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX));
	cable_state &= 0x60;
	ShowProgress(60);
	//Get distance to cable fault reading
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD0_OFFSET_HEX));
	reg_data &= 0x01;
	ShowProgress(70);
	reg_data += EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_LINKMD1_OFFSET_HEX));
	ShowProgress(80);
	fault_distance = (0.4 * (reg_data - 26));

	//Enable auto-negotiation
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX));

	reg_data &= ~(1 << 0x07);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL5_OFFSET_HEX), reg_data);

	//Enable AutoMDI/MDI-X
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX));

	reg_data &= ~(1 << 0x02);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (reg_addr + PORT_CONTROL6_OFFSET_HEX), reg_data);


	ShowProgress(100);
	switch (cable_state) {
	case 0x00:
		ShowProgress(100);
		UARTprintf("\n\tLINK CABLE: Normal\n");
		UARTprintf("\033[0m");
		return true;
	case 0x20:
		UARTprintf("\n\tLINK CABLE: Open Detected In Cable\n");
		break;
	case 0x40:
		UARTprintf("\n\tLINK CABLE: Short Detected In Cable\n");
		break;
	case 0x60:
		UARTprintf("\n\tLINK CABLE: Cable Diagnostics Failed\n");
		break;
	default:
		UARTprintf("n\tLINK CABLE: An unknown error occurred while testing\n");
		break;
	}
	UARTprintf("\tDISTANCE TO FAULT: %d", fault_distance);
	UARTprintf("\033[0m");
	return true;


}

//*****************************************************************************
//
//! Write 8-bit Value To Ethernet Controller (for Command-Line Interface)
//! Writes specified value to the register address specified and
//! reports on the success or failure of the operation to the user's command-line.
//!
//! \param params[0] 8-bit register address for KSZ8895MLUB
//! \param params[1] 8-bit data to write
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_WriteToEthernetController(char *params[MAX_PARAMS])
{

	uint8_t reg_addr = (uint8_t)strtol(params[0],NULL,16);
	UARTprintf("REG ADDR: 0x%08x\n", reg_addr);
	uint8_t reg_data = (uint8_t)strtol(params[1],NULL,16);

	//The new value for the specified register
	uint32_t read_data;

	UARTprintf("REG DATA: 0x%08x\n", reg_data);
	UARTprintf("[RUNNING TASK]: Writing To Ethernet Controller 1 \n");

	ShowProgress(50);

	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE,ETHO_1_SSI_CS_PIN, reg_addr, reg_data);

	//Read specified register from controller 1
	read_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, reg_addr);
	if (read_data == reg_data) {
		ShowProgress(100);
		UARTprintf("\nData Written To @ 0x%08x: 0x%08x", reg_addr, read_data);
	}
	else {
		ShowProgress(-1);
		UARTprintf("\033[0m");
		return false;
	}

	UARTprintf("\033[0m");

	return true;

}
//*****************************************************************************
//
//! Set Port Default VLAN Tag (for Command-Line Interface)
//! Writes to the appropriate registers on the KSZ8895MLUB to change the selected
//! port to the VLAN specified. Also turns on automatic tag insertion for all
//! frames that do not already have a VLAN specified (IMPORTANT!).
//!
//! \param params[0] 8-bit base port address (refer to "interpreter_task.h)
//! \param params[1] 12-bit VLAN ID to set this port to (MAX VLAN ID: 4096)
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetPortVLAN(char *params[MAX_PARAMS]) {
	//Parameter 1: Port Base Address
	uint32_t port_addr = (uint32_t)strtol(params[0],NULL,0);
	//Parameter 2: VLAN ID
	uint32_t vlan_id = (uint32_t)strtol(params[1],NULL,0);
	uint32_t reg_data = 0x00;

	//Turn on tag insertion for this port
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,(PORT_CONTROL0_OFFSET_HEX + port_addr));
	reg_data |= 1 << 2;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL0_OFFSET_HEX + port_addr), reg_data);

	//Set bits [3-0] of port control 3 register (VLAN bits [11-8])
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,(PORT_CONTROL3_OFFSET_HEX + port_addr));
	reg_data &= 0xF8;
	reg_data |= ((vlan_id & 0xE00) >> 8) << 0;
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL3_OFFSET_HEX + port_addr), reg_data);

	//Set bits [7-0] of port control 4 register (VLAN bits [7-0])
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL4_OFFSET_HEX + port_addr), (vlan_id & 0xFF));

	//Set port VLAN membership bits in port control register 1
	reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,(PORT_CONTROL1_OFFSET_HEX + port_addr));
	reg_data &= 0xE0;
	reg_data = ((AssertVLANS(vlan_id, port_addr) & 0x1F) << 0);
	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL1_OFFSET_HEX + port_addr), reg_data);

	return true;
}
//*****************************************************************************
//
//! Append VLAN to VLAN Table (for Command-Line Interface)
//! Writes the specified VLAN entry to the KSZ8895MLUB's indirectly accessible
//! VLAN table. Also scans adjacent ports for matching VLAN tags to update VLAN
//! table with new membership information ("AssertVLANS()").
//!
//! \param params[0] 8-bit base port address (refer to "interpreter_task.h)
//! \param params[1] 12-bit VLAN ID to add to VLAN Table (MAX VLAN ID: 4096)
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetVLANEntry(char *params[MAX_PARAMS]) {
	//Parameter 1: Port Base Address
	uint32_t port_addr = (uint32_t)strtol(params[0],NULL,0);
	//Parameter 2: VLAN ID
	uint32_t vlan_id = (uint32_t)strtol(params[1],NULL,0);

	if (vlan_id > 4095) {
		UARTprintf("VLAN entered is out of range. Valid options are 1 - 4095");
		return false;
	}
	uint8_t port_membership = 0x00;

	uint32_t indirect_reg_addr = (vlan_id / 4), indirect_reg_data = 0x00;
	//Obtain current indirect access data 0 and modify
	uint32_t indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0);
	//Select VLAN table, set operation mode to READ, and set 2 bits from MSB of indirect_reg_addr in bits 0 and 1 of this register
	indirect_access_data |= (INDIRECT_TABLESELECT_VLAN << INDIRECT_CONTROL_TABLESELECT) | (INDIRECT_READTYPE_READ << INDIRECT_CONTROL_READTYPEBIT) | (((indirect_reg_addr >> 8) & 0xFF) << INDIRECT_CONTROL_ADDRESS_HIGH);
	//Write the modified register to the Ethernet Controller
	if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0, indirect_access_data))
	{
		return false;
	}
	//Obtain current indirect access data 1 and modify
	indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_1);
	//Set this register to the remaining 8 bits of the 10-bit indirect_reg_addr
	indirect_access_data = (indirect_reg_addr & 0xFF);
	if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1, indirect_access_data))
	{
		return false;
	}

	//VLAN SETTINGS MODIFICATION START HERE
	/*Each indirect register contains informatin for four VLANs organized in the following way:
	* ENTRY 0 (12 bits) maps to first VLAN, indirect bits (12-0): register 119 (bits 4-0), register 120 (bits 7-0)
	* ENTRY 1 (12 bits) maps to second VLAN, indirect bits (25-13): register 117 (bits 1-0), register 118 (bits 7-0), register 119 (bits 7-5)
	* ENTRY 2 (12 bits) maps to third VLAN, indirect bits (38-26): register 116 (bits 6-0), register 117 (7-2)
	* ENTRY 3 (12 bits) maps to fourth VLAN, indirect bits (51-39): register 114 (bits 3-0), register 115 (7-0), register 116 (bit 7);
	*
	* VLAN GROUP = (VLAN_ID / 4)
	* VLAN = VLAN GROUP % 4
	*/
	switch (vlan_id % 4) {
	//User requested a change to the first VLAN in GROUP X
	case 0:
		//Get the information held in register 119
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_1);
		//Set VLAN entry to VALID
		indirect_reg_data |= (1 << 4);

		//Read current port membership data
		port_membership = (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_1) & 0xF) + (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_0) >> 7);
		//Set bit corresponding to currently selected port
		port_membership = AssertVLANS(vlan_id, port_addr); //SetPortMembershipBits(port_membership, port_addr);

		//Place first four bits of port_membership value into bits [3-0] of indirect register 119
		indirect_reg_data |= ((port_membership >> 1 ) << 0);
		//Overwrite register 119 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_1, indirect_reg_data))
		{
			return false;
		}
		//Get the information held in register 120
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_0);
		//Place the fifth bit into bit 7 of indirect register 120
		indirect_reg_data |= ((port_membership & 0x01) << 7);
		//Overwrite register 120 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_0, indirect_reg_data))
		{
			return false;
		}
		break;
	//User requested a change to the second VLAN in GROUP X
	case 1:
		//Get the information held in register 117
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_3);
		//Set VLAN entry to VALID
		indirect_reg_data |= (1 << 1);

		//Read current port membership data
		port_membership = (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_3) & 0x01) + (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_2) >> 4);
		//Set bit corresponding to currently selected port
		port_membership = AssertVLANS(vlan_id, port_addr); //SetPortMembershipBits(port_membership, port_addr);

		//Place first bit of port_membership value into bit [0] of indirect register 117
		indirect_reg_data |= ((port_membership >> 4 ) << 0);
		//Overwrite register 117 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_3, indirect_reg_data))
		{
			return false;
		}
		//Get the information held in register 118
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_2);
		//Place the last four bits of port_membership value into bits [7-4] of indirect register 118
		indirect_reg_data |= ((port_membership & 0xF) << 4);
		//Overwrite register 118 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_2, indirect_reg_data))
		{
			return false;
		}
		break;
	//User requested a change to the third VLAN in GROUP X
	case 2:
		//Get the information held in register 116
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_4);
		//Set VLAN entry to VALID
		indirect_reg_data |= (1 << 6);

		//Read current port membership data
		port_membership = (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_4) >> 1);
		//Set bit corresponding to currently selected port
		port_membership = AssertVLANS(vlan_id, port_addr); //SetPortMembershipBits(port_membership, port_addr);

		//Place port_membership value into bits [5-1] of indirect register 116
		indirect_reg_data |= ((port_membership) << 1);
		//Overwrite register 116 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_4, indirect_reg_data))
		{
			return false;
		}
		break;
	//User requested a change to the fourth VLAN in GROUP X
	case 3:
		//Get the information held in register 114
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_6);
		//Set VLAN entry to VALID
		indirect_reg_data |= (1 << 3);

		//Read current port membership data
		port_membership = (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_6) & 0x07) + (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_5) >> 6);
		//Set bit corresponding to currently selected port
		port_membership = AssertVLANS(vlan_id, port_addr); //SetPortMembershipBits(port_membership, port_addr);

		//Place first three bits of port_membership value into bits [2-0] of indirect register 114
		indirect_reg_data |= ((port_membership >> 2 ) << 0);
		//Overwrite register 114 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_6, indirect_reg_data))
		{
			return false;
		}
		//Get the information held in register 115
		indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_5);
		//Place the last two bits of port_membership value into bits [7-6] of indirect register 115
		indirect_reg_data |= ((port_membership & 0x3) << 6);
		//Overwrite register 115 with updated information
		if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_5, indirect_reg_data))
		{
			return false;
		}
		break;
	}

	uint8_t temp_data = 0x80;
	temp_data += port_membership;
	if (!EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(0x200 + (vlan_id - 1)),temp_data)) {
		return false;
	}

	//Obtain current indirect access data 0 and modify
	indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0);
	//Select VLAN table, set operation mode to READ, and set 2 bits from MSB of indirect_reg_addr in bits 0 and 1 of this register
	indirect_access_data &= ~0x1f;
	indirect_access_data |= (INDIRECT_TABLESELECT_VLAN << INDIRECT_CONTROL_TABLESELECT) | (INDIRECT_READTYPE_WRITE << INDIRECT_CONTROL_READTYPEBIT) | (((indirect_reg_addr >> 8) & 0xFF) << INDIRECT_CONTROL_ADDRESS_HIGH);
	//Write the modified register to the Ethernet Controller
	if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0, indirect_access_data))
	{
		return false;
	}
	//Obtain current indirect access data 1 and modify
	indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_1);
	//Set this register to the remaining 8 bits of the 10-bit indirect_reg_addr
	indirect_access_data = (indirect_reg_addr & 0xFF);
	if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1, indirect_access_data))
	{
		return false;
	}



	return true;
}

//*****************************************************************************
//
//! Display VLAN Table (for Command-Line Interface)
//! Writes all active VLAN Table entries to the Command-Line Interface. This
//! function is a long-running task and takes approximately 20 seconds to fully
//! compile the VLAN table.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowVLANTable(char *params[MAX_PARAMS]) {
	uint32_t vlan_id = 1, item_count = 0;
	uint8_t vlan_data;
	int item_index = 0;

	portTickType ui32WakeTime;
	bool ContinueRequested = false;

	VLANTableEntry Entries[10];

	//This task takes a while, so we want share nicely with other tasks instead of blocking
	uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	//Get system time in ticks
	ui32WakeTime = xTaskGetTickCount();


	UARTprintf("[Compiling VLAN Table]: Please wait...\n");
	//Compile VLAN Table
	for (;vlan_id < 4096; vlan_id++) {
		vlan_data = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(0x200 + (vlan_id - 1)));

		//If this entry is valid (active), add this record to VLAN table
		if ((vlan_data & 0x80) == 0x80) {
			if (vlan_id == 0 || vlan_id > 4095) {
				continue;
			}
			if (item_count > 10) {
				//Pause here, display entries and display menu (Next or Exit)
				while (!ContinueRequested) {
					char option_entered;

					UARTprintf("\nVLAN ID    STATUS     PORTS ASSIGNED\n");
					for (item_index = 0; item_index < 10; item_index++) {
						UARTprintf("%d", Entries[item_index].VLAN_ID);
						if (Entries[item_index].VLAN_ID < 10)
							UARTprintf("   ");
						if (Entries[item_index].VLAN_ID < 100)
							UARTprintf("  ");
						if (Entries[item_index].VLAN_ID < 1000)
							UARTprintf(" ");
						UARTprintf("    %s    ", "Active");
						if (Entries[item_index].PORT_REGISTRATION & 0x20)
							UARTprintf("fast-eth0 ");
						if (Entries[item_index].PORT_REGISTRATION & 0x10)
							UARTprintf("fast-eth1 ");
						if (Entries[item_index].PORT_REGISTRATION & 0x08)
							UARTprintf("fast-eth2 ");
						if (Entries[item_index].PORT_REGISTRATION & 0x04)
							UARTprintf("fast-eth3 ");

						UARTprintf("\n");
					}
					UARTprintf("\n\nSelect An Option:: [N]: Next, [E]: Exit\n");
					//Disable console logging
					UARTEchoSet(false);

					while (!UARTRxBytesAvail()) {
						uint32_t currentTime = xTaskGetTickCount();
						vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
					}
					option_entered = UARTgetc();

					switch (option_entered) {
						//User wants to exit menu
						case 'E': case 'e':
							UARTEchoSet(true);
							return true;
						case 'N': case 'n':
							ContinueRequested = true;
							item_count = 0;
							item_index = 0;
							UARTEchoSet(true);
							break;
					}
				}
			}

			Entries[item_index].VLAN_ID = vlan_id;
			Entries[item_index].PORT_REGISTRATION = (vlan_data & 0x7C);
			Entries[item_index].isActive = true;

			item_index++;
			item_count++;
		}
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}
	UARTprintf("\nVLAN ID    STATUS     PORTS ASSIGNED\n");
	if (!item_count) {
		UARTprintf("==== NO ENTRIES FOUND IN VLAN TABLE ====");
		return true;
	}
	for (item_index = 0; item_index < item_count; item_index++) {
			UARTprintf("%d", Entries[item_index].VLAN_ID);
			if (Entries[item_index].VLAN_ID < 10)
				UARTprintf("   ");
			if (Entries[item_index].VLAN_ID < 100)
				UARTprintf("  ");
			if (Entries[item_index].VLAN_ID < 1000)
				UARTprintf(" ");
			UARTprintf("   %s   ", "Active");
			if (Entries[item_index].PORT_REGISTRATION & 0x20)
				UARTprintf("fast-eth0 ");
			if (Entries[item_index].PORT_REGISTRATION & 0x10)
				UARTprintf("fast-eth1 ");
			if (Entries[item_index].PORT_REGISTRATION & 0x08)
				UARTprintf("fast-eth2 ");
			if (Entries[item_index].PORT_REGISTRATION & 0x04)
				UARTprintf("fast-eth3 ");

			UARTprintf("\n");

	}
	return true;
}


//*****************************************************************************
//
//! Enable 802.1q VLAN Tagging (for Command-Line Interface)
//! Enables VLAN filtering for all active ports.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_EnableVLANS(char *params[MAX_PARAMS]){
	uint32_t global_control_3 = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_3_HEX);
	global_control_3 |= (1 << 7);
	if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,GLOBAL_CONTROL_3_HEX, global_control_3))
	{
		return false;
	}
	return true;
}

//*****************************************************************************
//
//! Disable 802.1q VLAN Tagging (for Command-Line Interface)
//! Disables VLAN filtering for all active ports.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DisableVLANS(char *params[MAX_PARAMS]){
	uint32_t global_control_3 = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_3_HEX);
	global_control_3 &= ~(1 << 7);
	if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,GLOBAL_CONTROL_3_HEX, global_control_3))
	{
		return false;
	}
	return true;
}

//*****************************************************************************
//
//! Reinitialize EEPROM (for Command-Line Interface)
//! Sets the flag in register 0x1E (bit 7) that controls whether or not the EEPROM
//! will be erased during reboot. This function will return successfully but
//! the effect of the command will not be seen until after the system has been
//! restarted.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ReinitializeEEPROM(char *params[MAX_PARAMS]){
	if (!EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,EEPROM_FIRMWARE_SETTINGS,0x80))
	{
		return false;
	}
	UARTprintf("[IMPORTANT]: Reboot required for changes to take effect!\n");
	return true;
}

//*****************************************************************************
//
//! Delete Configuration (for Command-Line Interface)
//! Sets the flag for loading configuration and VLANs in EEPROM register 0x1E
//! back to a logic low level. When rebooting the switch, the configuration and
//! VLAN values will not be loaded.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DeleteConfig(char *params[MAX_PARAMS]) {
	uint8_t FirmwareSettings = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_SETTINGS);

	FirmwareSettings &= ~(1 << 6);
	FirmwareSettings &= ~(1 << 5);

	if (!EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_SETTINGS, FirmwareSettings)) {
		return false;
	}
	return true;
}

//*****************************************************************************
//
//! Save Running Configuration to EEPROM (for Command-Line Interface)
//! Saves all registers from 0x00 to 0xFF inside the ethernet controller to
//! the corresponding EEPROM registers (0x100 - 0x1FF) an reports on the
//! success or failure of the operation.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SaveSwitchConfiguration(char *params[MAX_PARAMS])
{
	 uint32_t read_addr = 0x00, eeprom_eth0_addr = 0x100, eeprom_vlan_addr = 0x200, vlan_id = 1;
	 int progress = 0, task = 1;
	 uint8_t flag_data = 0x00;
	 portTickType ui32WakeTime;

	 //This task takes a while, so we want share nicely with other tasks instead of blocking
	 uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	 //Get system time in ticks
	 ui32WakeTime = xTaskGetTickCount();

	 UARTEchoSet(false);
	//Get current system flags from EEPROM
	flag_data = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, FLAG_BASE);

	 UARTprintf("[%d]: Saving Ethernet Controller Configuration To EEPROM (%d%%)\n", task, (task*25));
	 //Display progress bar
	 progress = CreateProgressBar();

	 for (read_addr = 0; read_addr < 0xFF; read_addr++)
	{
		//Update progress of task to user
		//ShowProgress((read_addr * 100)/0xFF);
		UpdateProgressBar(&progress, Increment, (100*read_addr)/0xFF);

		//Read data at address "read_addr" from Ethernet Controller 1
		uint8_t data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, read_addr);

		//Save this information to the next sector in EEPROM
		if (!EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, eeprom_eth0_addr, data)) {
			//We encountered a bad write cycle, report this to the user
			return false;
		}

		//Increment to the next sector in EEPROM
		eeprom_eth0_addr++;

		//Delay for 40ms to allow other tasks to run
        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}

	//Set CONFIG_SAVED flag
	flag_data |= 1 << FLAG_CONFIG_SAVED;
	task++;

	//Write VLAN entries to EEPROM (Start: 0x200, End 0x11FF)
	//8-bit register
	//7 : Entry is valid/not valid/not
	//6 - 2 : Membership ID bits

	uint32_t global_control_3 = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_3_HEX);
	if (global_control_3 && 0x80) {
		//If VLANs are enabled, Save the changes!
		UARTprintf("\n[%d]: Saving VLANs To EEPROM (%d%%)\n", task, (task*25));
		progress = CreateProgressBar();
		ui32TaskDelay = VERY_SHORT_TASK_DLY;
		for (vlan_id = 0;vlan_id < 16; vlan_id++) {
			EEPROMPageErase(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, (eeprom_vlan_addr + (vlan_id*256)));
		}
		for (vlan_id = 1;vlan_id < 4096; vlan_id++) {
			uint32_t indirect_reg_addr = (vlan_id / 4), indirect_reg_data = 0x00, port_membership = 0x00, vlan_status = 0x00;
			uint32_t indirect_reg_values[7] = {0x00};

			//Obtain current indirect access data 0 and modify
			uint32_t indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0);
			//Select VLAN table, set operation mode to READ, and set 2 bits from MSB of indirect_reg_addr in bits 0 and 1 of this register
			indirect_access_data |= (INDIRECT_TABLESELECT_VLAN << INDIRECT_CONTROL_TABLESELECT) | (INDIRECT_READTYPE_READ << INDIRECT_CONTROL_READTYPEBIT) | (((indirect_reg_addr >> 8) & 0xFF) << INDIRECT_CONTROL_ADDRESS_HIGH);
			//Write the modified register to the Ethernet Controller
			if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0, indirect_access_data))
			{
				return false;
			}
			//Obtain current indirect access data 1 and modify
			indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_1);
			//Set this register to the remaining 8 bits of the 10-bit indirect_reg_addr
			indirect_access_data = (indirect_reg_addr & 0xFF);
			if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1, indirect_access_data))
			{
				return false;
			}
			//Read all indirect register values
			EthoControllerBulkRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_6, 7, indirect_reg_values);

			//VLAN SETTINGS MODIFICATION START HERE
			/*Each indirect register contains informatin for four VLANs organized in the following way:
			* ENTRY 0 (12 bits) maps to first VLAN, indirect bits (12-0): register 119 (bits 4-0), register 120 (bits 7-0)
			* ENTRY 1 (12 bits) maps to second VLAN, indirect bits (25-13): register 117 (bits 1-0), register 118 (bits 7-0), register 119 (bits 7-5)
			* ENTRY 2 (12 bits) maps to third VLAN, indirect bits (38-26): register 116 (bits 6-0), register 117 (7-2)
			* ENTRY 3 (12 bits) maps to fourth VLAN, indirect bits (51-39): register 114 (bits 3-0), register 115 (7-0), register 116 (bit 7);
			*
			* VLAN GROUP = (VLAN_ID / 4)
			* VLAN = VLAN GROUP % 4
			*/
			switch (vlan_id % 4) {
			//First VLAN in GROUP X
			case 0:
				//Get the information held in register 119
				indirect_reg_data = indirect_reg_values[5];
				//Get VLAN status (0 : Entry is invalid, 1: Entry is valid)
				vlan_status |= (indirect_reg_data & 0x10) >> 4;
				//Read current port membership data
				port_membership = (indirect_reg_values[5] & 0xF) + (indirect_reg_values[6] >> 7);
				break;
			//Second VLAN in GROUP X
			case 1:
				//Get the information held in register 117
				indirect_reg_data = indirect_reg_values[3];
				//Get VLAN status (0 : Entry is invalid, 1: Entry is valid)
				vlan_status |= (indirect_reg_data & 0x02) >> 1;
				//Read current port membership data
				port_membership = (indirect_reg_values[3] & 0x01) + (indirect_reg_values[4] >> 4);
				break;
			//Third VLAN in GROUP X
			case 2:
				//Get the information held in register 116
				indirect_reg_data = indirect_reg_values[2];
				//Get VLAN status (0 : Entry is invalid, 1: Entry is valid)
				vlan_status |= (indirect_reg_data & 0x40) >> 2;
				//Read current port membership data
				port_membership = (indirect_reg_values[2] >> 1);
				break;
			//Fourth VLAN in GROUP X
			case 3:
				//Get the information held in register 114
				indirect_reg_data = indirect_reg_values[0];
				//Get VLAN status (0 : Entry is invalid, 1: Entry is valid)
				vlan_status |= (indirect_reg_data & 0x08) >> 3;
				//Read current port membership data
				port_membership = (indirect_reg_values[0] & 0x07) + (indirect_reg_values[1] >> 6);
				break;
			}

			//Save aquired VLAN information to temporary 8-bit integer
			uint8_t vlan_data = 0x00;
			vlan_data |= vlan_status << 7;
			vlan_data |= port_membership << 2;
			//Write aquired data to current register in EEPROM (vlan_id normalized to zero by subtracting one from current value)
			if (!EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, (eeprom_vlan_addr + (vlan_id - 1)), vlan_data)) {
				return false;
			}

			UpdateProgressBar(&progress, Increment, (100*vlan_id)/4096);

			vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
		}

		flag_data |= 1 << FLAG_CONFIG_VLAN_VALID;
		task++;
	}
	else {
		flag_data &= ~(1 << FLAG_CONFIG_VLAN_VALID);
	}

	UARTprintf("\n[%d]: Saving Updated User Database To EEPROM (%d%%)\n", task, (task*25));
	progress = CreateProgressBar();
	ui32TaskDelay = VERY_SHORT_TASK_DLY;

	char empty_array[16] = {0x00};

	int current_user = 0, user_cnt = 0;

	for (user_cnt = 0; user_cnt < MAX_USERS; user_cnt++) {
		if (users[current_user].nextAction == Update || users[current_user].nextAction == Add) {
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65)),(uint8_t*)users[current_user].username, 16);
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 16),(uint8_t*)users[current_user].password, 16);
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 32),(uint8_t*)users[current_user].first_name, 16);
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 48),(uint8_t*)users[current_user].last_name, 16);
			EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 64),users[current_user].permissions);
		}
		if (users[current_user].nextAction == Delete) {
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65)),(uint8_t*)empty_array, 16);
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 16),(uint8_t*)empty_array, 16);
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 32),(uint8_t*)empty_array, 16);
			EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 48),(uint8_t*)empty_array, 16);
			EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 64),0x00);
			user_cnt -= 1;
		}
		current_user++;
		if (current_user > (MAX_USERS - 1) && (user_cnt < (MAX_USERS - 1)))
		{
			//Records were deleted, fill all remaining locations in EEPROM with blank records
			for (; user_cnt < MAX_USERS; user_cnt++) {
				EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65)),(uint8_t*)empty_array, 16);
				EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 16),(uint8_t*)empty_array, 16);
				EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 32),(uint8_t*)empty_array, 16);
				EEPROMBulkWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 48),(uint8_t*)empty_array, 16);
				EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (user_cnt*65) + 64),0x00);
			}
			break;
		}

		if (users[current_user].nextAction != Delete && (user_cnt > 0)) {
			UpdateProgressBar(&progress, Increment, (100*user_cnt)/MAX_USERS);
		}

        vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
	}

	flag_data |= 1 << FLAG_CONFIG_USERS_VALID;

	//Save Log Status Flags
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_1, ((LogStatusFlags >> 24) & 0xFF));
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_2, ((LogStatusFlags >> 16) & 0xFF));
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_3, ((LogStatusFlags >> 8) & 0xFF));
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_4, ((LogStatusFlags) & 0xFF));

	//Save Next Log Status Pointer
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_1, ((NextLogSlot >> 24) & 0xFF));
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_2, ((NextLogSlot >> 16) & 0xFF));
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_3, ((NextLogSlot >> 8) & 0xFF));
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_4, ((NextLogSlot) & 0xFF));




	//Save system flags to EEPROM
	EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, FLAG_BASE, flag_data);

	UARTEchoSet(true);

	return true;
}


//*****************************************************************************
//
//! Show Current Port Status (for Command-Line Interface)
//! Reads the registers specified by "PortConfigMappings" (refer to "interpreter_task.h")
//! and reports the current value of each in a textual format.
//
//*****************************************************************************
void ShowPortStatus(uint32_t port_addr) {
	int reg = 0, option = 0, value = 0, longest_string = 0, add_spaces = 0;
	//Find longest option for console alignment
	for (reg = 0; reg < PORT_MAX_MAPPINGS; reg++) {
		for (option=0; option<MAX_OPTIONS;option++) {
			if (PortConfigMappings[reg].options[option].description == 0) {
				break;
			}
			else if (strlen(PortConfigMappings[reg].options[option].description) > longest_string){
				longest_string = strlen(PortConfigMappings[reg].options[option].description);
			}
		}
	}

	//Set by developer: Increase additional number of spaces by 2 to seperate options from values better
	longest_string += 2;

	//Iterate through each option in the port register mappings
	for (reg = 0; reg < PORT_MAX_MAPPINGS; reg++) {
		if (PortConfigMappings[reg].options == 0)
			break;
		//Read the register address held by this mapping
		uint8_t data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PortConfigMappings[reg].base_addr + port_addr));

		//Use the mask for each option to check it against the possible values it can be
		for (option = 0; option < MAX_OPTIONS; option++) {
			if (PortConfigMappings[reg].options[option].description == 0) {
				break;
			}
			uint8_t masked_data = (data & PortConfigMappings[reg].options[option].mask);
			UARTprintf("\t%s:", PortConfigMappings[reg].options[option].description);

			//Go through each possible value to see if we matched one
			for (value = 0; value < MAX_VALUES; value++) {
				if (PortConfigMappings[reg].options[option].values[value].value_description == 0) {
					UARTprintf("\n");
					break;
				}
				if (PortConfigMappings[reg].options[option].values[value].value == masked_data) {
					//Print the string equivalent of the value found
					for (add_spaces = 0; add_spaces < (longest_string - strlen(PortConfigMappings[reg].options[option].description)); add_spaces++)
					{
						UARTprintf(" ");
					}

					//Add a very small delay in this function to allow the UART to completely flush its TX buffer
					delayMs(1);

					UARTprintf("%s\n", PortConfigMappings[reg].options[option].values[value].value_description);
					break;
				}
			}
		}
	}
}
//*****************************************************************************
//
//! Show Global Switch Status (for Command-Line Interface)
//! Reads the registers specified by "GlobalConfigMappings" (refer to "interpreter_task.h")
//! and reports the current value of each in a textual format.
//
//*****************************************************************************
void ShowGlobalStatus() {
	int reg = 0, option = 0, value = 0, longest_string = 0, add_spaces = 0;

	for (reg = 0; reg < GLOBAL_CONFIG_MAX_MAPPINGS; reg++) {
		for (option = 0; option < MAX_OPTIONS; option++) {
			if (GlobalConfigMappings[reg].options[option].description == 0) {
				break;
			}
			else if (strlen(GlobalConfigMappings[reg].options[option].description) > longest_string) {
				longest_string = strlen(GlobalConfigMappings[reg].options[option].description);
			}
		}
	}

	//Set by developer: Increase additional number of spaces by 2 to seperate options from values better
	longest_string += 2;

	//Iterate through each option in the port register mappings
	for (reg = 0; reg < GLOBAL_CONFIG_MAX_MAPPINGS; reg++) {
		if (GlobalConfigMappings[reg].options == 0)
			break;
		//Read the register address held by this mapping
		uint8_t data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (GlobalConfigMappings[reg].base_addr));

		//Use the mask for each option to check it against the possible values it can be
		for (option = 0; option < MAX_OPTIONS; option++) {
			if (GlobalConfigMappings[reg].options[option].description == 0) {
				break;
			}
			uint8_t masked_data = (data & GlobalConfigMappings[reg].options[option].mask);
			UARTprintf("\t%s:", GlobalConfigMappings[reg].options[option].description);

			//Go through each possible value to see if we matched one
			for (value = 0; value < MAX_VALUES; value++) {
				if (GlobalConfigMappings[reg].options[option].values[value].value_description == 0) {
					UARTprintf("\n");
					break;
				}
				if (GlobalConfigMappings[reg].options[option].values[value].value == masked_data) {
					//Print the string equivalent of the value found
					for (add_spaces = 0; add_spaces < (longest_string - strlen(GlobalConfigMappings[reg].options[option].description)); add_spaces++)
					{
						UARTprintf(" ");
					}

					//Add a very small delay in this function to allow the UART to completely flush its TX buffer
					delayMs(1);

					UARTprintf("%s\n", GlobalConfigMappings[reg].options[option].values[value].value_description);
					break;
				}
			}
		}
	}
}

//*****************************************************************************
//
//! Show Global Switch Status (for Command-Line Interface)
//! Reads the registers specified by both "PortConfigMappings"
//! and "GlobalConfigMappings" (refer to "interpreter_task.h")
//! and reports the current value of each in a textual format.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowRunningConfig(char *params[MAX_PARAMS]) {
	bool isValidOption = false;
	char option_entered = 0x00;
	uint32_t taskDelay = LONG_RUNNING_TASK_DLY;

	UARTprintf("\n========== GLOBAL SETTINGS ==========\n");
	ShowGlobalStatus();

	while (true) {
		switch (option_entered) {
		case 'G': case 'g':
			UARTprintf("\n========== GLOBAL SETTINGS ==========\n");
			ShowGlobalStatus();
			break;
		case '0':
			UARTprintf("\n========== PORT 0 SETTINGS ==========\n");
			ShowPortStatus(PORT1_OFFSET_HEX);
			break;
		case '1':
			UARTprintf("\n========== PORT 1 SETTINGS ==========\n");
			ShowPortStatus(PORT2_OFFSET_HEX);
			break;
		case '2':
			UARTprintf("\n========== PORT 2 SETTINGS ==========\n");
			ShowPortStatus(PORT3_OFFSET_HEX);
			break;
		case '3':
			UARTprintf("\n========== PORT 3 SETTINGS ==========\n");
			ShowPortStatus(PORT4_OFFSET_HEX);
			break;
		}
		UARTprintf("\nOPTIONS: [G]: Global Settings  [0]: Port 0  [1]: Port 1  [2]: Port 2  [3]: Port 3  [E]: EXIT");
		UARTEchoSet(false);
		isValidOption = false;
		option_entered = 0x00;
		while (!isValidOption) {
			while (!UARTRxBytesAvail()) {
				uint32_t currentTime = xTaskGetTickCount();
		        vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
			}
			option_entered = UARTgetc();
			switch (option_entered) {
			case 'G': case 'g':
				isValidOption = true;
				break;
			case '0':
				isValidOption = true;
				break;
			case '1':
				isValidOption = true;
				break;
			case '2':
				isValidOption = true;
				break;
			case '3':
				isValidOption = true;
				break;
			case 'E': case 'e':
				isValidOption = true;
				UARTEchoSet(true);
				return true;
			}
		}
		UARTEchoSet(true);
	}
}

//*****************************************************************************
//
//! Show Current Port Status (for Command-Line Interface)
//! Reads the registers specified by both "PortConfigMappings" (refer to "interpreter_task.h")
//! and reports the current value of each in a textual format. Aligns all printed
//!	text printed to command-line to increase legibility.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowPortStatus(char *params[MAX_PARAMS]) {
	//Parameter 1: Port Base Address
	uint32_t port_addr = (uint32_t)strtol(params[0],NULL,0);
	int reg = 0, option = 0, longest_string = 0;

	//Print port identifer
	switch (port_addr) {
		case PORT1_OFFSET_HEX:
			UARTprintf("Configuration for <Fast Ethernet 0>\n");
			break;
		case PORT2_OFFSET_HEX:
			UARTprintf("Configuration for <Fast Ethernet 1>\n");
			break;
		case PORT3_OFFSET_HEX:
			UARTprintf("Configuration for <Fast Ethernet 2>\n");
			break;
		case PORT4_OFFSET_HEX:
			UARTprintf("Configuration for <Fast Ethernet 3>\n");
			break;
		default:
			UARTprintf("Invalid Port Specified\n");
	}

	//Find longest option for console alignment
	for (reg = 0; reg < PORT_MAX_MAPPINGS; reg++) {
		for (option=0; option<MAX_OPTIONS;option++) {
			if (PortConfigMappings[reg].options[option].description == 0) {
				break;
			}
			else if (strlen(PortConfigMappings[reg].options[option].description) > longest_string){
				longest_string = strlen(PortConfigMappings[reg].options[option].description);
			}
		}
	}

	//Set by developer: Increase additional number of spaces by 2 to seperate options from values better
	longest_string += 2;

	ShowPortStatus(port_addr);

	return true;

}

//*****************************************************************************
//
//! Perform Soft Reset (for Command-Line Interface)
//! Issues a manual software reset of the Tiva C from the command-line. Requires
//! the user to enter the command twice before resetting.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ResetTivaC(char *params[MAX_PARAMS]) {
	if (!ResetIssued) {
		UARTprintf("\nAre you sure? Type 'system reset' again to confirm\n");
		ResetIssued = true;
		return false;
	}
	else {
		SysCtlReset();
	}
	return true;
}

//*****************************************************************************
//
//! Display Status of EEPROM Events (for Command-Line Interface)
//! Writes the current state (enabled/disabled) of all configured EEPROM events
//! to the CLI.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_EventStatus(char *params[MAX_PARAMS]) {
	int event_no = 0, longest_event = 0, spaces_to_align = 0;
	UARTprintf("\n ====== Events currently logged to EEPROM ======\n");

	for (event_no = 0; event_no < MAX_LOG_TYPES; event_no++) {
		if (LogTypes[event_no] == 0x00) {
			break;
		}
		if (strlen(LogTypes[event_no]) > longest_event) {
			longest_event = strlen(LogTypes[event_no]);
		}
	}
	for (event_no = 0; event_no < MAX_LOG_TYPES; event_no++) {
		if (LogTypes[event_no] == 0x00) {
			return true;
		}
		//Print log type string
		UARTprintf("\n %s", LogTypes[event_no]);

		//If this log type string is less than that longest available, add spaces to left align
		for (spaces_to_align = strlen(LogTypes[event_no]); spaces_to_align < longest_event; spaces_to_align++) {
			UARTprintf(" ");
		}

		//Show user whether this log type is currently active or not
		if ((LogStatusFlags >> event_no) & 1) {
			UARTprintf(" - [ENABLED]\n");
		}
		else {
			UARTprintf(" - [DISABLED]\n");
		}

	}
	return true;
}

//*****************************************************************************
//
//! Manage EEPROM Logged Events (for Command-Line Interface)
//! Displays a menu of checkboxes to allow for easy enabling/disabling of configured
//! system events to log to the EEPROM. The results of this menu will only be saved
//! when the user issues a configuration save command. This prevents the changes from
//! being lost on a reboot.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ManageEvents(char *params[MAX_PARAMS]) {
	int i = 0,total_items = 0,current_item=0;
	char option_entered = 0x00;

	uint32_t taskDelay = LONG_RUNNING_TASK_DLY;

	//Print what this menu is doing
	UARTprintf("\nCheck all events to ENABLE/DISABLE by using the arrow keys\nUse <ENTER> to select, <C> to confirm, <E> to exit\n");

	for (i = 0; i < MAX_LOG_TYPES; i++) {
		if (LogTypes[i] != 0x00) {
			if ((LogStatusFlags >> i) & 1) {
				UARTprintf("[#] EVENT: %s\n", LogTypes[i]);
			}
			else {
				UARTprintf("[ ] EVENT: %s\n", LogTypes[i]);
			}
			total_items++;
			current_item++;
		}
	}

	//Place cursor in first item checkbox
	for (i = 0; i < total_items; i++) {
		UARTprintf("\033[1A");
		current_item--;
	}
	UARTprintf("\033[1C");

	while (true) {

		//Disable console logging
		UARTEchoSet(false);

		while (!UARTRxBytesAvail()) {
			uint32_t currentTime = xTaskGetTickCount();
			vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
		}
		option_entered = UARTgetc();

		switch (option_entered) {
			//Up Arrow Entered
			case 0x41:
				if (current_item > 0) {
			       UARTprintf("\033[1A");
			       current_item--;
				}
				break;
			//Down Arrow Entered
			case 0x42:
				if (current_item < (total_items - 1)) {
			       UARTprintf("\033[1B");
			       current_item++;
				}
				break;
			case '\n': case '\r':
				if ((LogStatusFlags >> current_item) & 1) {
					UARTprintf(" \033[1D");
					LogStatusFlags &= ~(1 << current_item);
				}
				else {
					UARTprintf("#\033[1D");
					LogStatusFlags |= 1 << current_item;
				}
				break;
			//User wants to exit menu
			case 'E': case 'e':
				//Reset cursor to bottom of screen
				for (i = current_item; i < total_items; i++) {
				   UARTprintf("\033[1B");
				}
			    UARTprintf("\033[2B\033[1D");
				UARTEchoSet(true);
				return true;
			case 'C': case 'c':
				//Reset cursor to bottom of screen
				for (i = current_item; i < total_items; i++) {
				   UARTprintf("\033[1B");
				}
			    UARTprintf("\033[2B\033[1D");
			    //Return control of RX buffer to user
				UARTEchoSet(true);
				UARTprintf("\n[NOTICE]: Save switch configuration before turning off system!\n");
				return true;
		}
	}
}

//*****************************************************************************
//
//! List All EEPROM Logged Events (for Command-Line Interface)
//! Displays a dump of all events logged to the EEPROM. Since the current
//! iteration of this system does not have a way of saving or retriving a real-time
//! clock, the metric reported before the logged event is the number of FreeRTOS
//! ticks since the system was started. Maximum number of log entries that can be
//! stored/read is 400.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ListEvents(char *params[MAX_PARAMS]) {
	int entry_memaddr = EEPROM_LOG_BASE;
	//Read first 20 log entries


	for (; entry_memaddr < (EEPROM_LOG_BASE + (MAX_LOG_ENTRIES * 5)); entry_memaddr += 5) {
		uint32_t timestamp;
		uint8_t timestamp_data[4];
		LoggerCodes event;

		if (!EEPROMBulkRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,entry_memaddr, timestamp_data,4)) {
			return false;
		}
		timestamp = (timestamp_data[0] << 24) | (timestamp_data[1] << 16) | (timestamp_data[2] << 8) | (timestamp_data[3]);
		if (timestamp == 0 && (entry_memaddr == EEPROM_LOG_BASE)) {
			UARTprintf("\n=== NO LOG ENTRIES FOUND ===\n");
			return true;
		}
		else if (timestamp == 0 || (entry_memaddr >= NextLogSlot)) {
			UARTprintf("\n=== END OF LOG ===\n");
			return true;
		}
		else {
			event = (LoggerCodes)EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, entry_memaddr + 4);
			UARTprintf("[System Time: %d] - %s\n", timestamp, LogTypes[event]);
		}
	}
	return true;
}
//*****************************************************************************
//
//! Delete All EEPROM Logged Events (for Command-Line Interface)
//! Erases all EEPROM Sectors that are allocated for log entries and resets
//! next log slot to the initial EEPROM_LOG_BASE value.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DeleteEvents(char *params[MAX_PARAMS]) {
	int page_addr = EEPROM_LOG_BASE;
	for (; page_addr < (EEPROM_LOG_BASE + (8*256)); page_addr += 256) {
		EEPROMPageErase(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, page_addr);
	}
	NextLogSlot = EEPROM_LOG_BASE;
	UARTprintf("\n[NOTICE]: Save switch configuration before turning off system!\n");
	return true;
}

//*****************************************************************************
//
//! Add New User to System (for Command-Line Interface)
//! Issues prompts to the command-line to gather information for a new user. Once
//! all fields in the User_Data structure are filled, this function places the new
//! user at the next available slot in the "users" array. In order to prevent this
//! function from blocking the FreeRTOS tasks currently running, vTaskDelayUntil
//! is used while waiting for a new line character from the command-line.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_AddUser(char *params[MAX_PARAMS])
{
	int available_slot = MAX_USERS - 1, current_user = 0;

	User_Data NewUser = {0};

	portTickType ui32WakeTime;

	//This task takes a while, so we want share nicely with other tasks instead of blocking
	uint32_t ui32TaskDelay = SHORT_TASK_DLY;

	//Get system time in ticks
	ui32WakeTime = xTaskGetTickCount();

	//If all 15 slots allocated for users have been filled, the last one's USERNAME field will not be NULL.
	if (users[available_slot].username[0] != 0x00) {
		//Maximum user limit exceeded!
		UARTprintf("\nMaximum user limit exceeeded (15)! \nPlease delete an existing user before adding a new one.\n");
		return false;
	}

	//Get the next available slot in the "users" array
	for (;available_slot > -1; available_slot--)
	{
		//If the current slot's FIRST NAME field is NULL, we've found an empty slot
		if (users[available_slot].first_name[0] != 0x00) {
			//Normalize the available_slot varaible to that of a zero-based value. This allows its reuse later in this function.
			available_slot += 1;
			break;
		}
	}
	//If we failed to find an entry in the user's array, set the available slot variable to zero so we can place a new user there.
	if (available_slot == -1) {
		available_slot = 0;
	}

	//Disable buffer redirection for interpreter task. Let this function process the UART's RX buffer.
	UARTInterpreterEnabled = false;

	//Get a USERNAME value from the CLI.
	while (NewUser.username[0] == 0x00) {
		UARTprintf("\nUsername (16 character max): ");
		//Wait for enter key and pend this task unil done
		while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
			while (!UARTRxBytesAvail()) {
				//Pend until a new line character is detected.
				uint32_t currentTime = xTaskGetTickCount();
				vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
			}
		}
		//Get 16 characters from the UART RX buffer
		UARTgets(NewUser.username, 16);
		//Delete everything else
		UARTFlushRx();
		//Check to see if the username does not already exist in the "users" array.
		for (current_user = 0; current_user < MAX_USERS; current_user++) {
			if (strcmp(NewUser.username, users[current_user].username) == 0) {
				UARTprintf("\nUser already exists. Please enter a unique username.\n");
				memset(NewUser.username, 0x00, 16);
				break;
			}
		}
	}

	//Get a FIRST NAME value from the CLI.
	while (NewUser.first_name[0] == 0x00) {
		UARTprintf("\nFirst Name (16 character max): ");
		//Wait for enter key and pend this task unil done
		while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
			while (!UARTRxBytesAvail()) {
				//Pend until a new line character is detected.
				uint32_t currentTime = xTaskGetTickCount();
				vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
			}
		}
		//Get 16 characters from the UART RX buffer
		UARTgets(NewUser.first_name, 16);
		//Delete everything else.
		UARTFlushRx();
	}

	//Get a LAST NAME value from the CLI.
	while (NewUser.last_name[0] == 0x00) {
		UARTprintf("\nLast Name (16 character max): ");
		//Wait for enter key and pend this task unil done
		while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
			while (!UARTRxBytesAvail()) {
				//Pend until a new line character is detected.
				uint32_t currentTime = xTaskGetTickCount();
				vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
			}
		}
		//Get 16 characters from the UART RX buffer
		UARTgets(NewUser.last_name, 16);
		//Delete everything else.
		UARTFlushRx();
	}

	//Get a PASSWORD from the CLI.
	while (NewUser.password[0] == 0x00) {
		UARTprintf("\nPassword (16 character max): ");
		//Wait for enter key and pend this task unil done
		while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
			while (!UARTRxBytesAvail()) {
				//Pend until a new line character is detected.
				uint32_t currentTime = xTaskGetTickCount();
				vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
			}
		}
		//Get 16 characters from the UART RX buffer
		UARTgets(NewUser.password, 16);
		//Delete everything else.
		UARTFlushRx();
	}

	char value_entered = 0x00;

	//Get a PERMISSION LEVEL from the CLI.
	while (value_entered != '0' && value_entered != '1' && value_entered != '2' && value_entered != '3') {
		value_entered = 0x00;
		UARTprintf("\n\nENTER ONE OF THE FOLLOWING:\n0: User has read-only permissions\n1: User can change port settings\n2: User can change port and system settings\n3: User has full administrative rights\nPermission Level (0 | 1 | 2 | 3): ");
		//Wait for enter key and pend this task unil done
		while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
			while (!UARTRxBytesAvail()) {
				//Pend until a new line character is detected.
				uint32_t currentTime = xTaskGetTickCount();
				vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
			}
		}

		value_entered = UARTgetc();
		UARTFlushRx();
		//Is the value entered outside of the allowed values?
		if (value_entered != '0' && value_entered != '1' && value_entered != '2' && value_entered != '3') {
			UARTprintf("\nInvalid entry!\n");
		}
		else {
			NewUser.permissions = (PermLevel)(value_entered - '0');
		}
	}


	//Enable buffer redirection for interpreter task. Return control of UART RX buffer to interpreter task.
	UARTInterpreterEnabled = true;

	//Flag this entry to be saved to the EEPROM
	NewUser.nextAction = Add;

	users[available_slot] = NewUser;

	UARTprintf("\n\nUser added to table. Save switch configuration to make changes permanent!\n\tUsername: %s\n\tFirst Name: %s\n\tLast Name: %s\n", NewUser.username, NewUser.first_name, NewUser.last_name);
	return true;
}

//*****************************************************************************
//
//! List Currently Entered Users (for Command-Line Interface)
//! Goes through the "users" array and displays all current values to the CLI.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ListUsers(char *params[MAX_PARAMS]) {
	int user_index = 0;
	for (user_index = 0; user_index < MAX_USERS; user_index++) {
		if (users[0].username[0] == 0) {
			//Empty users array
			UARTprintf("\n === NO USERS IN DATABASE === \n");
			return false;
		}
		if (users[user_index].username[0] != 0) {
			UARTprintf("[%d] USER: %s\n\t%s %s\n\tROLE: %s\n", (user_index + 1), users[user_index].username, users[user_index].first_name, users[user_index].last_name, RoleDefs[users[user_index].permissions]);
			if (users[user_index].nextAction == Delete) {
				UARTprintf("\t[USER MARKED FOR DELETION]\n");
			}
		}
	}
	return true;
}

//*****************************************************************************
//
//! Delete Users Menu (for Command-Line Interface)
//! Provides the CLI with an easy-to-navigate selectable list of all current
//! users and allows selection of those to delete. In order to save any of the
//! checked users, the configuration must be saved to the EEPROM.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DeleteUsersMenu(char *params[MAX_PARAMS]) {
	int i = 0,total_items = 0,current_item=0;
	char option_entered = 0x00;

	uint32_t taskDelay = LONG_RUNNING_TASK_DLY;

	//Print what this menu is doing
	UARTprintf("\nCheck all users to DELETE by using the arrow keys\nUse <ENTER> to select, <C> to confirm, <E> to exit\n");

	if (users[0].username[0] == 0) {
		//Empty users array
		UARTprintf("\n === NO USERS IN DATABASE === \n");
		return false;
	}

	//Show all of the current users with check marks if their isMarked value is true.
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].username[0] != 0) {
			if (users[i].isMarked) {
				UARTprintf("[#] USER: %s\n\t%s %s\n\tROLE: %s\n", users[i].username, users[i].first_name, users[i].last_name, RoleDefs[users[i].permissions]);
			}
			else {
				UARTprintf("[ ] USER: %s\n\t%s %s\n\tROLE: %s\n", users[i].username, users[i].first_name, users[i].last_name, RoleDefs[users[i].permissions]);
			}
			//A counter to keep track of the length of the array
			total_items++;
			//A counter to keep track of the currently highlighted item.
			current_item++;
		}
	}

	//Place cursor in first item checkbox
	for (i = 0; i < total_items; i++) {
		//Move cursor UP three lines
		UARTprintf("\033[3A");
		current_item--;
	}
	//Move cursor RIGHT 1 column
	UARTprintf("\033[1C");

	while (true) {

		//Disable console logging
		UARTEchoSet(false);

		while (!UARTRxBytesAvail()) {
			uint32_t currentTime = xTaskGetTickCount();
			vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
		}
		option_entered = UARTgetc();

		//Check to see which of the menu action items the user selected
		switch (option_entered) {
			//Up Arrow Entered
			case 0x41:
				if (current_item > 0) {
					//Move cursor UP 1 row
			       UARTprintf("\033[3A");
			       current_item--;
				}
				break;
			//Down Arrow Entered
			case 0x42:
				if (current_item < (total_items - 1)) {
				   //Move cursor DOWN 1 row
			       UARTprintf("\033[3B");
			       current_item++;
				}
				break;
			case '\n': case '\r':
				if (users[current_item].isMarked) {
					//Move cursor LEFT 1 column
					UARTprintf(" \033[1D");
					users[current_item].isMarked = false;
				}
				else if (!users[current_item].isMarked) {
					//Move cursor LEFT 1 column
					UARTprintf("#\033[1D");
					users[current_item].isMarked = true;
				}
				break;
			//User wants to exit menu
			case 'E': case 'e':
				//Reset cursor to bottom of screen
				for (i = current_item; i < total_items; i++) {
					//Move cursor DOWN 1 row
				   UARTprintf("\033[3B");
				}
			    UARTprintf("\033[2B\033[1D");
				UARTEchoSet(true);
				return true;
			case 'C': case 'c':
				//Reset cursor to bottom of screen
				for (i = current_item; i < total_items; i++) {
				   UARTprintf("\033[3B");
				}
			    UARTprintf("\033[2B\033[1D");
			    //Mark users for deletion
			    for (i = 0; i < MAX_USERS; i++) {
			    	if (users[i].isMarked) {
			    		users[i].nextAction = Delete;
			    	}
			    	else {
			    		if (users[i].username[0] != 0) {
			    		users[i].nextAction = Update;
			    		}
			    	}
			    }
			    //Return control of RX buffer to user
				UARTEchoSet(true);
				UARTprintf("\n[NOTICE]: Save switch configuration to update user database\n");
				return true;
		}
	}
}

//*****************************************************************************
//
//! Show Current Static MAC Table (for Command-Line Interface)
//! Displays all valid entries in the Micrel KSZ8895MLUB Ethernet Controller's
//! static MAC table. In order to obtains these values, the function accessess
//! the indirect registers in the KSZ8895MLUB. Information regarding these
//! registers can be found in the datasheet for this part.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowStaticMACTable(char *params[MAX_PARAMS]) {

	//Setup indirect access registers
	//We want to READ (bit 4 of 0x6E = 1) the STATIC MAC table (bits [3:2] of 0x6E = 00)
	uint8_t reg6EBase = 0x10;

	//And check each entry in the MAC table from 0x00 to 0x3FF (10 bits)
	uint16_t current_entry = 0x00;

	for (current_entry = 0x00; current_entry < 0x400; current_entry++) {

		//Data for each entry will be held here
		uint32_t MACTableData[8] = {0x00};

		//Set reg6E to read <current_entry [8:9]>
		EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_0,(reg6EBase | ((current_entry & 0x300) >> 8)));

		//Set register 111 (0x6F [Indirect Access Control 1]) to the remaining 8 bits of <current_entry [7:0]>
		EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1,(current_entry & 0xFF));

		//This will start a read of <current_entry>
		EthoControllerBulkRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_7,8,MACTableData);

		//Check validity of entry before processing. This will be in MACTableData[1] bit 6.
		if ((MACTableData[1] >> 5) & 1) {
			//This entry is valid and we want to show it to the user!
			if (current_entry == 0x00) {
				//If this is the first entry checked, print the table header
				UARTprintf("== FILTER ID ==\t == USE FID ==\t == OVERRIDE STP ==\t == FORWARDING PORTS ==\t == MAC ADDRESS ==\n");
			}

			//Print Filter ID (FID)
			UARTprintf("%d\t",(MACTableData[0] >> 1));

			//Print Use FID?
			if (MACTableData[0] & 1) {
				UARTprintf("TRUE\t");
			}
			else {
				UARTprintf("FALSE\t");
			}

			//Print Override STP?
			if ((MACTableData[1] >> 7) & 1) {
				UARTprintf("YES\t");
			}
			else {
				UARTprintf("NO\t");
			}

			//Print forwarding ports
			if ((MACTableData[1] >> 4) & 1) {
				UARTprintf(" f3 ");
			}
			if ((MACTableData[1] >> 3) & 1) {
				UARTprintf(" f2 ");
			}
			if ((MACTableData[1] >> 2) & 1) {
				UARTprintf(" f1 ");
			}
			if ((MACTableData[1] >> 1) & 1) {
				UARTprintf(" f0 ");
			}
			if ((MACTableData[1] >> 0) & 1) {
				UARTprintf(" exp-port");
			}

			//Print MAC Address
			UARTprintf("\t%02X:%02X:%02X:%02X:%02X:%02X\n", MACTableData[2], MACTableData[3], MACTableData[4], MACTableData[5], MACTableData[6], MACTableData[7]);
		}
		else {
			//If this is the first entry checked, the static MAC table is empty. Return control to the user.
			if (current_entry == 0x00) {
				UARTprintf("\n==== NO ENTRIES FOUND IN STATIC MAC TABLE ====\n");
				return true;
			}
		}

	}
	UARTprintf("\n==== END OF STATIC MAC TABLE ====\n");

	return true;
}

//*****************************************************************************
//
//! Show Current Dynamic MAC Table (for Command-Line Interface)
//! Displays all valid entries in the Micrel KSZ8895MLUB Ethernet Controller's
//! dynamic MAC table. In order to obtains these values, the function accessess
//! the indirect registers in the KSZ8895MLUB. Information regarding these
//! registers can be found in the datasheet for this part.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowDynamicMACTable(char *params[MAX_PARAMS]) {
	//Setup indirect access registers
	//We want to READ (bit 4 of 0x6E = 1) the DYNAMIC MAC table (bits [3:2] of 0x6E = 10)
	uint8_t reg6EBase = 0x18;

	//And check each entry in the MAC table from 0x00 to 0x3FF (10 bits)
	uint16_t current_entry = 0x00;
	uint16_t total_entries = 0x400;

	for (current_entry = 0x00; current_entry < total_entries; current_entry++) {

		while (UARTTxBytesFree() < 100) {
			uint32_t taskDelay = LONG_RUNNING_TASK_DLY;
			uint32_t currentTime = xTaskGetTickCount();
			vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
		}
		//Data for each entry will be held here
		uint32_t MACTableData[9] = {0x00};

		//Set reg6E to read <current_entry [8:9]>
		EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_0,(reg6EBase | ((current_entry & 0x300) >> 8)));

		//Set register 111 (0x6F [Indirect Access Control 1]) to the remaining 8 bits of <current_entry [7:0]>
		EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1,(current_entry & 0xFF));

		//This will start a read of <current_entry>
		EthoControllerBulkRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_8,9,MACTableData);

		//MSB of first 8 bits will tell us if the MAC table is empty or not
		if ((MACTableData[0] >> 7) & 1) {
			UARTprintf("\n==== NO ENTRIES FOUND IN DYNAMIC MAC TABLE ====\n");
			return true;
		}

		//Recheck number of valid entries in MAC table. It's possible the user may unplug a port before we're done processing. If so, let's exit this loop!
		total_entries = ((MACTableData[0] & 0x7F) << 3) | ((MACTableData[1] & 0xE0) >> 5) + 1;
		if (current_entry > total_entries) {
			//The table shrinked, exit now!
			UARTprintf("\n ==== END OF TABLE \n ====\n");
			return true;
		}

		//An entry can still be changing while we're processing, so let's pend on it being complete. This is held in bit 8 of MACTableData[2]
		while ((MACTableData[2] >> 7) & 1) {
			//Waiting for entry to become valid
		}

		//This entry is valid and we want to show it to the user!
		if (current_entry == 0x00) {
			//If this is the first entry checked, print the table header
			UARTprintf("\n\t== MAC ADDRESS ==\t == SOURCE PORT ==\t == FILTER ID ==\n");
		}

		//Print MAC Address
		UARTprintf("\t%02X:%02X:%02X:%02X:%02X:%02X\t\t", MACTableData[3], MACTableData[4], MACTableData[5], MACTableData[6], MACTableData[7], MACTableData[8]);

		//Print Source Port
		switch (MACTableData[1] & 0x07) {
			case 0x00:
				UARTprintf("f3");
				break;
			case 0x01:
				UARTprintf("f2");
				break;
			case 0x02:
				UARTprintf("f1");
				break;
			case 0x03:
				UARTprintf("f0");
				break;
			case 0x04:
				UARTprintf("exp-port");
				break;
		}

		//Print Filter ID
		UARTprintf("\t\t\t%d\n",(MACTableData[2] & 0x7F));
	}
	UARTprintf("\n==== END OF DYNAMIC MAC TABLE ====\n");

	return true;
}


//*****************************************************************************
//
//! Send an I2C Command (for Command-Line Interface)
//! Allows the user to modify other layers using the I2C interface. To do this,
//! refer to "i2c_task.h" for valid I2C commands and how to send parameters. This
//! command will only send additional parameters up to that defined in MAX_PARAMS.
//! Once a valid command has been sent, the command line will inform the user what
//! the result of the operation was (1: success, 0: failed).
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_I2CSend(char *params[MAX_PARAMS]) {
	uint8_t command = (uint8_t)strtol(params[0],NULL,16);
	uint8_t optional_params[MAX_PARAMS - 1] = {0x00};
	int i = 0, j = 0;

	//Convert all entered parameters into unsigned 8-bit values until a NULL is found
	for (i = 0; i < (MAX_PARAMS - 1); i++) {
		if (params[i + 1] != 0x00)
			optional_params[i] = (uint8_t)strtol(params[i + 1],NULL,16);
		else
			break;
	}

	//Send the I2C command to the designated slave
	I2CMasterSlaveAddrSet(I2C_BASE_ADDR, I2C_DEVICE_ADDR, false);
	I2CMasterDataPut(I2C_BASE_ADDR, command);
	I2CMasterControl(I2C_BASE_ADDR, I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusy(I2C_BASE_ADDR));

	//Send all parameters for this command (if applicable)
	for (j = 0; j < (i-1); j++ ) {
		I2CMasterDataPut(I2C_BASE_ADDR, optional_params[j]);
		I2CMasterControl(I2C_BASE_ADDR, I2C_MASTER_CMD_SINGLE_SEND);
		while(I2CMasterBusy(I2C_BASE_ADDR));
	}

	//Recieve the result of the I2C operation. (Read back by I2C Interrupt)
	I2CMasterSlaveAddrSet(I2C_BASE_ADDR, I2C_DEVICE_ADDR, true);
	I2CMasterControl(I2C_BASE_ADDR, I2C_MASTER_CMD_SINGLE_RECEIVE);
	return true;
}


//*****************************************************************************
//
//! Logs Out From Switch CLI (for Command-Line Interface)
//! Logs the current user out of the command line and returns to a login prompt.
//! Also logs the UserLoggedOut event to EEPROM (if enabled).
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_Logout(char *params[MAX_PARAMS]) {
	UARTprintf("\033[2J\033[0m\n");
	Authenticated = false;

	LogItemEEPROM(UserLoggedOut);
	return true;
}

//*****************************************************************************
//
//! Create A Task Progress Bar (for Command-Line Interface)
//! Prints the beginning of a progress bar to the CLI and returns an initilized
//! integer to the user. This value should be passed by reference to the
//! UpdateProgressBar function when a change to the progress bar is needed.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns a value of zero to start empty progress bar.
//
//*****************************************************************************
int CreateProgressBar() {
	int progress = 0;
	UARTprintf("\033[2K\033[100D\033[34;47mTask Progress: [");
	UARTprintf("\033[s");
	return progress;
}
//*****************************************************************************
//
//! Update A Task Progress Bar (for Command-Line Interface)
//! Changes the current state of a progress bar by either incrementing, decrementing,
//! the current value. Once updated, the value of lastprogress is updated so that
//! the function can keep track of how much the progress bar has changed.
//
//*****************************************************************************
void UpdateProgressBar(int* lastprogress, PBarActions action, int newvalue) {
	int i = 0;
	if (*lastprogress >= 100 && action != Fill) {
		return;
	}
	UARTprintf("\033[u");
	switch (action) {
	case Reset:
		UARTprintf("\033[2K\033[100D\033[34;47mTask Progress: [");
		*lastprogress = 0;
		break;
	case Fill:
		*lastprogress = 100;
		break;
	}

	if (action != FillError) {
		for (i = 0; i < ((newvalue/2)-(*lastprogress/2)); i++) {
			UARTprintf("#");
		}
		UARTprintf("\033[s");
		*lastprogress = newvalue;
		for (i = 0; i < (50-(*lastprogress/2));i++) {
			UARTprintf(" ");
		}
	}
	else {
		UARTprintf("\033[2K\033[100DTask Progress: [");
		for (i = 0; i < (50); i++) {
			UARTprintf("!");
		}
	}
	UARTprintf("]\033[0m");
}

//*****************************************************************************
//
//! [DEPRECATED] Show Progress Bar (for Command-Line Interface)
//! Shows a simple progress bar for reporting task progress to the user.
//! [since v. 0.1.32] Replaced with CreateProgressBar and UpdateProgressBar to
//! remove unecessary writing to the CLI. Old progress bar erased the entire line
//! and redrew the progress bar (highly inefficient). The new functions only change
//! the part of the progress bar that is indicated by the developer.
//
//*****************************************************************************
void ShowProgress(int percent)
{	int i = 0;
	if (percent > 0) {
		UARTprintf("\033[2K\033[100D\033[34;47mTask Progress: [");
		for (i = 0; i < (percent/2); i++) {
			UARTprintf("#");
		}
		for (i = 0; i < (50-(percent/2));i++) {
			UARTprintf(" ");
		}
		UARTprintf("]\033[0m");
	}
	else {
		UARTprintf("\033[2K\033[100DTask Progress: [");
		for (i = 0; i < (50); i++) {
			UARTprintf("!");
		}
		UARTprintf("]\033[0m");
	}
}

//*****************************************************************************
//
//! SPECIAL FUNCTION FOR USE BY VLAN COMMANDS
//! The KSZ8895MLUB contains a register for each port that has a 5-bit VLAN mask
//! where each MSB ==> LSB set high indicates that port is on the same VLAN as the
//! current port. (i.e. set ports 4 and 3 to the same VLAN) <br>
//! PORT 4 (VLAN MASK): 01100 <== This port now knows port 3 is on the same VLAN <br>
//! PORT 3 (VLAN MASK): 01100 <== This port now knows port 4 is on the same VLAN <br>
//! Therefore, this function finds and updates all ports on the current VLAN ID. <br>
//! \param vlan_id	the VLAN currently being configured (0 - 4096)
//! \param port_id	the 32-bit port ID specified in interpreter.h
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t AssertVLANS(uint32_t vlan_id, uint32_t port_id) {

	uint8_t port_base_addrs[4] = {ETHO_PORT1_HARDWARE_HEX, ETHO_PORT2_HARDWARE_HEX, ETHO_PORT3_HARDWARE_HEX, ETHO_PORT4_HARDWARE_HEX};
	uint8_t  mask = 0x00, reg_data = 0x00;
	uint16_t vlan_data = 0x00;
	int i = 0;

	for (i = 0; i < 4; i++) {
		vlan_data = 0x00;
		vlan_data |= (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL3_OFFSET_HEX + port_base_addrs[i])) & 0xF) << 8;
		vlan_data |= (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL4_OFFSET_HEX + port_base_addrs[i])) & 0xFF) << 0;
		//Get this port's mask
		reg_data = (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, (PORT_CONTROL1_OFFSET_HEX + port_base_addrs[i])));

		if (vlan_data == vlan_id) {
			mask |= (1 << i);
			reg_data |= (1 << FindIndex(port_base_addrs, 4, port_id));
			EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,(PORT_CONTROL1_OFFSET_HEX + port_base_addrs[i]), reg_data);
		}
	}

	return mask;
}

size_t FindIndex(uint8_t a[], size_t size, int value )
{
    size_t index = 0;

    while ( index < size && a[index] != value ) ++index;

    return index;
}



