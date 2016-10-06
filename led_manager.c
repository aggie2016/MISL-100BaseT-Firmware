/**\file led_manager.c
 * \brief <b>A FreeRTOS task that spawns/deletes LED tasks at runtime.</b>
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
#include "led_task.h"
#include "led_manager.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
//! The stack size for the LED toggle task.
//
//*****************************************************************************
#define LEDTASKSTACKSIZE        500         // Stack size in words

//*****************************************************************************
//
//! The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define LED_ITEM_SIZE           sizeof(LEDProps)
#define LED_QUEUE_SIZE          20

//*****************************************************************************
//
//! Default LED toggle delay value. LED toggling frequency is twice this number.
//
//*****************************************************************************
#define LED_TOGGLE_DELAY        125

//*****************************************************************************
//
//! The queue that holds messages sent to the LED task.
//
//*****************************************************************************
xQueueHandle g_pLEDQueue;
static LEDParameters LEDTaskData;
static void LEDManagerTask(void *pvParameters)
{

	LEDProps LEDToModify;
	static TaskHandle_t TaskHandles[4] = {0};
    //
    // Loop forever.
    //
	GPIOPinWrite(LEDS[0].port_base, LEDS[0].pin_base, LEDS[0].pin_base);
	GPIOPinWrite(LEDS[1].port_base, LEDS[1].pin_base, LEDS[1].pin_base);
	GPIOPinWrite(LEDS[2].port_base, LEDS[2].pin_base, LEDS[2].pin_base);
	GPIOPinWrite(LEDS[3].port_base, LEDS[3].pin_base, LEDS[3].pin_base);

    while(1)
    {
        //
        // Read the next message, if available on queue.
        //
        if(xQueueReceive(g_pLEDQueue, &LEDToModify, NULL) == pdPASS)
        {
        	char TASK_ID[5] = "LED";
        	LEDTaskData.port_base = LEDS[LEDToModify.LEDID].port_base;
        	LEDTaskData.pin_base = LEDS[LEDToModify.LEDID].pin_base;
        	LEDTaskData.interval = LEDToModify.interval;

        	//Is LED ID Valid?
        	if (LEDToModify.LEDID < 4) {
				//Does task exist?
				if (TaskHandles[LEDToModify.LEDID] != 0)
				{
					vTaskDelete(TaskHandles[LEDToModify.LEDID]);
			    	GPIOPinWrite(LEDS[LEDToModify.LEDID].port_base, LEDS[LEDToModify.LEDID].pin_base, LEDS[LEDToModify.LEDID].pin_base);
					TaskHandles[LEDToModify.LEDID] = 0;
				}
				if (!LEDToModify.ClearLED) {
					TASK_ID[4] = LEDToModify.LEDID + '0';
					xTaskCreate(LEDTask, (const portCHAR *)TASK_ID, LEDTASKSTACKSIZE, &LEDTaskData, tskIDLE_PRIORITY + PRIORITY_LED_TASK, &TaskHandles[LEDToModify.LEDID]);
				}
        	}
        }
    }
}

//*****************************************************************************
//
//! Initializes the LED task.
//
//*****************************************************************************
uint32_t LEDManagerTaskInit(void)
{
    g_pLEDQueue = xQueueCreate(LED_QUEUE_SIZE, LED_ITEM_SIZE);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeGPIOOutput(LED1_PORT_BASE, LED1_PIN_BASE | LED2_PIN_BASE | LED3_PIN_BASE | LED4_PIN_BASE);

    //
    // Create the LED task.
    //
    if(xTaskCreate(LEDManagerTask, (const portCHAR *)"LED_MANAGER", LEDTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_LEDMANAGER_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
