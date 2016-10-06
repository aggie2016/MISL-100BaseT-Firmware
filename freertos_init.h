/**\file freertos_init.h
 * \brief <b>System Settings and minor function prototypes. Use this file to reconfigure for other system setups.</b>
 *
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

#ifndef FREERTOS_INIT_H_
#define FREERTOS_INIT_H_

//Single digit major version number
#define MAJOR_VERSION 1
//Single digit minor version number
#define MINOR_VERSION 1
//Two digit revision number
#define REVISION 12

//*****************************************************************************
//
//! \var TASK_STATES Define the different possible states of a task.
//
//*****************************************************************************
static const char *TASK_STATES[] = {
    "Running","Ready","Blocked","Suspended","Deleted"
};

//*****************************************************************************
//
//! \var ConsoleMode Defines whether or not the current connected console
//! session is from a PuTTY console window or an external application.
//! TRUE = Console, FALSE = External Application
//
//*****************************************************************************
static bool ConsoleMode = true;
//*****************************************************************************
//
//! \var I2C_DEVICE_ADDR The current I2C address of this slave/master device.
//! Reconfigurable through the console at runtime.
//
//*****************************************************************************
static uint8_t I2C_DEVICE_ADDR = 0x1A;
//*****************************************************************************
//
//! \var console_hostname The current identifier that the user sees when connecting
//! to this device over a console window. Reconfigurable through the console window
//! at runtime.
//
//*****************************************************************************
static char console_hostname[16] = "EEE-Switch";

void DTSTriggered(void);
void DTSOFF(void);
void I2CSendTestValue();

//*****************************************************************************
//
//! The maximum number of users this system is configured to allow. The "users"
//! array actually contains 16 values so that default credentials can be stored.
//! DEFAULT USER: root
//! DEFAULT PASSWORD: root
//
//*****************************************************************************
#define MAX_USERS 15


//*****************************************************************************
//
//! \brief Permission Levels that restricts which users can access the commands that
//! are available for use.
//
//*****************************************************************************
typedef enum {
	//! User will only be able to view the system's settings. No modification of any setting should be allowed
	ReadOnlyUser,
	//! User will additionally be able to modify the settings of each port.
	ModifyPortsOnly,
	//! User will additionally be able to modify settings of the system and read/write to system peripherals
	ModifySystem,
	//! User has full admin privilidges and can add/delete users.
	Administrator
} PermLevel;

typedef enum {
	Update,
	Delete,
	Add,
	None
} Action;

//*****************************************************************************
//
//! \brief An entry that defines each user allowed to access this system.
//
//*****************************************************************************
typedef struct Users {
	//! A 16-character username
	char username[16];
	//! A 16-character First Name
	char first_name[16];
	//! A 16-character Last Name
	char last_name[16];
	//! A 16-Character password
	char password[16];
	//! A permission level for this user
	PermLevel permissions;
	//! A flag used when adding/deleting users
	bool isMarked;
	//! A flag used when determining whether this user should be saved to the EEPROM.
	Action nextAction;
} User_Data;


static char * RoleDefs[4] = {"Read Only", "Modify Ports", "Modify System", "Administrator"};

extern bool Authenticated;
extern bool UsePasswordMask;
extern bool UARTInterpreterEnabled;

extern User_Data users[16];

void ShowDebugInformation();
//*****************************************************************************
//
// EEPROM port and pin settings.
// Change as appropriate to match your hardware configuration.
//
//*****************************************************************************
#define EEPROM_BASE_ADDR		SSI0_BASE
#define EEPROM_SSI_CLK			GPIO_PA2_SSI0CLK
#define EEPROM_SSI_CLK_PIN		GPIO_PIN_2
#define EEPROM_SSI_TX			GPIO_PA5_SSI0TX
#define EEPROM_SSI_TX_PIN		GPIO_PIN_5
#define EEPROM_SSI_RX			GPIO_PA4_SSI0RX
#define EEPROM_SSI_RX_PIN		GPIO_PIN_4
#define EEPROM_SSI_CS_BASE		GPIO_PORTA_BASE
#define EEPROM_SSI_CS_PIN		GPIO_PIN_3

#define EEPROM_SYS_BASE			SYSCTL_PERIPH_SSI0
#define EEPROM_SYS_PORT_BASE	SYSCTL_PERIPH_GPIOA
//*****************************************************************************
//
// Ethernet Controller port and pin settings.
// Change as appropriate to match your hardware configuration.
//
//*****************************************************************************
#define ETHO_1_BASE_ADDR		SSI1_BASE
#define ETHO_1_SSI_CLK 			GPIO_PD0_SSI1CLK
#define ETHO_1_SSI_CLK_PIN 		GPIO_PIN_0
#define ETHO_1_SSI_TX 			GPIO_PD3_SSI1TX
#define ETHO_1_SSI_TX_PIN		GPIO_PIN_3
#define ETHO_1_SSI_RX 			GPIO_PD2_SSI1RX
#define ETHO_1_SSI_RX_PIN		GPIO_PIN_2
#define ETHO_1_SSI_CS_BASE 		GPIO_PORTD_BASE
#define ETHO_1_SSI_CS_PIN 		GPIO_PIN_1

#define ETHO_1_SYS_BASE			SYSCTL_PERIPH_SSI1
#define ETHO_1_SYS_PORT_BASE	SYSCTL_PERIPH_GPIOD
//*****************************************************************************
//
// I2C port and pin settings.
// Change as appropriate to match your hardware configuration.
//
//*****************************************************************************
#define I2C_INTERRUPT_BASE		INT_I2C0
#define I2C_BASE_ADDR 			I2C0_BASE
#define I2C_PORT_BASE			GPIO_PORTB_BASE
#define I2C_SCL 				GPIO_PB2_I2C0SCL
#define I2C_SCL_PIN 			GPIO_PIN_2
#define I2C_SDA 				GPIO_PB3_I2C0SDA
#define I2C_SDA_PIN 			GPIO_PIN_3

#define I2C_SYS_PORT_BASE 		SYSCTL_PERIPH_GPIOB
#define I2C_SYS_BASE 			SYSCTL_PERIPH_I2C0
//*****************************************************************************
//
// LED selection settings. Change second parameter to redefine LED's purpose.
//
//*****************************************************************************
#define CONSOLE_OPEN_LED 					0
#define POWER_LED 							1
#define CONFIG_LED							2
#define POWER_LED_BLINKRATE					0
#define CONSOLE_OPEN_LED_BLINKRATE 			800
#define CONFIG_LED_BLINKRATE 				400
#define ERROR_LED_BLINKRATE					0


//*****************************************************************************
//
// RTOS task enable/disable settings. Most commonly used for debugging.
//
//*****************************************************************************
#define ENABLE_LED_MANAGER 	true
#define ENABLE_INTERPRETER 	true
#define ENABLE_I2C_MANAGER 	true
#define ENABLE_LOGGER 		true
#define ENABLE_PORT_MONITOR true

//*****************************************************************************
//
// Base addresses for settings and values stored in on-board EEPROM
//
//*****************************************************************************
#define EEPROM_FIRMWARE_SETTINGS 	0x1E
#define EEPROM_FIRMWARE_LOGFLAGS_1	0x1F
#define EEPROM_FIRMWARE_LOGFLAGS_2	0x20
#define EEPROM_FIRMWARE_LOGFLAGS_3	0x21
#define EEPROM_FIRMWARE_LOGFLAGS_4	0x22
#define EEPROM_FIRMWARE_NEXTLOG_1	0x23
#define EEPROM_FIRMWARE_NEXTLOG_2	0x24
#define EEPROM_FIRMWARE_NEXTLOG_3	0x25
#define EEPROM_FIRMWARE_NEXTLOG_4	0x26
#define EEPROM_SWITCH_CONFIG_BASE 	0x100
#define EEPROM_VLAN_TABLE_BASE 		0x200
#define EEPROM_USERS_BASE			0x1200
#define EEPROM_LOG_BASE				0x1600


#endif /* FREERTOS_INIT_H_ */
