/**\file interpreter_task.h
 * \brief <b>UART CLI Interpreter Task Command Definitions and Additional System Settings</b>
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
#ifndef INTERPRETER_TASK_H_
#define INTERPRETER_TASK_H_

#include "command_functions.h"
#include "freertos_init.h"

//! Maximum number of registers for each port. Size this value to the number of array entires needed in the PORT_CONFIG_MAPPINGS array.
#define PORT_MAX_MAPPINGS 10
//! Maximum number of possible options for each masked register
#define MAX_OPTIONS 9
//! Maximum number of value/string pairs for each option
#define MAX_VALUES 5

//! Maximum number of mappings for global switch settings. Size this value to the number of array entires needed in the GLOBAL_CONFIG_MAPPINGS array.
#define GLOBAL_CONFIG_MAX_MAPPINGS 7

//! Placeholders for command flags. Use to improve readability of new commands
#define NO_PARAMETERS  0
#define TERMINATING_COMMMAND true
#define HAS_CHILD  false
#define NO_CHILD_MENU 0

//! Maximum number of children to search for
#define MAX_DEPTH_INHERITANCE 12
//! Maximum number of total menu items in each command menu. [IMPORTANT]: Should be LARGER than the LARGEST command menu!
#define MAX_MENU_ITEMS 50
//! Maximum number of statically defined parameters in each menu item
#define MAX_PARAMS 20
//! Placeholder for no statically defined parameters. Use to improve readability of new commands.
#define EMPTY_STATIC_PARAMS {""}

/** @defgroup PBASE_OFFSET Base Hardware Register Definitions
 *  @brief Base register addresseses for each physical port. Do not change these values
 *  @{
 */
#define ETHO_PORT1_HARDWARE 						"0x10"
#define ETHO_PORT2_HARDWARE 						"0x20"
#define ETHO_PORT3_HARDWARE 						"0x30"
#define ETHO_PORT4_HARDWARE 						"0x40"
/** @} */

/** @defgroup PBASE_OFFSET_HEX Hexadecimal Base Hardware Register Definitions
 *  @brief Hexadecimal base register addresseses for each physical port. Do not change these values
 *  @{
 */
#define ETHO_PORT1_HARDWARE_HEX 					0x10
#define ETHO_PORT2_HARDWARE_HEX 					0x20
#define ETHO_PORT3_HARDWARE_HEX 					0x30
#define ETHO_PORT4_HARDWARE_HEX 					0x40
/** @} */


/** @defgroup POFFSET_LOGICAL Port Offset Register Definitions
 *  @brief Base register addresses for each logical port. Change these to match your hardware layout if necessary.
 *  MISL Switch Layer (v1): All physical ports are inverted logically (i.e. PHY 1 is controlled in the command line as logical port 4)
 *  @{
 */
#define PORT1_OFFSET 								ETHO_PORT4_HARDWARE
#define PORT2_OFFSET 								ETHO_PORT3_HARDWARE
#define PORT3_OFFSET 								ETHO_PORT2_HARDWARE
#define PORT4_OFFSET 								ETHO_PORT1_HARDWARE
/** @} */

/** @defgroup POFFSET_HEX Hexadecimal Port Offset Register Definitions
 *  @brief Hexadecimal Base register addresses for each logical port. Change these to match your hardware layout if necessary.
 *  MISL Switch Layer (v1): All physical ports are inverted logically (i.e. PHY 1 is controlled in the command line as logical port 4)
 *  @{
 */
#define PORT1_OFFSET_HEX 							ETHO_PORT4_HARDWARE_HEX
#define PORT2_OFFSET_HEX 							ETHO_PORT3_HARDWARE_HEX
#define PORT3_OFFSET_HEX 							ETHO_PORT2_HARDWARE_HEX
#define PORT4_OFFSET_HEX 							ETHO_PORT1_HARDWARE_HEX
/** @} */

/** @defgroup PC0_OFFSET Port Control 0 Register Definitions
 *  @brief Port Control Register 0 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL0_OFFSET 						"0x0"
#define PORT_CONTROL0_OFFSET_HEX 					0x0

//! Port Control Register 0 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL0 								PORT1_OFFSET
#define PORT2_CONTROL0 								PORT2_OFFSET
#define PORT3_CONTROL0 								PORT3_OFFSET
#define PORT4_CONTROL0 								PORT4_OFFSET
/** @} */

/** @defgroup PC1_OFFSET Port Control 1 Register Definitions
 *  @brief Port Control Register 1 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL1_OFFSET 						"0x1"
#define PORT_CONTROL1_OFFSET_HEX 					0x1

//! Port Control Register 1 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL1 								(PORT1_OFFSET + PORT_CONTROL1_OFFSET_HEX)
#define PORT2_CONTROL1 								(PORT2_OFFSET + PORT_CONTROL1_OFFSET_HEX)
#define PORT3_CONTROL1 								(PORT3_OFFSET + PORT_CONTROL1_OFFSET_HEX)
#define PORT4_CONTROL1 								(PORT4_OFFSET + PORT_CONTROL1_OFFSET_HEX)
/** @} */


/** @defgroup PC2_OFFSET Port Control 2 Register Definitions
 *  @brief Port Control Register 2 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL2_OFFSET 						"0x2"
#define PORT_CONTROL2_OFFSET_HEX 					0x2

//! Port Control Register 2 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL2 								(PORT1_OFFSET + PORT_CONTROL2_OFFSET_HEX)
#define PORT2_CONTROL2 								(PORT2_OFFSET + PORT_CONTROL2_OFFSET_HEX)
#define PORT3_CONTROL2 								(PORT3_OFFSET + PORT_CONTROL2_OFFSET_HEX)
#define PORT4_CONTROL2 								(PORT4_OFFSET + PORT_CONTROL2_OFFSET_HEX)
/** @} */

/** @defgroup PC3_OFFSET Port Control 3 Register Definitions
 *  @brief Port Control Register 3 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL3_OFFSET 						"0x3"
#define PORT_CONTROL3_OFFSET_HEX 					0x3

//! Port Control Register 3 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL3 								(PORT1_OFFSET + PORT_CONTROL3_OFFSET_HEX)
#define PORT2_CONTROL3 								(PORT2_OFFSET + PORT_CONTROL3_OFFSET_HEX)
#define PORT3_CONTROL3 								(PORT3_OFFSET + PORT_CONTROL3_OFFSET_HEX)
#define PORT4_CONTROL3 								(PORT4_OFFSET + PORT_CONTROL3_OFFSET_HEX)
/** @} */

