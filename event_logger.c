/**\file event_logger.c
 * \brief <b>System Logging FreeRTOS Task</b>
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

#include <event_logger.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "sysctl.h"
#include "gpio.h"
#include "rom.h"
#include "freertos_init.h"
#include "eee_hal.h"
#include "event_logger.h"
#include "uartstdio.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"




//*****************************************************************************
//
//! The stack size for the logger task.
//
//*****************************************************************************
#define LOGGERTASKSTACKSIZE        128         // Stack size in words

//*****************************************************************************
//
//! The size of items placed in the LOGGER message queue.
//! Based on size of LOGGER_CODES.
//
//*****************************************************************************
#define LOGGER_ITEM_SIZE           sizeof(LoggerCodes)
//*****************************************************************************
//
//! The number of items that can be placed in the LOGGER message queue.
//
//*****************************************************************************
#define LOGGER_QUEUE_SIZE          10


//*****************************************************************************
//
//! Interval to pause the LOGGER task between loops to allow other, higher
//! priority RTOS tasks to run for a while.
//
//*****************************************************************************
#define LOGGER_TASK_DELAY        200

xQueueHandle g_pLoggerQueue;
extern xSemaphoreHandle g_pUARTSemaphore;

//*****************************************************************************
//
//! Log entry status flags. Flags can be modified at run-time to prevent certain
//! types of entries from being logged. MAXIMUM NUMBER OF FLAGS IS 32.
//
//*****************************************************************************
uint32_t LogStatusFlags = 0x00;

//*****************************************************************************
//
//! Next logical memory slot for a log entry. Initialized by default to the
//! starting address allocated for log entries.
//
//*****************************************************************************
uint32_t NextLogSlot = EEPROM_LOG_BASE;

//*****************************************************************************
//
//! A task pointer (handle) to the LOGGER TASK that can be used in later iterations
//! to control when this task is created/disposed.
//
//*****************************************************************************
xTaskHandle LoggerTaskHandle;


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
static void LoggerTask(void *pvParameters)
{
    portTickType ui32WakeTime;
    uint32_t ui32LOGGERToggleDelay;
    LoggerCodes code_issued;
    LoggerCodes last_code_issued;

    ui32LOGGERToggleDelay = LOGGER_TASK_DELAY;

    //
    // Get the current tick count.
    //
    ui32WakeTime = xTaskGetTickCount();
    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Read the next message, if available on queue.
        //
        if(xQueueReceive(g_pLoggerQueue, &code_issued, 0) == pdPASS)
        {
        	//Is this log entry type enabled?
        	if ((LogStatusFlags >> code_issued) & 1) {

        		//Only log unique events
        		if (code_issued != last_code_issued) {
					int eeprom_sector = 0;
					//Get RTOS system time since vTaskStartScheduler()
					TickType_t RTOSTime = xTaskGetTickCount();
					uint8_t LogTime[4] = {((RTOSTime >> 24) & 0xFF), ((RTOSTime >> 16) & 0xFF), ((RTOSTime >> 8) & 0xFF), ((RTOSTime) & 0xFF)};

					//Write FreeRTOS system time to EEPROM
					for (eeprom_sector = 0; eeprom_sector < 4; eeprom_sector++) {
						EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(NextLogSlot + eeprom_sector), LogTime[eeprom_sector]);
					}
					//Write Log Message to EEPROM
					EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, NextLogSlot + eeprom_sector, code_issued);
					if (((NextLogSlot + 5) > EEPROM_LOG_BASE + MAX_LOG_ENTRIES)) {
						NextLogSlot = EEPROM_LOG_BASE;
					}
					else {
						//Each log takes five sectors.
						NextLogSlot += 5;
					}
					//Hold the last code logged so that we are wasting space logging concurrent writes.
					last_code_issued = code_issued;
        		}
        	}
        }
        vTaskDelayUntil(&ui32WakeTime, ui32LOGGERToggleDelay / portTICK_RATE_MS);
    }
}

//*****************************************************************************
//
//! Initializes the EEPROM LOGGER task.
//
//*****************************************************************************
uint32_t LoggerTaskInit(void)
{

    g_pLoggerQueue = xQueueCreate(LOGGER_QUEUE_SIZE, LOGGER_ITEM_SIZE);

    //
    // Create the LED task.
    //
    if(xTaskCreate(LoggerTask, (const portCHAR *)"LOGGER", LOGGERTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_LOGGER_TASK, &LoggerTaskHandle) != pdTRUE)
    {
        return(1);
    }
    //
    // Success.
    //
    return(0);
}

//*****************************************************************************
//
//! Log Item EEPROM
//! \brief Takes the LoggerCode passed as "code" and places it in the LOGGER
//! task queue if the task is running. Otherwise, this entry is discarded.
//!
//! \param code The LoggerCodes value to place in the next logical memory slot
//! for a log.
//
//*****************************************************************************
void LogItemEEPROM(LoggerCodes code) {
    if ((eTaskGetState(LoggerTaskHandle) == eReady || eTaskGetState(LoggerTaskHandle) == eRunning || eTaskGetState(LoggerTaskHandle) == eBlocked) && (LoggerTaskHandle != 0)) {
    	xQueueSend(g_pLoggerQueue,&code,0);
    }
}

