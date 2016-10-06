/**\file port_monitor_task.c
 * \brief <b>A FreeRTOS task that periodically scans all of the port interrupt registers for status changes.</b>
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
#include "utils/uartstdio.h"
#include "port_monitor_task.h"
#include "freertos_init.h"
#include "interpreter_task.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
//! An external reference to the UART mutex that blocks communication with the
//! interface until it becomes available.
//
//*****************************************************************************
extern xSemaphoreHandle g_pUARTSemaphore;


//*****************************************************************************
//
//! PORT MONITOR TASK
//! This task periodically scans the interrupt status register in the KSZ8895MLUB
//! (register 0x7C) for active high bits. When one is found, the corresponding
//! port will have it's MAC address learning turned off temporarily so that
//! a flush of all dynamic MACs addresses learned can be performed. Once complete
//! the interrupt bit is cleared and the user is informed that the port has
//! been (connected/disconnected).
//!
//! This task will only scan the connected ports after a user has successfully
//! logged into the system.
//
//*****************************************************************************
static void PortMonitorTask(void *pvParameters)
{
    while(1)
    {
		uint32_t taskDelay = LONG_RUNNING_TASK_DLY;
		uint32_t currentTime = xTaskGetTickCount();

		if (Authenticated) {
			//Check all port interrupt flags
			uint32_t flags = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INTERRUPT_STATUS_REGISTER);
			uint8_t eth0_reg_settings = 0x00;
			if (flags & 0x10) { // PORT 5
				//Clear port interrupt flag
				flags = 0x10;
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INTERRUPT_STATUS_REGISTER, flags);

				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x5E);
				if ((eth0_reg_settings >> 5) & 1) {
					UARTprintf("\n[SYSTEM]: Expansion port connected!\n");
				}
				else {
					UARTprintf("\n[SYSTEM]: Expansion port disconnected!\n");
				}

				//Disable port learning for port 5
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x52);
				eth0_reg_settings |= (1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x52, eth0_reg_settings);

				//Flush Dyn MAC Table for this port
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX);
				eth0_reg_settings |= (1 << 5);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX, eth0_reg_settings);

				while ((EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX) >> 5) 	& 	1) {
					uint32_t currentTime = xTaskGetTickCount();
					vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
				}

				//Enable port learning for port 5
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x52);
				eth0_reg_settings &= ~(1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x52, eth0_reg_settings);

			}
			if (flags & 0x08) { // PORT 4
				//Clear port interrupt flag
				flags = 0x08;
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INTERRUPT_STATUS_REGISTER, flags);

				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x4E);
				if ((eth0_reg_settings >> 5) & 1) {
					UARTprintf("\n[SYSTEM]: Port 0 connected!\n");
				}
				else {
					UARTprintf("\n[SYSTEM]: Port 0 disconnected!\n");
				}

				//Disable port learning for port 4
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x42);
				eth0_reg_settings |= (1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x42, eth0_reg_settings);

				//Flush Dyn MAC Table for this port
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX);
				eth0_reg_settings |= (1 << 5);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX, eth0_reg_settings);

				while ((EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX) >> 5) 	& 	1) {
					uint32_t currentTime = xTaskGetTickCount();
					vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
				}

				//Enable port learning for port 4
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x42);
				eth0_reg_settings &= ~(1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x42, eth0_reg_settings);

			}
			if (flags & 0x04) { // PORT 3
				//Clear port interrupt flag
				flags = 0x04;
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INTERRUPT_STATUS_REGISTER, flags);

				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x3E);
				if ((eth0_reg_settings >> 5) & 1) {
					UARTprintf("\n[SYSTEM]: Port 1 connected!\n");
				}
				else {
					UARTprintf("\n[SYSTEM]: Port 1 disconnected!\n");
				}

				//Disable port learning for port 3
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x32);
				eth0_reg_settings |= (1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x32, eth0_reg_settings);

				//Flush Dyn MAC Table for this port
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX);
				eth0_reg_settings |= (1 << 5);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX, eth0_reg_settings);

				while ((EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX) >> 5) 	& 	1) {
					uint32_t currentTime = xTaskGetTickCount();
					vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
				}

				//Enable port learning for port 3
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x32);
				eth0_reg_settings &= ~(1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x32, eth0_reg_settings);
			}
			if (flags & 0x02) { // PORT 2
				//Clear port interrupt flag
				flags = 0x02;
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INTERRUPT_STATUS_REGISTER, flags);

				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x2E);
				if ((eth0_reg_settings >> 5) & 1) {
					UARTprintf("\n[SYSTEM]: Port 2 connected!\n");
				}
				else {
					UARTprintf("\n[SYSTEM]: Port 2 disconnected!\n");
				}


				//Disable port learning for port 2
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x22);
				eth0_reg_settings |= (1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x22, eth0_reg_settings);

				//Flush Dyn MAC Table for this port
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX);
				eth0_reg_settings |= (1 << 5);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX, eth0_reg_settings);

				while ((EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX) >> 5) 	& 	1) {
					uint32_t currentTime = xTaskGetTickCount();
					vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
				}

				//Enable port learning for port 2
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x22);
				eth0_reg_settings &= ~(1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x22, eth0_reg_settings);
			}
			if (flags & 0x01) { // PORT 1
				//Clear port interrupt flag
				flags = 0x01;
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INTERRUPT_STATUS_REGISTER, flags);

				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x1E);
				if ((eth0_reg_settings >> 5) & 1) {
					UARTprintf("\n[SYSTEM]: Port 3 connected!\n");
				}
				else {
					UARTprintf("\n[SYSTEM]: Port 3 disconnected!\n");
				}

				//Disable port learning for port 4
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x12);
				eth0_reg_settings |= (1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x12, eth0_reg_settings);

				//Flush Dyn MAC Table for this port
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX);
				eth0_reg_settings |= (1 << 5);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX, eth0_reg_settings);

				while ((EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GLOBAL_CONTROL_0_HEX) >> 5) 	& 	1) {
					uint32_t currentTime = xTaskGetTickCount();
					vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
				}

				//Enable port learning for port 4
				eth0_reg_settings = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x12);
				eth0_reg_settings &= ~(1 << 0);
				EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x12, eth0_reg_settings);
			}
		}


		vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);

    }
}

//*****************************************************************************
//
//! Initializes and creates the port monitoring task.
//
//*****************************************************************************
uint32_t PortManagerTaskInit(void)
{
    if(xTaskCreate(PortMonitorTask, (const portCHAR *)"PORT_MONITOR", PORT_MONITOR_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_PORT_MONITOR_TASK, NULL) != pdTRUE)
    {
        return(1);
    }
    //
    // Success.
    //
    return(0);
}