/** @defgroup PC4_OFFSET Port Control 4 Register Definitions
 *  @brief Port Control Register 4 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL4_OFFSET 						"0x4"
#define PORT_CONTROL4_OFFSET_HEX 					0x4

//! Port Control Register 4 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL4 								(PORT1_OFFSET + PORT_CONTROL4_OFFSET_HEX)
#define PORT2_CONTROL4 								(PORT2_OFFSET + PORT_CONTROL4_OFFSET_HEX)
#define PORT3_CONTROL4 								(PORT3_OFFSET + PORT_CONTROL4_OFFSET_HEX)
#define PORT4_CONTROL4 								(PORT4_OFFSET + PORT_CONTROL4_OFFSET_HEX)
/** @} */

/** @defgroup PC5_OFFSET Port Control 5 Register Definitions
 *  @brief Port Control Register 5 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL5_OFFSET 						"0xC"
#define PORT_CONTROL5_OFFSET_HEX 					0xC

//! Port Control Register 5 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL5 								(PORT1_OFFSET + PORT_CONTROL5_OFFSET_HEX)
#define PORT2_CONTROL5 								(PORT2_OFFSET + PORT_CONTROL5_OFFSET_HEX)
#define PORT3_CONTROL5 								(PORT3_OFFSET + PORT_CONTROL5_OFFSET_HEX)
#define PORT4_CONTROL5 								(PORT4_OFFSET + PORT_CONTROL5_OFFSET_HEX)
/** @} */


/** @defgroup PC6_OFFSET Port Control 6 Register Definitions
 *  @brief Port Control Register 6 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_CONTROL6_OFFSET 						"0xD"
#define PORT_CONTROL6_OFFSET_HEX 					0xD

//! Port Control Register 6 offset for KSZ8895MLUB. Do not change these values
#define PORT1_CONTROL6 								(PORT1_OFFSET + PORT_CONTROL6_OFFSET)
#define PORT2_CONTROL6 								(PORT2_OFFSET + PORT_CONTROL6_OFFSET)
#define PORT3_CONTROL6 								(PORT3_OFFSET + PORT_CONTROL6_OFFSET)
#define PORT4_CONTROL6 								(PORT4_OFFSET + PORT_CONTROL6_OFFSET)
/** @} */

/** @defgroup PLK0_OFFSET Port LinkMD 0 Register Definitions
 *  @brief Port LINKMD Register 0 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_LINKMD0_OFFSET 						"0xA"
#define PORT_LINKMD0_OFFSET_HEX 					0xA

//! Port LINKMD Register 0 offset for KSZ8895MLUB. Do not change these values
#define PORT1_LINKMD0 								(PORT1_OFFSET + PORT_LINKMD0_OFFSET_HEX)
#define PORT2_LINKMD0 								(PORT2_OFFSET + PORT_LINKMD0_OFFSET_HEX)
#define PORT3_LINKMD0 								(PORT3_OFFSET + PORT_LINKMD0_OFFSET_HEX)
#define PORT4_LINKMD0 								(PORT4_OFFSET + PORT_LINKMD0_OFFSET_HEX)
/** @} */


/** @defgroup PLK1_OFFSET Port LinkMD 1 Register Definitions
 *  @brief Port LINKMD Register 1 offset for KSZ8895MLUB. Change as needed to match your hardware. Both offset and offset_hex values should be the same.
 *  @{
 */
#define PORT_LINKMD1_OFFSET 						"0xB"
#define PORT_LINKMD1_OFFSET_HEX 					0xB

//! Port LINKMD Register 1 offset for KSZ8895MLUB. Do not change these values
#define PORT1_LINKMD1 								(PORT1_OFFSET + PORT_LINKMD1_OFFSET_HEX)
#define PORT2_LINKMD1 								(PORT2_OFFSET + PORT_LINKMD1_OFFSET_HEX)
#define PORT3_LINKMD1 								(PORT3_OFFSET + PORT_LINKMD1_OFFSET_HEX)
#define PORT4_LINKMD1 								(PORT4_OFFSET + PORT_LINKMD1_OFFSET_HEX)
/** @} */


/** @defgroup PS0_OFFSET Port Status 0 Register Definitions
 *  @brief Port Status 0 register offsets for KSZ8895MLUB. Most values held inside these registers are read-only.
 *  @{
 */
#define PORT_STATUS0_OFFSET_HEX 					0x9
#define PORT1_STATUS0 								(PORT1_OFFSET + PORT_STATUS0_OFFSET_HEX)
#define PORT2_STATUS0 								(PORT2_OFFSET + PORT_STATUS0_OFFSET_HEX)
#define PORT3_STATUS0 								(PORT3_OFFSET + PORT_STATUS0_OFFSET_HEX)
#define PORT4_STATUS0 								(PORT4_OFFSET + PORT_STATUS0_OFFSET_HEX)
/** @} */

/** @defgroup PS1_OFFSET Port Status 1 Register Definitions
 *  @brief Port Status 1 register offsets for KSZ8895MLUB. Most values held inside these registers are read-only.
 *  @{
 */
#define PORT_STATUS1_OFFSET_HEX 					0xE
#define PORT1_STATUS1 								(PORT1_OFFSET + PORT_STATUS1_OFFSET_HEX)
#define PORT2_STATUS1 								(PORT2_OFFSET + PORT_STATUS1_OFFSET_HEX)
#define PORT3_STATUS1 								(PORT3_OFFSET + PORT_STATUS1_OFFSET_HEX)
#define PORT4_STATUS1 								(PORT4_OFFSET + PORT_STATUS1_OFFSET_HEX)
/** @} */

/** @defgroup PS2_OFFSET Port Status 2 Register Definitions
 *  @brief Port Status 2 register offsets for KSZ8895MLUB. Most values held inside these registers are read-only.
 *  @{
 */
