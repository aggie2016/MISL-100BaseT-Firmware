/**\file led_manager.h
 * \brief <b>LED Settings and function prototypes</b>
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

#ifndef LED_MANAGER_H_
#define LED_MANAGER_H_

//*****************************************************************************
//
//! \brief Parameters that the LED manager uses to assign LEDs to logical identifiers.
//
//*****************************************************************************
typedef struct LEDParams {
	//! The base address of the port to use for this LED task
	uint32_t port_base;
	//! The pin to use when toggling the LED
	uint32_t pin_base;
	//! The interval in (ms) to use when toggling the LED
	uint32_t interval;
} LEDParameters;

//*****************************************************************************
//
//! \brief Parameters that the LED manager uses to pass function parameters to each LED
//! task it spawns.
//
//*****************************************************************************
typedef struct LEDInfo {
	//! A number representing which LED this new task should control (0 - 3)
	uint8_t LEDID;
	//! An interval in (ms) to use when toggling the LED on and off. An interval
	//! less than or equal to zero means that this task should hold the LED on
	//! until deleted.
	uint32_t interval;
	//! Should this LED task be deleted?
	bool ClearLED;
} LEDProps;

//*****************************************************************************
//
//! Configuration settings that allow the developer to change which port and pins
//! are used as LEDs
//
//*****************************************************************************
#define LED1_PIN_BASE GPIO_PIN_0
#define LED2_PIN_BASE GPIO_PIN_1
#define LED3_PIN_BASE GPIO_PIN_2
#define LED4_PIN_BASE GPIO_PIN_3

#define LED1_PORT_BASE GPIO_PORTE_BASE
#define LED2_PORT_BASE GPIO_PORTE_BASE
#define LED3_PORT_BASE GPIO_PORTE_BASE
#define LED4_PORT_BASE GPIO_PORTE_BASE


//*****************************************************************************
//
//! Once configured above, the "LEDS" array assigns a port and pin to a logical
//! LED. The LED manager then spawns four tasks on demand that are passed a single
//! entry in this array.
//
//*****************************************************************************
static LEDParameters LEDS[4] = {
		{LED1_PORT_BASE, LED1_PIN_BASE},
		{LED2_PORT_BASE, LED2_PIN_BASE},
		{LED3_PORT_BASE, LED3_PIN_BASE},
		{LED4_PORT_BASE, LED4_PIN_BASE}
};

extern uint32_t LEDManagerTaskInit(void);

#endif /* LED_MANAGER_H_ */
