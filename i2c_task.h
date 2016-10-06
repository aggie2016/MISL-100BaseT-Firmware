/**\file i2c_task.h
 * \brief <b> Settings and Function Prototypes for I2C Interpreter Task</b>
 *
 *
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

#include "command_functions.h"
#include "interpreter_task.h"

#ifndef I2C_TASK_H_
#define I2C_TASK_H_

//*****************************************************************************
//
//! Just like the UART CLI Interpreter Task, the I2C task has a maximum number
//! of acceptable parameters set to 20.
//
//*****************************************************************************
#define MAX_PARAMS 			20
//*****************************************************************************
//
//! The maximum number of I2C commands that can be stored is 50.
//
//*****************************************************************************
#define I2CBUFFERSIZE		50
//*****************************************************************************
//
//! The current maximum command code implemented. The developer should change
//! this value if commands are added or removed!
//
//*****************************************************************************
#define MAX_I2C_COMMAND		0x50

//*****************************************************************************
//
//! The stack size for the LED toggle task.
//
//*****************************************************************************
#define I2CTASKSTACKSIZE        900         // Stack size in words

//*****************************************************************************
//
//! The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define I2C_ITEM_SIZE           sizeof(I2C_Packet)
#define I2C_QUEUE_SIZE          5

#define POLL_SEMAPHORE			0
#define I2C_SLAVE_SEND_DLY		40

//! \brief I2CManager queue item. Used to pass commands in real-time from I2C port to the
//! FreeRTOS interpreter.
//!
//! Values placed in I2CRXBuffer are read sequentially to ensure that a valid command has been
//! issued over the I2C port specified in freertos_init.h. The current value of I2CRXIndex is
//! used to indicate the current number of values held in the I2CRXBuffer.
typedef struct I2CPacket {
	//! \brief Parameters passed from I2C interrupt to the I2CManager task.
	//! The item in position 0 should always be the I2C command code as set in I2C_Mappings
	//! Any items after this 8-bit value are interpreted as function parameters.
	uint8_t I2CRXBuffer[I2CBUFFERSIZE];
	//! Number of parameters contained inside the buffer.
	uint8_t I2CRXIndex;
} I2C_Packet;

extern uint32_t I2CManagerTaskInit(void);

//! \brief I2C interpretation structure. Contains I2C expected code along with the number
//! of custom, static, and returned parameters.
//!
//! Addition of records to the I2C_Mappings table should implement ALL of these properties.
//! The number of static parameters (those defined in the "static_parameters" array) should
//! be identifed in static_pcount. The number of custom parameters to be sent by the master
//! over I2C should be identified in "custom_pcount". The value placed in "return_pcount"
//! will be sent over I2C when calling a command to allow the master to know how many values
//! will be returned after execution.
typedef struct I2CCodes {
	//! The I2C command value in hex. Max number of entries limited to 256
	uint8_t command_code;
	//! The number of parameters statically defined in "static_parameters"
	uint8_t static_pcount;
	//! The number of custom parameters expected from the master during communication
	uint8_t custom_pcount;
	//! The number of values this function should return to the master
	uint8_t return_pcount;
	//! Predefined values used as input to the attached function pointer.
	uint8_t static_parameters[MAX_PARAMS];
	//! A function to call for this I2C command. Function must accept a pointer-to-char array
	uint8_t (*func)(uint8_t*);
} I2C_Codes;

uint8_t I2CNotImplementedFunction(uint8_t params[MAX_PARAMS]);

static const I2C_Codes I2C_Mappings[80] = {

	//SYSTEM COMMANDS
	// Read or Write a value to the Ethernet Controller
	{0x00,0,3,1,{},I2C_WriteReadFromEthernetController},
	// Save running configuration to EEPROM
	{0x01,0,0,1,{},I2C_SaveSwitchConfiguration},
	// Download running configuration from EEPROM
	{0x02,0,0,0xFF,{},I2C_DownloadSwitchConfiguration},
	// Clear running configuration from EEPROM
	{0x03,0,0,1,{},I2C_ClearSwitchConfiguration},
	// Upload running configuration to EEPROM
	{0x04,0,0,0,{},I2CNotImplementedFunction},
	// Reset the Ethernet Controller
	{0x05,0,0,0,{},I2CNotImplementedFunction},
	// Reset MISL Switch Layer
	{0x06,0,0,0,{},I2CNotImplementedFunction},
	{0x07,0,0,0,{},I2CNotImplementedFunction},
	{0x08,0,0,0,{},I2CNotImplementedFunction},
	{0x09,0,0,0,{},I2CNotImplementedFunction},
	{0x0A,0,0,0,{},I2CNotImplementedFunction},
	{0x0B,0,0,0,{},I2CNotImplementedFunction},
	{0x0C,0,0,0,{},I2CNotImplementedFunction},
	{0x0D,0,0,0,{},I2CNotImplementedFunction},
	{0x0E,0,0,0,{},I2CNotImplementedFunction},
	{0x0F,0,0,0,{},I2CNotImplementedFunction},

	//QUICK ETHERNET PORT 1 CONTROL COMMANDS
	// Turn on port 1
	{0x10,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_ClearBitEthernetController},
	// Turn off port 1
	{0x11,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_SetBitEthernetController},
	// Set port 1 to full-duplex
	{0x12,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_SetBitEthernetController},
	// Set port 1 to half-duplex
	{0x13,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_ClearBitEthernetController},
	// Set port 1 to 10BaseT (10Mbps)
	{0x14,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_ClearBitEthernetController},
	// Set port 1 to 100BaseT (100Mbps)
	{0x15,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_SetBitEthernetController},
	// Enable Auto MDI/MDI-X
	{0x16,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Disable Auto MDI/MDI-X
	{0x17,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Restart Auto-Negotiation
	{0x18,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x05},I2C_SetSCEthernetController},
	// Turn packet transmission on
	{0x19,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Turn packet transmission off
	{0x1A,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Turn packet reception on
	{0x1B,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_SetBitEthernetController},
	// Turn packet recption off
	{0x1C,3,0,1,{PORT1_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_ClearBitEthernetController},
	// Run port 1 diagnostics
	{0x1D,0,0,2,{},I2CNotImplementedFunction},
	// Set port 1 VLAN
	{0x1E,0,0,1,{},I2CNotImplementedFunction},
	// Retrieve port 1 status
	{0x1F,0,0,1,{},I2CNotImplementedFunction},


	//QUICK ETHERNET PORT 2 CONTROL COMMANDS
	// Turn on port 2
	{0x20,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_ClearBitEthernetController},
	// Turn off port 2
	{0x21,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_SetBitEthernetController},
	// Set port 2 to full-duplex
	{0x22,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_SetBitEthernetController},
	// Set port 2 to half-duplex
	{0x23,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_ClearBitEthernetController},
	// Set port 2 to 10BaseT (10Mbps)
	{0x24,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_ClearBitEthernetController},
	// Set port 2 to 100BaseT (100Mbps)
	{0x25,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_SetBitEthernetController},
	// Enable Auto MDI/MDI-X
	{0x26,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Disable Auto MDI/MDI-X
	{0x27,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Restart Auto-Negotiation
	{0x28,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x05},I2C_SetSCEthernetController},
	// Turn packet transmission on
	{0x29,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Turn packet transmission off
	{0x2A,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Turn packet reception on
	{0x2B,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_SetBitEthernetController},
	// Turn packet recption off
	{0x2C,3,0,1,{PORT2_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_ClearBitEthernetController},
	// Run port 2 diagnostics
	{0x2D,0,0,2,{},I2CNotImplementedFunction},
	// Set port 2 VLAN
	{0x2E,0,0,1,{},I2CNotImplementedFunction},
	// Retrieve port 2 status
	{0x2F,0,0,1,{},I2CNotImplementedFunction},


	//QUICK ETHERNET PORT 3 CONTROL COMMANDS
	// Turn on port 3
	{0x30,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_ClearBitEthernetController},
	// Turn off port 3
	{0x31,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_SetBitEthernetController},
	// Set port 3 to full-duplex
	{0x32,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_SetBitEthernetController},
	// Set port 3 to half-duplex
	{0x33,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_ClearBitEthernetController},
	// Set port 3 to 10BaseT (10Mbps)
	{0x34,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_ClearBitEthernetController},
	// Set port 3 to 100BaseT (100Mbps)
	{0x35,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_SetBitEthernetController},
	// Enable Auto MDI/MDI-X
	{0x36,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Disable Auto MDI/MDI-X
	{0x37,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Restart Auto-Negotiation
	{0x38,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x05},I2C_SetSCEthernetController},
	// Turn packet transmission on
	{0x39,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Turn packet transmission off
	{0x3A,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Turn packet reception on
	{0x3B,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_SetBitEthernetController},
	// Turn packet recption off
	{0x3C,3,0,1,{PORT3_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_ClearBitEthernetController},
	// Run port 3 diagnostics
	{0x3D,0,0,2,{},I2CNotImplementedFunction},
	// Set port 3 VLAN
	{0x3E,0,0,1,{},I2CNotImplementedFunction},
	// Retrieve port 3 status
	{0x3F,0,0,1,{},I2CNotImplementedFunction},


	//QUICK ETHERNET PORT 4 CONTROL COMMANDS
	// Turn on port 4
	{0x40,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_ClearBitEthernetController},
	// Turn off port 4
	{0x41,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x03},I2C_SetBitEthernetController},
	// Set port 4 to full-duplex
	{0x42,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_SetBitEthernetController},
	// Set port 4 to half-duplex
	{0x43,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x05},I2C_ClearBitEthernetController},
	// Set port 4 to 10BaseT (10Mbps)
	{0x44,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_ClearBitEthernetController},
	// Set port 4 to 100BaseT (100Mbps)
	{0x45,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL5_OFFSET_HEX,0x06},I2C_SetBitEthernetController},
	// Enable Auto MDI/MDI-X
	{0x46,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Disable Auto MDI/MDI-X
	{0x47,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Restart Auto-Negotiation
	{0x48,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL6_OFFSET_HEX,0x05},I2C_SetSCEthernetController},
	// Turn packet transmission on
	{0x49,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_SetBitEthernetController},
	// Turn packet transmission off
	{0x4A,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x02},I2C_ClearBitEthernetController},
	// Turn packet reception on
	{0x4B,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_SetBitEthernetController},
	// Turn packet recption off
	{0x4C,3,0,1,{PORT4_OFFSET_HEX,PORT_CONTROL2_OFFSET_HEX,0x01},I2C_ClearBitEthernetController},
	// Run port 4 diagnostics
	{0x4D,0,0,2,{},I2CNotImplementedFunction},
	// Set port 4 VLAN
	{0x4E,0,0,1,{},I2CNotImplementedFunction},
	// Retrieve port 4 status
	{0x4F,0,0,1,{},I2CNotImplementedFunction},

};


#endif /* I2C_TASK_H_ */