#define PORT_STATUS2_OFFSET_HEX 					0xF
#define PORT1_STATUS2 								(PORT1_OFFSET + PORT_STATUS2_OFFSET_HEX)
#define PORT2_STATUS2 								(PORT2_OFFSET + PORT_STATUS2_OFFSET_HEX)
#define PORT3_STATUS2 								(PORT3_OFFSET + PORT_STATUS2_OFFSET_HEX)
#define PORT4_STATUS2 								(PORT4_OFFSET + PORT_STATUS2_OFFSET_HEX)
/** @} */

/** @defgroup INDIR_OFFSET Indirect Access Register Definitions
 *  @brief Register addresses for accessing VLAN table. Normal access to VLAN table has to be done using indirect register read/write statements.
 *  @{
 */
#define INDIRECT_ACCESS_CONTROL_0 					0x6E
#define INDIRECT_ACCESS_CONTROL_1 					0x6F

#define INDIRECT_CONTROL_READTYPEBIT 				0x04
#define INDIRECT_READTYPE_READ 						1
#define INDIRECT_READTYPE_WRITE 					0

#define INDIRECT_CONTROL_TABLESELECT 				0x02
#define INDIRECT_TABLESELCT_STATICMAC 				0
#define INDIRECT_TABLESELECT_VLAN 					1
#define INDIRECT_TABLESELECT_DYNMAC					2

#define INDIRECT_CONTROL_ADDRESS_HIGH 				0x00
#define INDIRECT_CONTROL_ADDRESS_LOW 				0x00

#define INDIRECT_REGISTER_DATA_8 					0x70
#define INDIRECT_REGISTER_DATA_7 					0x71
#define INDIRECT_REGISTER_DATA_6 					0x72
#define INDIRECT_REGISTER_DATA_5 					0x73
#define INDIRECT_REGISTER_DATA_4 					0x74
#define INDIRECT_REGISTER_DATA_3 					0x75
#define INDIRECT_REGISTER_DATA_2 					0x76
#define INDIRECT_REGISTER_DATA_1 					0x77
#define INDIRECT_REGISTER_DATA_0 					0x78
/** @} */

/** @defgroup GLC_OFFSET Global Control Register Definitions
 *  @brief Base register addresses for KSZ8895MLUB. Change as needed to match your hardware.
 *  @{
 */
#define GLOBAL_CONTROL_0 							"0x02"
#define GLOBAL_CONTROL_0_HEX						0x02
#define GLOBAL_CONTROL_1 							"0x03"
#define GLOBAL_CONTROL_1_HEX						0x03
#define GLOBAL_CONTROL_2							"0x04"
#define GLOBAL_CONTROL_2_HEX						0x04
#define GLOBAL_CONTROL_3 							"0x05"
#define GLOBAL_CONTROL_3_HEX						0x05
#define GLOBAL_CONTROL_9 							"0x0B"
#define GLOBAL_CONTROL_9_HEX						0x0B
/** @} */


#define INTERRUPT_STATUS_REGISTER 0x7C

//**************************************************************************************************************************************
//
// <<<Command Line Options Begin Here>>>
// Each command category has sub categories that will be analyzed using a Parent/Child (Tree) relationship.
//
// Format of Each Command type is:
//		text:					validation string to ensure the user is selecting this entry
// 		help:					information regarding the use of this function and any parameters
// 		isExecutable:			is this a terminating command (no child elements)?
// 		paramsRequired: 		number of static values in command declaration that will be passed to supplied function pointer
//		paramsUserProvided:		are the parameters needed for that attached handler provided by the user?
// 		func: 					function pointer to a handler used when executing this command
// 		functionParams: 		custom parameters to use when calling the handler pointed to by "func"
//		childCommand:			pointer-to-array for the next child that maps to this parent item
//		permissionsRequired:	Enumeration that controls who can use this command. Refer to enum for perm level definitions.

// Hierarchy is
// 	CATEGORY
//      \ COMMAND
//			\OPTIONS
//				\SETTINGS
//
// Since parent commands require access their children, the ordering of all items below is the inverse of the diagram listed above
//**************************************************************************************************************************************


//**************************************************************************************************************************************
//
//! \brief A Command Fragment -- See Detailed Documentation
//!
//!
//! A Command Line Interface (CLI) command fragment. Each of these commands requires
//! a string for the command text, a string to provide help for this parameter,
//! a bool indicating whether this is the end of a complete command,
//! an integer representing the number of parameters required by this fragment,
//! a bool that determines whether or not this command fragment represents a
//! parameter that must be provided by the user (custom, i.e. <hex value> may be 0x01 on the CLI),
//! a function to call (only provide the name, i.e. COM_NewFunction),
//! a set of predefined values that need to be collected when the user types this fragment in,
//! a sub-menu (child command) that makes up the next fragment of the command
//! and a permissions level to use this command.
//
//**************************************************************************************************************************************
typedef struct Command {
	//!Command string that will be entered by the user. Should be a SINGLE (possibly hyphenated) word.
	const char * const text;
	//!Help text that will be displayed when the user appends a '?' after the currently entered command word
	const char * const help;
	//!Designates this command as an executable (terminating) command. Ensure that functions labeled this way have function pointers attached to them
	const bool isExecutable;
	//!Number of statically defined (in the command's definition) or custom (user entered parameters)
	const int paramsRequired;
	//!Designates this command as requiring custom input from the command line. The user will not enter the command text defined but their own input.
	const bool paramsUserProvided;
	//!A function to call when the command is issued. COMMAND MUST BE TERMINATING_COMMMAND TO CALL FUNCTION.
	bool (*func)(char**);
	//!Predefined function parameters that are passed to the function pointer when called. If any custom user input is passed to the command line, it will be combined with these values
	const char *functionParams[15];
	//!A sub-menu to link to this command. If this value is not null, you must set the isExecutable value to HAS_CHILD.
	const struct Command *childCommand;
	//!A permissions level that restricts use of this command to authorized users.
	const PermLevel permissionsRequired;
} Command;

/**
 * @brief Placeholder for commands that have not be setup. Should never be called once
 * command structure is fully complete. For development purposes, if called,
 * we encountered a bug and will wait infinitely for someone to help us!
 * (Lost in thought, please send search party!)
 */
