/**\file led_task.c
 * \brief <b>A Simple FreeRTOS task designed to blink an LED on the port, pin and interval specified.</b>
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
//! Default LED toggle delay value. LED toggling frequency is twice this number.
//
//*****************************************************************************
#define LED_DEF_TOGGLE_DELAY        125


//*****************************************************************************
//
//! This task toggles the user selected LED at a user selected frequency.
//
//*****************************************************************************
void LEDTask(void *pvParameters)
{
    portTickType ui32WakeTime;
    uint32_t ui32LEDToggleDelay;
    LEDParameters *params = (LEDParameters*)pvParameters;
    ui32WakeTime = xTaskGetTickCount();
    //
    // Initialize the LED Toggle Delay to default value.
    //
    if (params->interval > 0) {
        ui32LEDToggleDelay = params->interval;
    }
    else {
    	ui32LEDToggleDelay = LED_DEF_TOGGLE_DELAY;
    }


    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Get the current tick count.
        //
    	GPIOPinWrite(params->port_base, params->pin_base, params->pin_base);
         //
        // Wait for the required amount of time.
        //
        vTaskDelayUntil(&ui32WakeTime, ui32LEDToggleDelay / portTICK_RATE_MS);

    	if (params->interval > 0) {
    		GPIOPinWrite(params->port_base, params->pin_base, 0);
    	}
        vTaskDelayUntil(&ui32WakeTime, ui32LEDToggleDelay / portTICK_RATE_MS);
    }
}

