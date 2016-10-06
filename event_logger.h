/**\file event_logger.h
 * \brief <b>Settings and Function Prototypes for System Logging Task</b>
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

#ifndef EVENT_LOGGER_H_
#define EVENT_LOGGER_H_

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

//*****************************************************************************
//
//! Maximum number of log types that can be configured. This number reflects
//! the number of bits in a uint32_t. Each of the bits, when toggled high indicate
//! to the logger that a LogType at that position is enabled.
//! (i.e. 0000 0000 0000 0000 0000 0000 0000 1000 indicates that EEPROMWriteOP is
//! the only event type enabled for logging)
//
//*****************************************************************************
#define MAX_LOG_TYPES 			32
//*****************************************************************************
//
//! Maximum number of log entries that can be placed in EEPROM. Since the EEPROM
//! log is placed at after all other values placed in EEPROM, this value can be
//! increased if necessary. However, it is HIGHLY RECOMMENDED that this value
//! is not increased past 2000, as the system would need to read 10,000 sectors
//! (2000 entries * 5 sectors/entry) to print the entire contents of the log.
//
//*****************************************************************************
#define MAX_LOG_ENTRIES			400

//*****************************************************************************
//
//! Types of entries that are currently configured for logging.
//
//*****************************************************************************
typedef enum {
	//! Logs system restarts (either on start or restart)
	SystemRestarted,
	//! Logs FreeRTOS and system stack overflow errors
	StackOverflow,
	//! Logs any EEPROM write operations
	EEPROMWriteOP,
	//! Logs any EEPROM read operations
	EEPROMReadOP,
	//! Logs any errors in performing a write/read of the EEPROM
	EEPROMIOException,
	//! Logs any Ethernet Controller read operations
	EthoControllerReadOP,
	//! Logs any Ethernet Controller write operations
	EthoControllerWriteOP,
	//! Logs any errors in performing a write/read of the EEPROM
	EthoControlIOException,
	//! Logs when a user signs into the system
	UserLoggedIn,
	//! Logs when a user signs out of the system
	UserLoggedOut
} LoggerCodes;

//*****************************************************************************
//
// Prototypes for the LOGGER task.
//
//*****************************************************************************

//*****************************************************************************
//
//! System logger for any enabled events. Uses a FreeRTOS queue to buffer events
//! that needed to be placed as entries in the EEPROM. All logs start at
//! EEPROM_LOG_BASE specified by developer in freertos_init.h <br>
//!
//! Log entries consist of the following: <br>
//! 32-bits = FreeRTOS Task Time <br>
//! 8-bits = Log Entry Type <br>
//!
//! (Max number of log types limited by LogStatusFlags [32 types])
//
//*****************************************************************************
extern uint32_t LoggerTaskInit(void);

//*****************************************************************************
//
//! Log Item EEPROM
//! \brief Takes the LoggerCode passed as "code" and places it in the LOGGER
//! task queue if the task is running. Otherwise, this entry is discarded.
//!
//! This function should be used anywhere in the firmware where an event should
//! be logged to the EEPROM
//!
//! \param code The LoggerCodes value to place in the next logical memory slot
//! for a log.
//
//*****************************************************************************
extern void LogItemEEPROM(LoggerCodes code);


//*****************************************************************************
//
//! Log Types Mapped to user-readable strings. This array should be in the same
//! order as the LoggerCodes enum. If any changes are made to LoggerCodes, those
//! changes need to be reflected in this array.
//
//*****************************************************************************
static char *LogTypes[MAX_LOG_TYPES] = {"System Restarted", "Stack Overflow", "EEPROM Write Operations", "EEPROM Read Operations", "EEPROM Read/Write Errors",
		"Ethernet Controller Read Operations", "Ethernet Controller Write Operations", "Ethernet Controller Read/Write Errors", "User Logged In", "User Logged Out", 0x00};

//*****************************************************************************
//
//! Enable/Disable flags for Log Types. Modified by CLI.
//
//*****************************************************************************
extern uint32_t LogStatusFlags;
//*****************************************************************************
//
//! Next logical slot for EEPROM log entries
//
//*****************************************************************************
extern uint32_t NextLogSlot;

//*****************************************************************************
//
//! A handle to the EEPROM logger task so that its current status (running/not running)
//! can be checked.
//
//*****************************************************************************
extern xTaskHandle LoggerTaskHandle;

#endif /* EVENT_LOGGER_H_ */
