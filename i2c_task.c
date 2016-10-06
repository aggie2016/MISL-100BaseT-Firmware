/**\file i2c_task.c
 * \brief <b> I2C Interpreter Task (slave-only) </b>
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

#include <eee_hal.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "drivers/rgb.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "i2c_task.h"
#include "i2c.h"
#include "freertos_init.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
//! An external handle to the I2C Interface Mutex that blocks other tasks
//! from using the resource until it becomes available.
//
//*****************************************************************************
extern xSemaphoreHandle g_pI2CSemaphore;

//*****************************************************************************
//
//! A handle to the I2C queue that gets filled by the I2C ISR.
//
//*****************************************************************************
xQueueHandle g_pI2CQueue;


//*****************************************************************************
//
//! A task for interpreting, much like the UART interpreter, the commands
//! in (8-bits [0x00 - 0xFF]) that are received over the I2C interface. Each
//! command just checked against those found in the i2c_task.h file. Most functions
//! do not require anything beyond the command code to be called over I2C.
//! However, there are a few functions that require additional information after
//! the command code to be recognized as valid commands. For more information
//! regarding this refer to i2c_task.h
//
//*****************************************************************************
static void I2CManagerTask(void *pvParameters)
{
	I2C_Packet data;

    while(1)
    {
        //
        // Read the next message, if available on queue.
        //
        if(xQueueReceive(g_pI2CQueue, &data, (portTICK_PERIOD_MS*100)) == pdPASS)
        {
        	int i = 0;
        	uint8_t  functionParameters[MAX_PARAMS] = {0x00};
        	//COMMAND CODE
        	if (data.I2CRXBuffer[0] < MAX_I2C_COMMAND) {

        		//Gather all static parameters for function call, skip first item since it's our command code.
        		if (I2C_Mappings[data.I2CRXBuffer[0]].static_pcount != NO_PARAMETERS)
        		{
        			for (i = 0; i < I2C_Mappings[data.I2CRXBuffer[0]].static_pcount; i++) {
        				functionParameters[i] = I2C_Mappings[data.I2CRXBuffer[0]].static_parameters[i];
        			}
        		}

        		//Gather all custom parameters sent over i2c and append them after the static parameters found (if any).
        		if (I2C_Mappings[data.I2CRXBuffer[0]].custom_pcount != NO_PARAMETERS)
        		{
            		for (i = 0; i < I2C_Mappings[data.I2CRXBuffer[0]].custom_pcount; i++)
            		{
            			functionParameters[i + I2C_Mappings[data.I2CRXBuffer[0]].static_pcount] = data.I2CRXBuffer[i+1];
            		}
        		}

        		//If the user sent a valid command, get the I2C bus and call it.
        		if (I2C_Mappings[data.I2CRXBuffer[0]].command_code == data.I2CRXBuffer[0]) {
        			// Pend on the I2C interface becoming available
        			xSemaphoreTake(g_pI2CSemaphore, POLL_SEMAPHORE);
        			// Adhere to I2C timing and pause for 50 microseconds
        			delayUs(I2C_SLAVE_SEND_DLY);
        			// Send the number of values this I2C function will return to the master
        			I2CSlaveDataPut(I2C_BASE_ADDR,I2C_Mappings[data.I2CRXBuffer[0]].return_pcount);
        			I2CMasterControl(I2C_BASE_ADDR, I2C_MASTER_CMD_SINGLE_RECEIVE);
        			// Call the function pointer with the parameters gathered.
            		uint8_t returnValue = I2C_Mappings[data.I2CRXBuffer[0]].func(functionParameters);
            		if (I2C_Mappings[data.I2CRXBuffer[0]].return_pcount == 1) {
            			I2CSlaveDataPut(I2C_BASE_ADDR,returnValue);
            			I2CMasterControl(I2C_BASE_ADDR, I2C_MASTER_CMD_SINGLE_RECEIVE);
            		}
            		xSemaphoreGive(g_pI2CSemaphore);
            		UARTprintf("\nI2CFunctionReturned: 0x%02x\n", returnValue);
        		}
        	}
        }
    }
}

//*****************************************************************************
//
//! Initializes the I2C management task. This should return zero if the task
//! was created successfully.
//
//*****************************************************************************
uint32_t I2CManagerTaskInit(void)
{
	g_pI2CQueue = xQueueCreate(I2C_QUEUE_SIZE, I2C_ITEM_SIZE);

	//
    // Create the I2C Manager task.
    //
    if(xTaskCreate(I2CManagerTask, (const portCHAR *)"I2C_MANAGER", I2CTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_I2CMANAGER_TASK, NULL) != pdTRUE)
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
//! A blank function placeholder for any new I2C commands that have not been
//! created. Most of the empty slots in i2c_task.h use this prototype as a
//! placeholder until a command to call has been designed.
//
//*****************************************************************************
uint8_t I2CNotImplementedFunction(uint8_t params[MAX_PARAMS]) {
	return 0;
}