bool NotImplementedFunction(char* x[MAX_PARAMS]);
uint32_t InterpreterTaskInit(void);

//***********************************************************************************
//
//! Generic shared command for enabling and disabling an option:
//! Will call either the Set or ClearBit function that modified a single register to either 0 or 1.
//! Enable in this case sets the register bit in question to 1.
//! Disable in this case sets the register bit in question to 0.
//! Requires a parent menu item that passes the address of the register to use.
//
//********************************************************************************************
static const Command Enable_Disable_Options[3] = {
		{"enable", 	"enable this option", 	TERMINATING_COMMMAND, 	1,	false, 	COM_SetBitEthernetController, 	{"Enabling Feature..."},	NO_CHILD_MENU,	ModifyPortsOnly},
		{"disable", "disable this option", 	TERMINATING_COMMMAND, 	1,	false, 	COM_ClearBitEthernetController, {"Disabling Feature..."},	NO_CHILD_MENU,	ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};
//*******************************************************************************************************************
//
//! INVERTED Generic shared command for enabling and disabling an option:
//! Will call either the Set or ClearBit function that modified a single register to either 0 or 1.
//! Enable in this case sets the register bit in question to 0.
//! Disable in this case sets the register bit in question to 1.
//! Requires a parent menu item that passes the address of the register to use.
//
//********************************************************************************************************************
static const Command INV_Enable_Disable_Options[3] = {
		{"enable", 	"enable this option", 	TERMINATING_COMMMAND, 	1,	false, 	COM_ClearBitEthernetController, {"Enabling Feature..."},	NO_CHILD_MENU,	ModifyPortsOnly},
		{"disable", "disable this option", 	TERMINATING_COMMMAND, 	1,	false, 	COM_SetBitEthernetController, 	{"Disabling Feature..."},	NO_CHILD_MENU,	ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};

//********************************************************************************************
//
//! Generic shared command for setting the bitrate of a port.
//! Calls the SetSystemFlag function that modifies the EEPROM Global Setting Sector (0x0001E)
//! Requires a parent menu item that passes the register to use.
//
//********************************************************************************************
static const Command Bitrate_Settings[2] = {
		{"<speed [bits/sec]>", "manually set the speed for this port", TERMINATING_COMMMAND, 1,true, NotImplementedFunction, EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};
//************************************************************************************************************
//
//! Commands for modifying registers on the EEPROM through the CLI
//! Calls the WriteRegister function that requires both an device, address, and data to use.
//! Requires 2 parent menu items that defines the register address and device to modify.
//
//************************************************************************************************************
static const Command WriteEEPROMRegister_Options2[2] = {
		{"<data [0x00000000 - 0xFFFFFFFF]>", "read a setting from a register on the selected KSZ8895MQX", TERMINATING_COMMMAND, 1,true, COM_WriteToEEPOM, EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ModifySystem},
		{0,0,0,0,0,0,0}
};
static const Command WriteEEPROMRegister_Options[2] = {
		{"<register-addr [0x00000000 - 0xFFFFFFFF]>", "read a setting from a register on the selected KSZ8895MQX", HAS_CHILD, 1,true, NotImplementedFunction, EMPTY_STATIC_PARAMS,WriteEEPROMRegister_Options2,	ModifySystem},
		{0,0,0,0,0,0,0}
};
static const Command ReadEEPROMRegister_Options[2] = {
		{"<register-addr [0x00 - 0xFF]>", "read a setting from a register on the selected KSZ8895MQX", TERMINATING_COMMMAND, 1,true, COM_ReadFromEEPROM, EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ReadOnlyUser},
		{0,0,0,0,0,0,0}
};
//************************************************************************************************************
//
//! Parent commands for modifying and reinitializing registers on the EEPROM through the CLI
//! Reinitialize calls the ReinitializeEEPROM function.
//
//************************************************************************************************************
static const Command EEPROM_Options[5] = {
		{"read-reg", 		"read a register from the EEPROM", 							HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	ReadEEPROMRegister_Options,		ReadOnlyUser},
		{"write-reg", 		"write to a register on the EEPROM", 						HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	WriteEEPROMRegister_Options,	ModifySystem},
		{"reinitialize", 	"reset the EEPROM to factory settings [RESTART REQUIRED]", 	TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ReinitializeEEPROM, 	EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,					ModifySystem},
		{0,0,0,0,0,0,0}
};
//*******************************************************************************************************************
//
//! Commands for changing the format for the activity, duplex, and power LEDs on all ports
//
//********************************************************************************************************************
static const Command LED_Options[3] = {
		{"mode-0", 	"set port LEDs to use mode 0", 	TERMINATING_COMMMAND, 	1,	false, 	COM_ClearBitEthernetController, {"Setting LEDs To Mode 0"},	NO_CHILD_MENU,	ModifyPortsOnly},
		{"mode-1", 	"set port LEDs to use mode 1", 	TERMINATING_COMMMAND, 	1,	false, 	COM_SetBitEthernetController, 	{"Setting LEDs To Mode 1"},	NO_CHILD_MENU,	ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};

static const Command Table_Options[4] = {
		{"vlan-table", 			"shows the current VLAN table", 						TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ShowVLANTable, 			EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,					ReadOnlyUser},
		{"static-mac-table",	"shows the static MAC table", 							TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ShowStaticMACTable, 	EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,					ReadOnlyUser},
		{"dyn-mac-table", 		"shows the dynamic MAC table", 							TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ShowDynamicMACTable, 	EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,					ReadOnlyUser},
		{0,0,0,0,0,0,0}
};


static const Command I2C_Command_Code[2] = {
		{"<i2c-command [0x00 - 0xFF]>", 		"command to issue to loopback interface", 	TERMINATING_COMMMAND, 	1,	true, 	COM_I2CSend, 			EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,					ModifySystem},
		{0,0,0,0,0,0,0}
};

static const Command I2C_Options[2] = {
		{"send-command", 		"send an I2C over loopback", 								HAS_CHILD, 	NO_PARAMETERS,	false, 	NotImplementedFunction, EMPTY_STATIC_PARAMS,		I2C_Command_Code,					ModifySystem},
		{0,0,0,0,0,0,0}
};

//*******************************************************************************************************************
//
//! Mid level commands for changing system properties
//
//********************************************************************************************************************
static const Command System_Commands[10] = {
		{"eeprom", 				"change settings for the EEPROM", 						HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,		EEPROM_Options,					ModifySystem},
		{"i2c", 				"control other layers with I2C", 						HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,		I2C_Options,					ModifySystem},
		{"status", 				"show global system information", 						TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ShowRunningConfig, 		EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,					ReadOnlyUser},
		{"rapid-link-aging",	"enable/disable fast device aging after link change", 	HAS_CHILD, 				3,				false, 	NotImplementedFunction, 	{GLOBAL_CONTROL_0,"0x00"},	Enable_Disable_Options,			ModifySystem},
		{"large-packets", 		"allow 2KB packets", 									HAS_CHILD, 				3,				false, 	NotImplementedFunction, 	{GLOBAL_CONTROL_1,"0x06"},	Enable_Disable_Options,			ModifySystem},
		{"power-saving", 		"enable/disable power saving on all PHYs", 				HAS_CHILD, 				3,				false, 	NotImplementedFunction, 	{GLOBAL_CONTROL_9,"0x03"},	INV_Enable_Disable_Options,		ModifySystem},
		{"led-mode", 			"set LED mode 0 or mode 1", 							HAS_CHILD, 				3,				false, 	NotImplementedFunction, 	{GLOBAL_CONTROL_9,"0x01"},	LED_Options,					ModifySystem},
		{"show", 				"access VLAN and MAC tables", 							HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,		Table_Options,					ReadOnlyUser},
		{"reset", 				"performs a soft reset of the system", 					TERMINATING_COMMMAND, 	NO_PARAMETERS, 	false, 	COM_ResetTivaC, 			EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,					ModifySystem},
		{0,0,0,0,0,0,0}
};
//*******************************************************************************************************************
//
//! Ethernet Controller Register Manipulation Commands
//
//********************************************************************************************************************
static const Command WriteRegister_Options2[2] = {
		{"<data [0x00 - 0xFF]>", "read a setting from a register on the selected KSZ8895MQX", TERMINATING_COMMMAND, 1,true, COM_WriteToEthernetController, EMPTY_STATIC_PARAMS,		NO_CHILD_MENU,	ModifySystem},
		{0,0,0,0,0,0,0}
};
static const Command WriteRegister_Options[2] = {
		{"<register-addr [0x00 - 0xFF]>", "read a setting from a register on the selected KSZ8895MQX", HAS_CHILD, 1,true, NotImplementedFunction, EMPTY_STATIC_PARAMS,WriteRegister_Options2,	ModifySystem},
		{0,0,0,0,0,0,0}
};
static const Command ReadRegister_Options[2] = {
		{"<register-addr [0x00 - 0xFF]>", "read a setting from a register on the selected KSZ8895MQX", TERMINATING_COMMMAND, 1,true, COM_ReadFromEthernetController, EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ReadOnlyUser},
		{0,0,0,0,0,0,0}
};

static const Command Controller_Options[3] = {
		{"read-reg", 	"read a setting from a register on the selected KSZ8895MQX", 	HAS_CHILD, 	NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	ReadRegister_Options,	ReadOnlyUser},
		{"write-reg", 	"write to a register on the selected KSZ8895MQX", 				HAS_CHILD, 	NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	WriteRegister_Options,	ModifySystem},
		{0,0,0,0,0,0,0}
};
//*******************************************************************************************************************
//
//! Commands to modify the configuration saved on the 25AA1024 EEPROM
//
//********************************************************************************************************************
static const Command Config_Commands[3] = {
		{"save", 	"move the current configuration to the EEPROM", 		TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_SaveSwitchConfiguration, 	EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ModifyPortsOnly},
		{"delete", 	"remove the current configuration from the EEPROM", 	TERMINATING_COMMMAND, 	1,				false, 	COM_DeleteConfig, 				EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ModifySystem},
		{0,0,0,0,0,0,0}
};
//*******************************************************************************************************************
//
//! Commands for modifying the operation of individual ports
//
//********************************************************************************************************************
static const Command Sniffing_Settings[5] = {
		{"disable", 	"return this port to normal operation", 	TERMINATING_COMMMAND, 	3,	false, 	COM_ClearBitEthernetController, 	{PORT_CONTROL1_OFFSET,		"0x07",		"Returning port to normal operational mode..."},	NO_CHILD_MENU,			ModifyPortsOnly},
		{"designate", 	"sets this port as the sniffer port", 		TERMINATING_COMMMAND, 	3,	false, 	COM_SetBitEthernetController, 		{PORT_CONTROL1_OFFSET,		"0x07",		"Setting port as sniffer..."},						NO_CHILD_MENU,			ModifyPortsOnly},
		{"sniff-tx", 	"copy all TX packets to sniffer port", 		HAS_CHILD, 				2,	false, 	NotImplementedFunction, 			{PORT_CONTROL1_OFFSET, 		"0x05"},														Enable_Disable_Options, ModifyPortsOnly},
		{"sniff-rx", 	"copy all RX packets to sniffer port", 		HAS_CHILD, 				2,	false, 	NotImplementedFunction, 			{PORT_CONTROL1_OFFSET, 		"0x06"},														Enable_Disable_Options, ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};

static const Command VLAN__TableSettings[2] = {
		{"<vlan-id [1-4096]>", "set the VLAN for this port", TERMINATING_COMMMAND, 1,true, COM_SetVLANEntry, EMPTY_STATIC_PARAMS, 	NO_CHILD_MENU,	ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};

static const Command VLAN_Settings[5] = {
		{"enable", 				"globally enables the use of VLAN filtering", 	TERMINATING_COMMMAND, 	0,	false, 	COM_EnableVLANS, 		EMPTY_STATIC_PARAMS, 	NO_CHILD_MENU,			ModifyPortsOnly},
		{"disable", 			"globally disables the use of VLAN filtering", 	TERMINATING_COMMMAND, 	0,	false, 	COM_DisableVLANS, 		EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,			ModifyPortsOnly},
		{"add", 				"add an entry to the VLAN table", 				HAS_CHILD, 				0,	false, 	NotImplementedFunction, EMPTY_STATIC_PARAMS,	VLAN__TableSettings,	ModifyPortsOnly},
		{"<vlan-id [1-4096]>", 	"set the VLAN for this port", 					TERMINATING_COMMMAND, 	1,	true, 	COM_SetPortVLAN, 		EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,			ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};
static const Command Duplex_Settings[5] = {
		{"half-duplex", 	"set this port to use flow control when managing incoming and outgoing packets", 	TERMINATING_COMMMAND, 	3,	false, 	COM_ClearBitEthernetController, 	{PORT_CONTROL5_OFFSET,"0x05","Setting port to HALF-DUPLEX..."}, 	NO_CHILD_MENU,	ModifyPortsOnly},
		{"full-duplex", 	"set this port to operate bi-directionally", 										TERMINATING_COMMMAND, 	3,	false, 	COM_SetBitEthernetController, 		{PORT_CONTROL5_OFFSET,"0x05","Setting port to FULL-DUPLEX..."}, 	NO_CHILD_MENU,	ModifyPortsOnly},
		{"100BT", 			"set this port to operate at 100BaseT", 											TERMINATING_COMMMAND, 	3,	false, 	COM_SetBitEthernetController, 		{PORT_CONTROL5_OFFSET,"0x06","Setting port to 100 Mbps..."}, 		NO_CHILD_MENU,	ModifyPortsOnly},
		{"10BT", 			"set this port to operate at 10BaseT", 												TERMINATING_COMMMAND, 	3,	false, 	COM_ClearBitEthernetController, 	{PORT_CONTROL5_OFFSET,"0x06","Setting port to 10 Mbps..."}, 		NO_CHILD_MENU,	ModifyPortsOnly},
		{0,0,0,0,0,0}
};
static const Command Port_Options[15] = {
		{"enable", 				"turn this port on", 										TERMINATING_COMMMAND, 	3,				false, 	COM_ClearBitEthernetController, 	{PORT_CONTROL6_OFFSET,"0x03","Enabling Selected Port..."},				 	NO_CHILD_MENU,				ModifyPortsOnly},
		{"disable", 			"turn this port off", 										TERMINATING_COMMMAND, 	3,				false, 	COM_SetBitEthernetController, 		{PORT_CONTROL6_OFFSET,"0x03","Disabling Selected Port..."},				 	NO_CHILD_MENU,				ModifyPortsOnly},
		{"vlan", 				"assign a vlan to this port", 								HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 			EMPTY_STATIC_PARAMS, 														VLAN_Settings,				ModifyPortsOnly},
		{"speed",				"modify the rate at which this port operates", 				HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 			EMPTY_STATIC_PARAMS,														Duplex_Settings,			ModifyPortsOnly},
		{"status",				"information regarding the current state of this port", 	TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ShowPortStatus, 				EMPTY_STATIC_PARAMS,														NO_CHILD_MENU,				ReadOnlyUser},
		{"broadcast-storm", 	"enable/disable broadcast storm protection", 				HAS_CHILD, 				2,				false, 	NotImplementedFunction, 			{PORT_CONTROL0_OFFSET, "0x07"},												Enable_Disable_Options,		ModifyPortsOnly},
		{"sniff-state", 		"sniffing settings for this port", 							HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 			EMPTY_STATIC_PARAMS, 														Sniffing_Settings,			ModifyPortsOnly},
		{"toggle-tx", 			"enable/disable packet transmission", 						HAS_CHILD, 				2,				false, 	NotImplementedFunction, 			{PORT_CONTROL2_OFFSET, "0x02"},												Enable_Disable_Options,		ModifyPortsOnly},
		{"toggle-rx", 			"enable/disable packet reception", 							HAS_CHILD, 				2,				false, 	NotImplementedFunction, 			{PORT_CONTROL2_OFFSET, "0x01"},												Enable_Disable_Options,		ModifyPortsOnly},
		{"run-diag", 			"run cable diagnostics", 									TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_RunCableDiagnostics, 			EMPTY_STATIC_PARAMS,														NO_CHILD_MENU,				ReadOnlyUser},
		{"auto-neg", 			"enable/disable auto-negoatiation", 						HAS_CHILD, 				2,				false, 	NotImplementedFunction, 			{PORT_CONTROL5_OFFSET, "0x07"},												INV_Enable_Disable_Options,	ModifyPortsOnly},
		{"restart-auto-neg", 	"restart auto-negotiation", 								TERMINATING_COMMMAND, 	3,				false, 	COM_SetSCBitEthernetController, 	{PORT_CONTROL6_OFFSET, "0x05", "Restarting Auto-MDI/MDIX"},					NO_CHILD_MENU,				ModifyPortsOnly},
		{"auto-mdix", 			"enable/disable auto MDI/MDI-X", 							HAS_CHILD, 				2,				false, 	NotImplementedFunction, 			{PORT_CONTROL6_OFFSET, "0x02"},												INV_Enable_Disable_Options,	ModifyPortsOnly},
		{"force-mdi", 			"manually enable/disable MDI mode", 						HAS_CHILD, 				2,				false, 	NotImplementedFunction, 			{PORT_CONTROL6_OFFSET, "0x01"},												Enable_Disable_Options,		ModifyPortsOnly},
		{0,0,0,0,0,0,0}
};
static const Command Port_Commands[5] = {
		{"f0", 	"settings for fast-ethernet0", 	HAS_CHILD, 	1,	false, 	NotImplementedFunction, 	{PORT1_OFFSET},	Port_Options, 	ReadOnlyUser},
		{"f1", 	"settings for fast-ethernet1", 	HAS_CHILD, 	1,	false, 	NotImplementedFunction, 	{PORT2_OFFSET},	Port_Options,	ReadOnlyUser},
		{"f2", 	"settings for fast-ethernet2", 	HAS_CHILD, 	1,	false, 	NotImplementedFunction, 	{PORT3_OFFSET},	Port_Options,	ReadOnlyUser},
		{"f3", 	"settings for fast-ethernet3", 	HAS_CHILD, 	1,	false, 	NotImplementedFunction, 	{PORT4_OFFSET},	Port_Options,	ReadOnlyUser},
		{0,0,0,0,0,0,0}
};


//*******************************************************************************************************************
//
// Event Management
//
//********************************************************************************************************************
static const Command Event_Options[5] = {
		{"status", 			"list currently enabled/disabled events", 	TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_EventStatus, 		EMPTY_STATIC_PARAMS, NO_CHILD_MENU, 		ReadOnlyUser},
		{"manage", 			"add an event to log", 						TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ManageEvents, 		EMPTY_STATIC_PARAMS, NO_CHILD_MENU, Administrator},
		{"list", 			"show all logged events", 					TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ListEvents, 		EMPTY_STATIC_PARAMS, NO_CHILD_MENU, Administrator},
		{"clear", 			"clear all logged events", 					TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_DeleteEvents, 		EMPTY_STATIC_PARAMS, NO_CHILD_MENU, Administrator},
		{0,0,0,0,0,0,0}
};

//*******************************************************************************************************************
//
// User Management
//
//********************************************************************************************************************
static const Command User_Options[4] = {
		{"list", 		"list all users allowed to access this switch", 	TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_ListUsers, 				EMPTY_STATIC_PARAMS,	NO_CHILD_MENU, ReadOnlyUser},
		{"add", 		"add a user", 										TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_AddUser, 				EMPTY_STATIC_PARAMS,	NO_CHILD_MENU, Administrator},
		{"delete", 		"delete a user", 									TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_DeleteUsersMenu, 		EMPTY_STATIC_PARAMS,	NO_CHILD_MENU, Administrator},
		{0,0,0,0,0,0,0}
};

//*******************************************************************************************************************
//
// Administrative Commands
//
//********************************************************************************************************************
static const Command Admin_Commands[3] = {
		{"users", 	"manage the users allowed to administrate this switch", 	HAS_CHILD, 	NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	User_Options, 	ReadOnlyUser},
		{"events", 	"manage the events logged to EEPROM", 						HAS_CHILD, 	NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	Event_Options, 	ReadOnlyUser},
		{0,0,0,0,0,0,0}
};

//*******************************************************************************************************************
//
// Main Menu Commands
//
//********************************************************************************************************************
static const Command Command_Categories[7] = {
		{"admin", 		"commands for changing the settings of the switch layer", 			HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	&Admin_Commands[0],			ReadOnlyUser},
		{"port", 		"modify a port on the switch board", 								HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS, 	&Port_Commands[0],			ReadOnlyUser},
		{"controller", 	"modify a setting on the ethernet controller", 						HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	&Controller_Options[0],		ReadOnlyUser},
		{"system", 		"advanced settings for changing the operation of this device", 		HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	&System_Commands[0],		ReadOnlyUser},
		{"config", 		"save or delete this switch's running configuration", 				HAS_CHILD, 				NO_PARAMETERS,	false, 	NotImplementedFunction, 	EMPTY_STATIC_PARAMS,	&Config_Commands[0],		ModifyPortsOnly},
		{"logout", 		"exit this session. Does not automatically save configuration.",	TERMINATING_COMMMAND, 	NO_PARAMETERS,	false, 	COM_Logout, 	EMPTY_STATIC_PARAMS, 				NO_CHILD_MENU,				ReadOnlyUser},
		{0,0,0,0,0,0,0}
};

//*******************************************************************************************************************
//
//! Mappings for individual possible register values. Used a sub-menu of ConfigOption
//
//*******************************************************************************************************************
typedef struct ConfigValues {
	//! A possible value for the parent setting when masked
	const uint32_t value;
	//! A helpful description for this value when masked
	const char * const value_description;
} ConfigValues;
//*******************************************************************************************************************
//
//! Mappings for individual register settings. Used a sub-menu of ConfigBase
//
//*******************************************************************************************************************
typedef struct ConfigOption {
	//! An hexadecimal number to mask the current register value with. Allows the system to check each ConfigValue against
	//! this computed value
	const uint32_t mask;
	//! A helpful descrition of this setting
	const char * const description;
	//! Possible values for this setting when the parent register is masked
	const ConfigValues values[MAX_VALUES];
} ConfigOption;
//*******************************************************************************************************************
//
//! Mappings of each register in the KSZ8895MLUB. Used in conjunction with ConfigOption and (eventually) ConfigValues
//
//*******************************************************************************************************************
typedef struct ConfigBase {
	//! The base address of the register.
	const uint32_t base_addr;
	//! A helpful title for the register.
	const char * const title;
	//! Possible settings that the register holds.
	const ConfigOption options[MAX_OPTIONS];
} ConfigBase;


//*******************************************************************************************************************
//
//! MAPPINGS FOR REGISTER VALUES
//! These linked structures convert the hexadecimal value stored in each ethernet controller register defined below
//! to a user readable string. <br>
//!
//! The structure of each of these entries is as follows: <br>
//! /code
//! {REGISTER BASE (in hex), A FRIENDLY TITLE, {
//!     <Array of register values defined below>
//! 	}
//!
//! {REGISTER BIT MASK, A FRIENDLY NAME FOR THIS SETTING, {
//! 	<Possible values for this setting, again defined below>
//! }
//!
//! {REGISTER VALUE WHEN MASKED, NAME FOR THIS VALUE}
//! /endcode
//
//********************************************************************************************************************
static const ConfigBase GlobalConfigMappings[GLOBAL_CONFIG_MAX_MAPPINGS] = {
	//Global Switch Information Register
	{0x01, "Global Switch Information",{
			// 0x01 & 0xF0 masks off the setting for Chip ID
			{0xF0, 		"Chip ID", 					{
					//Value will be 0x40 (when masked by 0xF0) when the setting is for KSZ8895MQX/FQX/ML
					{0x40, "KSZ8895MQX/FQX/ML"},
					//Value will be 0x60 (when masked by 0xF0) when the setting is for KSZ8895RQX
					{0x60, "KSZ8895RQX"},
					//THIS NULL ENTRY IS REQUIRED TO INDICATED THE END OF THE ARRAY
					{0,0}}
			},
			// 0x01 & 0x01 masks off the setting for Switch State
			{0x01, 		"Switch State", 			{
					//Value will be 0x01 (when masked by 0x01) when the switch is started
					{0x01, "Started"},
					//Value will be 0x00 (when masked by 0x00) when the switch is stopped
					{0x00, "Stopped"},
					//THIS NULL ENTRY IS REQUIRED TO INDICATED THE END OF THE ARRAY
					{0,0}}
			},
			{0,0,0,0,0,0}
		}
	},
	{0x03, "Global Control 1",{
			{0x80, 		"Pass All Frames", 			{{0x80, "True"}, 				{0x00, "False"},				{0,0}}},
			{0x40, 		"2K Byte Support", 			{{0x40, "True"}, 				{0x00, "False"},				{0,0}}},
			{0x20, 		"TX Flow Control Disable", 	{{0x20, "True"}, 				{0x00, "False"},				{0,0}}},
			{0x10, 		"RX Flow Control Disable", 	{{0x10, "True"}, 				{0x00, "False"},				{0,0}}},
			{0x8, 		"Frame Length Field Check", {{0x08, "True"}, 				{0x00, "False"},				{0,0}}},
			{0x2, 		"Fast Aging", 				{{0x01, "True"}, 				{0x02, "False"},				{0,0}}},
			{0x1, 		"Agressive Back-Off", 		{{0x01, "True"}, 				{0x00, "False"},				{0,0}}},
			{0,0,0,0,0,0}
		}
	},
	{0x05, "Global Control 3",{
			{0x80, 		"802.1Q VLANs Enabled", 	{{0x80, "True"}, 				{0x00, "False"},				{0,0}}},
			{0x1, 		"Sniff Mode Select", 		{{0x01, "True"}, 				{0x00, "False"},				{0,0}}},
			{0,0,0,0,0,0}
		}
	},
	{0x0B, "Global Control 9",{
			{0x2, 		"LED Mode", 				{{0x02, "Mode 1"}, 				{0x00, "Mode 0"},				{0,0}}},
			{0x1, 		"SPI Read Trigger", 		{{0x01, "Rising Edge"}, 		{0x00, "Falling Edge"},			{0,0}}},
			{0,0,0,0,0,0}
		}
	},
	{0x0C, "Global Control 10",{
			{0x30, 		"CPU Interface Speed", 		{{0x00, "41.67 MHz"}, {0x10, "83.33 MHz"},{0x20, "125 MHz"},	{0,0}}},
			{0,0,0,0,0,0}
		}
	},
	{0x0E, "Power Management",{
			{0x18, 		"Power Management Mode", 	{{0x00, "Normal Mode"}, {0x08, "Energy Detection Mode"},{0x10, "Soft Power Down Mode"},{0x18, "Power Saving Mode"},	{0,0}}},
			{0,0,0,0,0,0}
		}
	},
	{0,0,0}
};

static const ConfigBase PortConfigMappings [PORT_MAX_MAPPINGS] = {
	{0x00, "Port Control 0",{
			{0x80, 		"Broadcast Storm Protection", 				{{0x80, "True"}, {0x00, "False"},								{0,0}}},
			{0,0,0}
		}
	},
	{0x01, "Port Control 1",{
			{0x80, 		"Sniffer Port", 							{{0x80, "True"}, {0x00, "False"},								{0,0}}},
			{0x40, 		"Sniffing RX", 								{{0x40, "True"}, {0x00, "False"},								{0,0}}},
			{0x20, 		"Sniffing TX", 								{{0x20, "True"}, {0x00, "False"},								{0,0}}},
			{0,0,0}
		}
	},
	{0x02, "Port Control 2",{
			{0x2, 		"Transmit Enabled", 						{{0x02, "True"}, {0x00, "False"},								{0,0}}},
			{0x1, 		"Receive Enabled", 							{{0x01, "True"}, {0x00, "False"},								{0,0}}},
			{0,0,0}
		}
	},
	{0x09, "Port Status 0",{
			{0x80, 		"MDI/MDI-X Mode", 							{{0x80, "HP Auto MDI/MDI-X"}, {0x00, "Micrel Auto MDI/MDI-X"},	{0,0}}},
			{0x20, 		"Polarity", 								{{0x20, "Reversed"}, {0x00, "Not Reversed"},					{0,0}}},
			{0x10, 		"TX Flow Control", 							{{0x10, "Active"}, {0x00, "Disabled"},							{0,0}}},
			{0x8, 		"RX Flow Control", 							{{0x08, "Active"}, {0x00, "Disabled"},							{0,0}}},
			{0x4, 		"Port Speed", 								{{0x04, "100 Mbps"}, {0x00, "10 Mbps"},							{0,0}}},
			{0x2, 		"Port Duplex Mode", 						{{0x02, "Full"}, {0x00, "Half"},								{0,0}}},
			{0,0,0}
		}
	},
	{0x0C, "Port Control 5",{
			{0x80, 		"Auto-Negotiation", 						{{0x80, "Disabled"}, {0x00, "Enabled"},							{0,0}}},
			{0x40, 		"Forced Speed (AN must be Disabled)", 		{{0x40, "100 Mbps"}, {0x00, "10 Mbps"},							{0,0}}},
			{0x20, 		"Forced Duplex (AN must be Disabled)", 		{{0x20, "Full"}, {0x00, "Half"},								{0,0}}},
			{0,0,0}
		}
	},
	{0x0D, "Port Control 6",{
			{0x80, 		"LEDs Disabled", 							{{0x80, "True"}, {0x00, "False"},								{0,0}}},
			{0x8, 		"Port State", 								{{0x08, "Administratively Disabled"}, {0x00, "ON"},				{0,0}}},
			{0x4, 		"Auto MDI/MDI-X", 							{{0x04, "Disabled"}, {0x00, "Enabled"},							{0,0}}},
			{0,0,0}
		}
	},
	{0x0E, "Port Status 1",{
			{0x80, 		"MDIX Status", 								{{0x80, "Port using MDI"}, {0x00, "Port using MDI-X"},			{0,0}}},
			{0x40, 		"Auto-Negotiation State", 					{{0x40, "Done"}, {0x00, "In-Progress"},							{0,0}}},
			{0x20, 		"Link Status", 								{{0x20, "Connected"}, {0x00, "Disconnected"},					{0,0}}},
			{0,0,0}
		}
	},
	{0,0,0}
};




#endif /* INTERPRETER_TASK_H_ */
