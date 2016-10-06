/**\file interpreter_task.c
 * \brief <b>UART CLI Interpreter Task</b>
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
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "utils/uartstdio.h"
#include "interpreter_task.h"
#include "freertos_init.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "string.h"
#include "event_logger.h"


//*****************************************************************************
//
//! The stack size for the LED toggle task.
//
//*****************************************************************************
#define INTERPRETERTASKSTACKSIZE        256         // Stack size in words

//*****************************************************************************
//
//! The item size and queue size for the LED message queue.
//
//*****************************************************************************
#define INTERPRETER_ITEM_SIZE           sizeof(char[UART_RX_BUFFER_SIZE])
#define INTERPRETER_QUEUE_SIZE          10
//*****************************************************************************
//
//! The queue that holds messages sent to the LED task.
//
//*****************************************************************************
xQueueHandle g_pINTERPRETERQueue;
extern xQueueHandle g_pLoggerQueue;

extern xSemaphoreHandle g_pUARTSemaphore;

User_Data ActiveUser;

static void InterpreterTask(void *pvParameters)
{
	//Holds the value passed to us by the xQueueReceive function
    char consoleinput[UART_RX_BUFFER_SIZE] = "";
    //Used for string split function to get the commands issued. Holds a maximum of 128 words.
    static char *commandwords[UART_RX_BUFFER_SIZE] = {{0}};

    while(1)
    {
    	while (!Authenticated) {
    		portTickType ui32WakeTime;
    		int i = 0;
    		char auth_username[16] = {0x00};
    		char auth_password[16] = {0x00};

    		//This task takes a while, so we want share nicely with other tasks instead of blocking
    		uint32_t ui32TaskDelay = SHORT_TASK_DLY;

    		//Get system time in ticks
    		ui32WakeTime = xTaskGetTickCount();
    		UARTprintf("\n\n=== AUTHENTICATION REQUIRED ===\n");

    		while (auth_username[0] == 0x00) {
    			UARTprintf("Username: ");
				//Wait for enter key and pend this task unil done
				while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
					while (!UARTRxBytesAvail()) {
						uint32_t currentTime = xTaskGetTickCount();
						vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
					}
				}

	    		UARTgets(auth_username, 16);
    		}

    		//Replace everything the user receives from the microcontroller with '*'
    		UsePasswordMask = true;

    		while (auth_password[0] == 0x00) {
        		UARTprintf("\nPassword: ");
				//Wait for enter key and pend this task until done
				while ((UARTPeek('\n') == -1) && (UARTPeek('\r') == -1)) {
					while (!UARTRxBytesAvail()) {
						uint32_t currentTime = xTaskGetTickCount();
						vTaskDelayUntil(&ui32WakeTime, ui32TaskDelay / portTICK_RATE_MS);
					}
				}
	    		UARTgets(auth_password, 16);
    		}

    		//Allow cleartext communication over the UART TX buffer.
    		UsePasswordMask = false;



    		//Check to see if user exists
    		for (i=0; i<(MAX_USERS+1);i++) {
    			if (users[i].username[0] != 0x00) {
					if (strcmp(users[i].username, auth_username) == 0)
					{
						if (strcmp(users[i].password, auth_password) == 0) {
							//User authenticated!
							Authenticated = true;
							ActiveUser = users[i];
							ShowDebugInformation();
							UARTprintf("\n\n=== AUTHENTICATION SUCCESSFUL ===\nWelcome %s %s\n", ActiveUser.first_name, ActiveUser.last_name);
						    UARTprintf("For help with a command, append a '?' and hit <ENTER>\n");
						    UARTprintf("ex: port f0 ? \n");
							UARTprintf("\n\033[1m%s\033[0m>", console_hostname);

							//Log to EEPROM
							LogItemEEPROM(UserLoggedIn);

							break;
						}
					}
    			}
    		}
    		if (!Authenticated) {
    		UARTprintf("\nAUTHENTICATION FAILED!\n");
    		}
    	}
        //
        // Read the next message, if available on queue.
        //
        if(xQueueReceive(g_pINTERPRETERQueue, &consoleinput, (portTICK_PERIOD_MS*100)) == pdPASS)
        {
        	char *token = strtok (consoleinput, " ");
        	int n_spaces = 0, i,j,k=0,l=0,longest_string=0;
        	char *params[MAX_PARAMS] = {0};
        	bool foundChild = false;
            xSemaphoreTake(g_pUARTSemaphore,portMAX_DELAY);

            //Empty and reinitialize the commandwords array
            memset(commandwords, 0, sizeof(commandwords));


        	while (token) {
        		++n_spaces;
        		commandwords[n_spaces-1] = token;
        		token = strtok (NULL, " ");
        		if (n_spaces > 126) {
        			UARTprintf("Command issued is too long. A maximum of 127 words can be issued at any given time.");
        		}
        	}

        	UARTprintf("\n");


        	Command (*Menu)[MAX_MENU_ITEMS];
        	Menu = &Command_Categories;

        	//Check if user hit enter without entering parameters
        	if (commandwords[0] != 0x00) {
				for (i = 0; i < MAX_DEPTH_INHERITANCE; i++) {
					for (j = 0; j < MAX_MENU_ITEMS; j++) {


						if ((*Menu)[j].text == 0 && !foundChild)
						{
							//End of array found without a match. Inform user and print help menu for that menu
							UARTprintf("Command Not Recognized.\n");
							i = MAX_DEPTH_INHERITANCE;
							break;
						}
						else if ((*Menu)[j].text == 0 && foundChild) {

							//Show help for the current menu since we received a partial command
							UARTprintf("Incomplete Command Entered: \n");
							//Print the commands issued up to the error
							for (k = 0; k < i; k++) {

								UARTprintf("%s ", commandwords[k]);
							}

							UARTprintf("<incomplete>\nFor help with commands, type a '?' after the command.\n");

							//Set the outer loop iterator to the maximum so we will exit after the next break
							i = MAX_DEPTH_INHERITANCE;
							break;
						}
						//Did we receive a valid command word?
						if ((strcmp((*Menu)[j].text,commandwords[i]) == 0 || ((*Menu)[j].paramsUserProvided && i < n_spaces)) && (strcmp("?", commandwords[i]) != 0))
						{
								//Save this branch's function parameters
								if ((*Menu)[j].paramsRequired != NO_PARAMETERS) {
									if ((*Menu)[j].paramsUserProvided) {
										for (l = 0; l < (*Menu)[j].paramsRequired; l++) {
												params[k] = commandwords[i];
												k++;

										}
									}
									else {
										for (l=0; l < (*Menu)[j].paramsRequired; l++) {
											params[k] = (*Menu)[j].functionParams[l];
											k++;
										}
									}
								}
								//Is this is termininating command? If so, run the function tied to it
								if ((*Menu)[j].isExecutable) {
									if (commandwords[i+1] != 0x00)
									{
										UARTprintf("Invalid Command, too many parameters entered!\n");
									}
									else if ((*Menu)[j].permissionsRequired > ActiveUser.permissions) {
										UARTprintf("[UNAUTHORIZED]: You require elevated permissions to use this command!\n");
									}
									else {
										//Call function here
										if ((*Menu)[j].func(params) == true) {
											UARTprintf("\nCommand Executed Successfully\n");
										}
										else {
											UARTprintf("\nAn error occurred while executing this task.\n");
										}
									}

									i = MAX_DEPTH_INHERITANCE;
									break;
							}
							else {
								//Notify future iterations that we are now dealing with child nodes.
								foundChild = true;
								//Set the menu item we are looking at to the child of the current menu item
								Menu = ((*Menu)[j].childCommand);
								break;
							}
						}
						else if (strcmp("?", commandwords[i]) == 0) {
							bool childHasElevatedPermissions = false;
							//User has requested help for this menu. Print help!
							//In order to make sure help text is aligned properly, first find the longest string
							for (k=0; k<MAX_MENU_ITEMS;k++) {
								if ((*Menu)[k].text == 0){
									//End of help
									break;
								}
								else if (strlen((*Menu)[k].text) > longest_string){

									longest_string = strlen((*Menu)[k].text);
								}
							}

							//Print the help text with spaces added for all words shorter than the longest found
							for (k=0; k<MAX_MENU_ITEMS;k++) {
								if ((*Menu)[k].text == 0){
									//End of help
									break;
								}
								UARTprintf("\t%s", (*Menu)[k].text);
								if ((*Menu)[k].permissionsRequired > ActiveUser.permissions) {
									UARTprintf("*");
									childHasElevatedPermissions = true;
								}
								for (l = 0; l < (longest_string - strlen((*Menu)[k].text)); l++)
								{
									UARTprintf(" ");
								}
								if ((*Menu)[k].permissionsRequired > ActiveUser.permissions) {
									UARTprintf("\b");
								}
								UARTprintf("\t%s\n", (*Menu)[k].help);
							}
							if (childHasElevatedPermissions) {
							UARTprintf("\n[*] Command requires elevated priviledges!\n");
							}

							i = MAX_DEPTH_INHERITANCE;
							break;
						}




					}
				}
        	}

        	//Print the command line message
        	UARTprintf("\033[1m%s\033[0m>", console_hostname);

        	xSemaphoreGive(g_pUARTSemaphore);
        }
    }
}

//*****************************************************************************
//
//! Initializes the Interpreter task.
//
//*****************************************************************************
uint32_t InterpreterTaskInit(void)
{

	g_pINTERPRETERQueue = xQueueCreate(INTERPRETER_QUEUE_SIZE, INTERPRETER_ITEM_SIZE);

    //
    // Create the Interpreter task.
    //
    if(xTaskCreate(InterpreterTask, (const portCHAR *)"InterpreterTask", INTERPRETERTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_INTERPRETER_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}

bool NotImplementedFunction(char* x[MAX_PARAMS]) {
	UARTprintf("Function Not Implemented!");
	return true;
}

