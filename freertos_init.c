/**\file freertos_init.c
 * \brief <b>Main Entry Point for the EEE 100BaseTX Switch Layer Firmware</b>
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

#include <eee_hal.h>
#include <event_logger.h>
#include <stdbool.h>
#include <stdint.h>
#include "stdio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_i2c.h"
#include "gpio.h"
#include "pin_map.h"
#include "rom.h"
#include "ssi.h"
#include "i2c.h"
#include "sysctl.h"
#include "uart.h"
#include "interrupt.h"
#include "uartstdio.h"
#include "led_manager.h"
#include "interpreter_task.h"
#include "event_logger.h"
#include "port_monitor_task.h"
#include "i2c_task.h"
#include "freertos_init.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "string.h"

//*****************************************************************************
//
// The mutex that protects concurrent access of UART/SSI from multiple tasks.
//
//*****************************************************************************

//*****************************************************************************
//
//! Mutex to control and block access to the UART RX and TX buffers. Prevents
//! multiple FreeRTOS tasks from interacting with this interface at the same
//! time.
//
//*****************************************************************************
xSemaphoreHandle g_pUARTSemaphore;
//*****************************************************************************
//
//! Mutex to control and block access to the SSI0 [EEPROM] RX and TX buffers.
//! Prevents multiple FreeRTOS tasks from interacting with this interface at
//! the same time.
//
//*****************************************************************************
xSemaphoreHandle g_pSPI0Semaphore;
//*****************************************************************************
//
//! Mutex to control and block access to the SSI0 [Ethernet Controller] RX and
//! TX buffers. Prevents multiple FreeRTOS tasks from interacting with this
//! interface at the same time.
//
//*****************************************************************************
xSemaphoreHandle g_pSPI1Semaphore;
//*****************************************************************************
//
//! Mutex to control and block access to the I2C RX and TX buffers.
//! Prevents multiple FreeRTOS tasks from interacting with this
//! interface at the same time.
//
//*****************************************************************************
xSemaphoreHandle g_pI2CSemaphore;

//*****************************************************************************
//
//! External handle to the LED task Queue.
//
//*****************************************************************************
extern xQueueHandle g_pLEDQueue;
//*****************************************************************************
//
//! External handle to the I2C task Queue.
//
//*****************************************************************************
extern xQueueHandle g_pI2CQueue;
//*****************************************************************************
//
//! External handle to the EEPROM System Logger task Queue.
//
//*****************************************************************************
extern xQueueHandle g_pLoggerQueue;
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif


//*****************************************************************************
//
// Ignore warning messages where arrays are passed as parameters to array-to-pointer
// parameters in function calls.
//
//*****************************************************************************
#pragma diag_suppress 515
#pragma diag_suppress 169

//*****************************************************************************
//
//! Global Variable used to determine whether a valid user has signed in yet.
//! Prevents interpreter from receiving commands until a valid user has accessed
//! the system.
//
//*****************************************************************************
bool Authenticated = false;
//*****************************************************************************
//
//! Global Variable used to replace the UART echoed characters with '*' until
//! a valid password has been entered.
//
//*****************************************************************************
bool UsePasswordMask = false;
//*****************************************************************************
//
//! Global Variable to enable/disable the ability of the UART interpreter to
//! receive input from the CLI.
//
//*****************************************************************************
bool UARTInterpreterEnabled = true;


//*****************************************************************************
//
//! Default values for system users. Places default root credentials outside of
//! the bounds provided by MAX_USERS (15).
//
//*****************************************************************************
User_Data users[16] = {
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{0,0,0,0,ReadOnlyUser,false, None},
		{"root","DEFAULT","ADMIN","root",Administrator,false, None}
};



//*****************************************************************************
//
//! A hook called by FreeRTOS when a stack overflow error is detected. Used in
//! in this firmware to notify the user when the system has stopped due to an
//! illegal access beyond that allocated to each task.
//!
//! \param *pxTask pointer the task that caused the stack overflow
//! \param *pcTaskName string name for the task
//!
//! \return Returns void
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //

	//Task that this stack overflow occured on
	char *task_name = pcTaskGetTaskName(&pxTask);
	//Task that threw the stack overflow notification
	char *calling_task_name = pcTaskGetTaskName(NULL);
	//State of the task that a stack overflow occurred on
	eTaskState task_state = eTaskGetState(&pxTask);

	//Display this information to the user.
	UARTprintf("Task encountered a stack overflow error: \n\tTask Name: %s\n\tTask State: %s\n\tCalling Task: %s", task_name, TASK_STATES[task_state],calling_task_name);

	uint32_t taskDelay = LONG_RUNNING_TASK_DLY;
	uint32_t currentTime;

    if (eTaskGetState(LoggerTaskHandle) == eReady || eTaskGetState(LoggerTaskHandle) == eRunning || eTaskGetState(LoggerTaskHandle) == eBlocked) {
    	LoggerCodes StackOverflow_Exception = StackOverflow;
    	xQueueSend(g_pLoggerQueue,&StackOverflow_Exception,0);
    }
    while (uxQueueMessagesWaiting(g_pLoggerQueue)) {
    	currentTime = xTaskGetTickCount();
    	vTaskDelayUntil(&currentTime, taskDelay / portTICK_RATE_MS);
    }

    while(1)
    {
    	//Wait here for someone to help us! (Lost in thought, please send search party)
    }
}

//*****************************************************************************
//
//! Displays the current system status/version information on load to the user.
//! Provides information regarding the status of devices that should be connected.
//! If any disconnected system devices are found, those devices are highlighted
//! in red as "FAILED" tests.
//!
//! \return Returns void
//
//*****************************************************************************
void ShowDebugInformation() {
    //Print initialization text to the commmand prompt window
	UARTprintf("\033[8;45;100t\n");
    UARTprintf("\033[2J\n[Console Mode]: Operating in VT100/ASCII Mode\n");
    UARTprintf("[Auto]: Set Window Size to 100x45\n");
    UARTprintf("\nEagle Embedded Engineering 100BaseTX Switch Configuration Interface\n");

    //Test and verify operation of EEPROM
    UARTprintf("[BOOTING]: Testing EEPROM:");
    if (EEPROMSingleWrite(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, 0x00000001, 0x19)) {
    	UARTprintf(" \033[30;42mPASSED!\033[0m\n");
    }
    else {
    	UARTprintf(" \033[30;41mFAILED!\033[0m\n");
    }

    //Test and verify operation of Ethernet Controllers
    UARTprintf("[BOOTING]: Testing Ethernet Controller:");
    if (EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,0x01) > 0) {
    	UARTprintf(" \033[30;42mPASSED!\033[0m\n");
    }
    else {
    	UARTprintf(" \033[30;41mFAILED!\033[0m\n");
    }


    UARTprintf("\n-----------------------------------------\n");
    UARTprintf(  "|  (c) 2016 Eagle Embedded Engineering  |\n");
    UARTprintf(  "|       MISL Ethernet Switch Layer      |\n");
    UARTprintf(  "|            Firmware v%d.%d.%d           |\n", MAJOR_VERSION, MINOR_VERSION, REVISION);
    UARTprintf(  "-----------------------------------------\n");
    UARTprintf(  "-----------------------------------------\n");
    UARTprintf(  "|      Enter commands one at a time     |\n");
    UARTprintf(  "| followed by a single carriage return  |\n");
    UARTprintf(  "-----------------------------------------\n\n");

}

//*****************************************************************************
//
//! This hook is called when the DTS pin on the connected UART-to-USB converter
//! is driven active-low
//!
//! \return Returns void
//
//*****************************************************************************
void
DTSTriggered(void)
{
	 LEDProps DTSPin;
	 DTSPin.LEDID = 0;
	 DTSPin.interval = CONSOLE_OPEN_LED_BLINKRATE;
	 DTSPin.ClearLED = false;

	 char AuthString[20] = {0};
	 int i = 0;
     //Clear the incoming interrupt flag and disable it temporarily
	 GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_6);
	 GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_6);


	 //Remove this function from the ISR table as we will be replacing it with an ISR that deals with console disconnect events
	 GPIOIntUnregister(GPIO_PORTD_BASE);

	 //Register the function that tells the microcontroller a console port has been disconnected (DTR is off)
	 GPIOIntRegister(GPIO_PORTD_BASE, DTSOFF);



	 //[TODO]: Write routine to send test characters that only a windows application would see over a console
	 UARTFlushRx();
	 UARTFlushTx(true);
	 UARTprintf("EEE\n");
	 //Wait for short period for Windows App to process characters
	 delayMs(50);
	 UARTEchoSet(false);
     while(UARTCharsAvail(UART1_BASE) != false) {
    	 if (i < 20) {
    	 AuthString[i] = UARTCharGetNonBlocking(UART1_BASE);
    	 }
    	 i++;
     }

	 if (strcmp(AuthString,"EEEWinApp2016") == 0) {
		 UARTprintf("WinAppModeActivated\n");
		 ConsoleMode = false;
	 }
	 else {
		 ConsoleMode = true;
	 }


	 if (ConsoleMode) {
		UARTEchoSet(true);
		Authenticated = false;

	 }
	 else {
		 //Initialize communication with a windows application
	 }

	 //Let the input pin settle before filling up our queue unecessarily
	 delayMs(1);

	 if (ENABLE_LED_MANAGER) {
		 if(xQueueSendFromISR(g_pLEDQueue, &DTSPin, NULL) != pdPASS)
		 {
			 //
			 // Error. The queue should never be full. If so print the
			 // error message on UART and wait for ever.
			 //
			 UARTprintf("\nQueue full. This should never happen.\n");
			 while(1)
			 {
			 }
		 }
	 }


	//Enable the other interrupt
	GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_RISING_EDGE);
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_6);

	UARTFlushTx(true);
	UARTprintf("\n\n=== AUTHENTICATION REQUIRED ===\nUsername: ");

}

//*****************************************************************************
//
//! This hook is called when the DTS pin on the connected UART-to-USB converter
//! is driven active-high meaning the user is no longer connected to us!
//!
//! \return Returns void
//
//*****************************************************************************
void DTSOFF(void)
{
	LEDProps DTSPin;

	GPIOIntClear(GPIO_PORTD_BASE, GPIO_INT_PIN_6);
	GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_6);
	GPIOIntUnregister(GPIO_PORTD_BASE);
	GPIOIntRegister(GPIO_PORTD_BASE, DTSTriggered);

	DTSPin.LEDID = 0;
	DTSPin.interval = CONSOLE_OPEN_LED_BLINKRATE;
	DTSPin.ClearLED = true;

	delayMs(1);

	 if (ENABLE_LED_MANAGER) {
		if(xQueueSendFromISR(g_pLEDQueue, &DTSPin, NULL) != pdPASS)
		{

		 //
		 // Error. The queue should never be full. If so print the
		 // error message on UART and wait for ever.
		 //
			while(1)
			{
			}
		}
	 }
	GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_6);
}

//*****************************************************************************
//
//! Configure the UART and its pins.  This must be called before UARTprintf().
//!
//! \return Returns void
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    UARTEchoSet(true);
    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(1, 115200, 16000000);
    ///
    /// SETUP THE DATA TERMINAL READY (DTS) PIN DETECTION
    /// This will tell the microcontroller when the user opens a valid console connection. Pin B4 should be tied to the DTS pin
    /// on the UART-to-USB converter IC
    ///

}

//*****************************************************************************
//
//! Configure the SSI interfaces with the appropriate GPIO pins and enable them.
//!
//! \return Returns void
//
//*****************************************************************************
void ConfigureSSI(void) {

	//*************************************************
	//
	// Configure and Initialize all pins used for
	// communication over SSI0 (SPI0) EEPROM
	//
	//*************************************************
	//Enable SSI0
	ROM_SysCtlPeripheralEnable(EEPROM_SYS_PORT_BASE);
	ROM_SysCtlPeripheralEnable(EEPROM_SYS_BASE);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	ROM_SysCtlPeripheralEnable(ETHO_1_SYS_PORT_BASE);
	ROM_SysCtlPeripheralEnable(ETHO_1_SYS_BASE);



	//
	// Configure GPIO Pins for SPI Mode [Using SSI0]
	//
	ROM_GPIOPinConfigure(EEPROM_SSI_CLK);
	//ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	ROM_GPIOPinConfigure(EEPROM_SSI_RX);
	ROM_GPIOPinConfigure(EEPROM_SSI_TX);
	GPIOPinTypeGPIOOutput(EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN);

	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);

	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);

    GPIOPadConfigSet(EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeSSI(EEPROM_SSI_CS_BASE, EEPROM_SSI_CLK_PIN | EEPROM_SSI_RX_PIN | EEPROM_SSI_TX_PIN);

    SSIConfigSetExpClk(EEPROM_BASE_ADDR, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 1000000, 8);
    SSIEnable(EEPROM_BASE_ADDR);
	//*************************************************
	//
	// Configure and Initialize all pins used for
	// communication over SSI1 (SPI1) Ethernet Controller
	//
	//*************************************************

	//
	// Configure GPIO Pins for SPI Mode [Using SSI1]
	//
	ROM_GPIOPinConfigure(ETHO_1_SSI_CLK);
	//ROM_GPIOPinConfigure(GPIO_PD1_SSI1FSS);
	ROM_GPIOPinConfigure(ETHO_1_SSI_RX);
	ROM_GPIOPinConfigure(ETHO_1_SSI_TX);
	GPIOPinTypeGPIOOutput(ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN);
    GPIOPadConfigSet(ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeSSI(ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CLK_PIN | ETHO_1_SSI_RX_PIN | ETHO_1_SSI_TX_PIN);

    SSIConfigSetExpClk(ETHO_1_BASE_ADDR, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 4000000, 8);
    SSIEnable(ETHO_1_BASE_ADDR);


	//*************************************************
	//
	// Configure and Initialize all pins used for
	// communication over SSI2 (SPI2)
	//
	//*************************************************
}

//*****************************************************************************
//
//! Configure the I2C interface with the appropriate GPIO pins and enable it.
//! Additionally, this function sets up this switch layer as a slave device with
//! the address specified in this file's header.
//!
//! \return Returns void
//
//*****************************************************************************
void ConfigureI2C(void) {

	ROM_SysCtlPeripheralEnable(I2C_SYS_BASE);
	ROM_SysCtlPeripheralEnable(I2C_SYS_PORT_BASE);

    //
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    //

    GPIOPinConfigure(I2C_SCL);
    GPIOPinConfigure(I2C_SDA);

    //
    // Select the I2C function for these pins.  This function will also
    // configure the GPIO pins pins for I2C operation, setting them to
    // open-drain operation with weak pull-ups.
    //
    GPIOPinTypeI2C(I2C_PORT_BASE, I2C_SDA_PIN);
    GPIOPinTypeI2CSCL(I2C_PORT_BASE, I2C_SCL_PIN);

    //Enable Loopback mode
    //HWREG(I2C0_BASE + I2C_O_MCR) |= 0x01;

    //Enable I2C Interrupts
    IntEnable(INT_I2C0);

    // Set the slave address for the I2C port
    I2CSlaveInit(I2C_BASE_ADDR, I2C_DEVICE_ADDR);

    // Set the slave address for the I2C port
    I2CSlaveAddressSet(I2C_BASE_ADDR, 0,I2C_DEVICE_ADDR);

    //Setup communications over I2C as a slave device
    I2CSlaveIntEnableEx(I2C_BASE_ADDR, I2C_SLAVE_INT_START|I2C_SLAVE_INT_STOP|I2C_SLAVE_INT_DATA);

    //
    // Setup I2C master clock speed using system clock. If the third parameter
    // is setup as true, the I2C port will operate at 400 kbps. Otherwise, the
    // bitrate of the port will be 100 kbps.
    //
	I2CMasterInitExpClk(I2C_BASE_ADDR, SysCtlClockGet(), false);

	// Enable this device as a slave device. Since all communication is being sent
	// from master to slave while loopback is enabled, then this statement
	// is necessary to configure the port to allow reception of data
	//
    I2CSlaveEnable(I2C_BASE_ADDR);


    // Configure the master I2C port as a read-only device. This can be altered during
    // runtime
    I2CMasterSlaveAddrSet(I2C_BASE_ADDR, I2C_DEVICE_ADDR, false);


}


void ConfigureWatchdog(void) {
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    //
    // Enable the watchdog interrupt.
    //
    ROM_IntEnable(INT_WATCHDOG);
    //
    // Set the period of the watchdog timer.
    //
    ROM_WatchdogReloadSet(WATCHDOG0_BASE, ROM_SysCtlClockGet());
    //
    // Enable reset generation from the watchdog timer.
    //
    ROM_WatchdogResetEnable(WATCHDOG0_BASE);
    //
    // Enable the watchdog timer.
    //
    ROM_WatchdogEnable(WATCHDOG0_BASE);
}

//*****************************************************************************
//
//! Interrupt handler for all I2C communication. Detects when a valid command has
//! been issued by checking the number of parameters sent over I2C. Once validated,
//! the information is placed on the I2CQueue as an I2C_Packet. This information is
//! used to call the appropriate function pointer that will execute the users' request.
//! EXAMPLE:
//! 	<START BIT> <7-bit ADDRESS> <I2C CODE> <PARAMETER N> <PARAMETER N+1> <END>
//!
//! If the function called returns values, the master will have to request N number of
//! values over I2C. For example, if the function exports the current running configuration
//! (256 values), the I2C master will have to reqest information 256 times.
//!
//! \return Returns void
//
//*****************************************************************************
void I2C0SlaveIntHandler(void)
{
	static I2C_Packet data;

	memset(&data, 0, sizeof(I2C_Packet));

    // Clear the I2C0 interrupt flag.
    uint32_t int_status = I2CSlaveIntStatusEx(I2C_BASE_ADDR, false);
    uint32_t slave_status = I2CSlaveStatus(I2C_BASE_ADDR);
    I2CSlaveIntClearEx(I2C_BASE_ADDR, int_status);
    // Disable the I2C0 interrupt while processing the request
    I2CSlaveIntDisable(I2C_BASE_ADDR);

    xSemaphoreTakeFromISR(g_pI2CSemaphore, NULL);

    if (slave_status & I2C_SLAVE_ACT_RREQ) {
    	//Is this I2C interrupt from a START command?
		if (int_status & I2C_SLAVE_INT_START ) {
			//Reset the indexer
			data.I2CRXIndex = 0;
		}
		else if (int_status & I2C_SLAVE_INT_DATA) {
			// Read a byte from the master.
			data.I2CRXBuffer[data.I2CRXIndex] = I2CSlaveDataGet(I2C_BASE_ADDR);
			//Is the current number of parameters received greater than the max?
			if (data.I2CRXIndex > I2CBUFFERSIZE)
			{
				//If so, wrap back to zero
				data.I2CRXIndex = 0;
			}
			//Is the current number of parameters equal to or greater than the number of custom parameters required for the sent code?
			if (data.I2CRXIndex >= I2C_Mappings[data.I2CRXBuffer[0]].custom_pcount)
			{
				//If so, place this packet in the I2CManager queue
				UARTprintf("\nDetectedI2CCode: 0x%02x\n", I2C_Mappings[data.I2CRXBuffer[0]].command_code);
				if(xQueueSendFromISR(g_pI2CQueue, &data, NULL) != pdPASS)
				{
				 //
				 // Error. The queue should never be full. If so print the
				 // error message on UART and wait for ever.
				 //
				 UARTprintf("\nQueue full. This should never happen.\n");
				 while(1)
				 {
				 }
				}
			}

			data.I2CRXIndex++;
		}
		else if (int_status & I2C_SLAVE_INT_STOP) {
			//We received a stop commmand.
		}
    }

    if (int_status & I2C_MASTER_INT_DATA) {
    	UARTprintf("Value read back from slave: %02X\n", I2CMasterDataGet(I2C_BASE_ADDR));
    }

    xSemaphoreGiveFromISR(g_pI2CSemaphore, NULL);
    I2CSlaveIntEnable(I2C_BASE_ADDR);
}


//*****************************************************************************
//
//! The interrupt handler for the watchdog.  This feeds the dog (so that the
//! processor does not get reset) and winks the LED connected to GPIO B3.
//
//*****************************************************************************
void
WatchdogIntHandler(void)
{
    //
    // Clear the watchdog interrupt.
    //
    ROM_WatchdogIntClear(WATCHDOG0_BASE);
}


//******************************************************************************
//
//! Function that erases all sectors in the Micrel AA1024 EEPROM and then
//! overwrites all used sectors with zeros. This intialization routine is only
//! called on first boot as specified by bit 2 in register 0x1E. If this bit is
//! '0', the initialization is performed and the bit is set to '1'.
//!
//! \return Returns void
//
//******************************************************************************
bool InitializeEEPROM(void) {
	//LOAD CONFIGURATION FROM EEPROM TO ETHERNET CONTROLLER
	uint8_t FirmwareSettings = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,EEPROM_FIRMWARE_SETTINGS), vlan_data;
	uint32_t reg = 0, progress = 0, users_cnt = 0;

	UARTprintf("\033[2J");

	//Was the EEPROM initialized? This bit should be a '1'
	if ((FirmwareSettings & 0x80) == 0x80)
	{
		UARTprintf("[BOOTING]: Reintializing EEPROM...");
		EEPROMChipErase(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN);
		UARTprintf("DONE!\n");
	}
	if ((FirmwareSettings & 0x40) == 0x40) {
		//Load config from Ethernet Controller
		UARTprintf("\n[BOOTING]: Loading configuration from memory...please wait\n");
		progress = CreateProgressBar();
		for (reg = 0; reg < 0xFF; reg++)
		{
			if (EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,reg,EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_SWITCH_CONFIG_BASE + reg))))
			{
				UpdateProgressBar(&progress, Increment, (100*reg)/0xFF);
				//ShowProgress((100*reg)/0xFF);
				delayMs(10);
			}
		}
		UARTprintf("\n");

		//Load Log Status Flags from EEPROM registers [0x1F - 0x22]
		LogStatusFlags = (EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_1) << 24) 		|
							(EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_2) << 16) 	|
							(EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_3) << 8) 	|
							(EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_LOGFLAGS_4));

		//Load last used log slot for this iteration
		NextLogSlot = (EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_1) << 24) 			|
							(EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_2) << 16) 	|
							(EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_3) << 8) 	|
							(EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN, EEPROM_FIRMWARE_NEXTLOG_4));

		if (NextLogSlot < EEPROM_LOG_BASE) {
			NextLogSlot = EEPROM_LOG_BASE;
		}

		if ((FirmwareSettings & 0x20) == 0x20) {
			//Load config from Ethernet Controller
			UARTprintf("\n[BOOTING]: Reconfiguring VLANS from memory...please wait\n");
			progress = CreateProgressBar();
			for (reg = 0; reg < 4095; reg++)
			{
				vlan_data = EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_VLAN_TABLE_BASE + reg));
				if ((vlan_data & 0x80) == 0x80)
				{
					//Valid VLAN, Save to VLAN table
					uint8_t port_membership = 0x00;

					uint32_t indirect_reg_addr = ((reg+1) / 4), indirect_reg_data = 0x00;
					//Obtain current indirect access data 0 and modify
					uint32_t indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0);
					//Select VLAN table, set operation mode to READ, and set 2 bits from MSB of indirect_reg_addr in bits 0 and 1 of this register
					indirect_access_data |= (INDIRECT_TABLESELECT_VLAN << INDIRECT_CONTROL_TABLESELECT) | (INDIRECT_READTYPE_READ << INDIRECT_CONTROL_READTYPEBIT) | (((indirect_reg_addr >> 8) & 0xFF) << INDIRECT_CONTROL_ADDRESS_HIGH);
					//Write the modified register to the Ethernet Controller
					if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0, indirect_access_data))
					{
						return false;
					}
					//Obtain current indirect access data 1 and modify
					indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_1);
					//Set this register to the remaining 8 bits of the 10-bit indirect_reg_addr
					indirect_access_data = (indirect_reg_addr & 0xFF);
					if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1, indirect_access_data))
					{
						return false;
					}

					//VLAN SETTINGS MODIFICATION START HERE
					/*Each indirect register contains informatin for four VLANs organized in the following way:
					* ENTRY 0 (12 bits) maps to first VLAN, indirect bits (12-0): register 119 (bits 4-0), register 120 (bits 7-0)
					* ENTRY 1 (12 bits) maps to second VLAN, indirect bits (25-13): register 117 (bits 1-0), register 118 (bits 7-0), register 119 (bits 7-5)
					* ENTRY 2 (12 bits) maps to third VLAN, indirect bits (38-26): register 116 (bits 6-0), register 117 (7-2)
					* ENTRY 3 (12 bits) maps to fourth VLAN, indirect bits (51-39): register 114 (bits 3-0), register 115 (7-0), register 116 (bit 7);
					*
					* VLAN GROUP = (VLAN_ID / 4)
					* VLAN = VLAN GROUP % 4
					*/
					switch ((reg+1) % 4) {
					//User requested a change to the first VLAN in GROUP X
					case 0:
						//Get the information held in register 119
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_1);
						//Set VLAN entry to VALID
						indirect_reg_data |= (1 << 4);

						//Read current port membership data
						port_membership = ((vlan_data & 0x7F) >> 2);

						//Place first four bits of port_membership value into bits [3-0] of indirect register 119
						indirect_reg_data |= ((port_membership >> 1 ) << 0);
						//Overwrite register 119 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_1, indirect_reg_data))
						{
							return false;
						}
						//Get the information held in register 120
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_0);
						//Place the fifth bit into bit 7 of indirect register 120
						indirect_reg_data |= ((port_membership & 0x01) << 7);
						//Overwrite register 120 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_0, indirect_reg_data))
						{
							return false;
						}
						break;
					//User requested a change to the second VLAN in GROUP X
					case 1:
						//Get the information held in register 117
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_3);
						//Set VLAN entry to VALID
						indirect_reg_data |= (1 << 1);

						//Read current port membership data
						port_membership = ((vlan_data & 0x7F) >> 2);

						//Place first bit of port_membership value into bit [0] of indirect register 117
						indirect_reg_data |= ((port_membership >> 4 ) << 0);
						//Overwrite register 117 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_3, indirect_reg_data))
						{
							return false;
						}
						//Get the information held in register 118
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_2);
						//Place the last four bits of port_membership value into bits [7-4] of indirect register 118
						indirect_reg_data |= ((port_membership & 0xF) << 4);
						//Overwrite register 118 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_2, indirect_reg_data))
						{
							return false;
						}
						break;
					//User requested a change to the third VLAN in GROUP X
					case 2:
						//Get the information held in register 116
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_4);
						//Set VLAN entry to VALID
						indirect_reg_data |= (1 << 6);

						//Read current port membership data
						port_membership = ((vlan_data & 0x7F) >> 2);

						//Place port_membership value into bits [5-1] of indirect register 116
						indirect_reg_data |= ((port_membership) << 1);
						//Overwrite register 116 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_4, indirect_reg_data))
						{
							return false;
						}
						break;
					//User requested a change to the fourth VLAN in GROUP X
					case 3:
						//Get the information held in register 114
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_6);
						//Set VLAN entry to VALID
						indirect_reg_data |= (1 << 3);

						//Read current port membership data
						port_membership = ((vlan_data & 0x7F) >> 2);

						//Place first three bits of port_membership value into bits [2-0] of indirect register 114
						indirect_reg_data |= ((port_membership >> 2 ) << 0);
						//Overwrite register 114 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_6, indirect_reg_data))
						{
							return false;
						}
						//Get the information held in register 115
						indirect_reg_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_REGISTER_DATA_5);
						//Place the last two bits of port_membership value into bits [7-6] of indirect register 115
						indirect_reg_data |= ((port_membership & 0x3) << 6);
						//Overwrite register 115 with updated information
						if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_REGISTER_DATA_5, indirect_reg_data))
						{
							return false;
						}
						break;
					}

					//Obtain current indirect access data 0 and modify
					indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0);
					//Select VLAN table, set operation mode to READ, and set 2 bits from MSB of indirect_reg_addr in bits 0 and 1 of this register
					indirect_access_data &= ~0x1f;
					indirect_access_data |= (INDIRECT_TABLESELECT_VLAN << INDIRECT_CONTROL_TABLESELECT) | (INDIRECT_READTYPE_WRITE << INDIRECT_CONTROL_READTYPEBIT) | (((indirect_reg_addr >> 8) & 0xFF) << INDIRECT_CONTROL_ADDRESS_HIGH);
					//Write the modified register to the Ethernet Controller
					if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_0, indirect_access_data))
					{
						return false;
					}
					//Obtain current indirect access data 1 and modify
					indirect_access_data = EthoControllerSingleRead(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, INDIRECT_ACCESS_CONTROL_1);
					//Set this register to the remaining 8 bits of the 10-bit indirect_reg_addr
					indirect_access_data = (indirect_reg_addr & 0xFF);
					if (!EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN,INDIRECT_ACCESS_CONTROL_1, indirect_access_data))
					{
						return false;
					}

				}
				UpdateProgressBar(&progress, Increment, (100*(reg))/4095);
				delayMs(3);
			}
			UARTprintf("\n");

		}
		if ((FirmwareSettings & 0x10) == 0x10) {
			UARTprintf("\n[BOOTING]: Loading User Database...please wait\n");
			progress = CreateProgressBar();
			//Load Users Into Memory, Read in 15 users
			for (users_cnt = 0; users_cnt < MAX_USERS; users_cnt++) {
				//Get username from EEPROM (16 characters [16 registers])
				EEPROMBulkRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (users_cnt*65)),users[users_cnt].username, 16);
				//Get password from EEPROM (16 characters [16 registers])
				EEPROMBulkRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (users_cnt*65) + 16),users[users_cnt].password, 16);
				//Get first name from EEPROM (16 characters [16 registers])
				EEPROMBulkRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (users_cnt*65) + 32),users[users_cnt].first_name, 16);
				//Get last name from EEPROM (16 characters [16 registers])
				EEPROMBulkRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (users_cnt*65) + 48),users[users_cnt].last_name, 16);
				//Get permission level from user. PermLevel is cast from 8-bit unsigned integer
				users[users_cnt].permissions = (PermLevel)EEPROMSingleRead(EEPROM_BASE_ADDR, EEPROM_SSI_CS_BASE, EEPROM_SSI_CS_PIN,(EEPROM_USERS_BASE + (users_cnt*65) + 64));
				//Set this user to be left unchanged on next configuration save.
				users[users_cnt].nextAction = None;

				UpdateProgressBar(&progress, Increment, (100*(users_cnt))/MAX_USERS);
			}
			UARTprintf("\n");
		}
	}
	return true;
}





//*****************************************************************************
//
//! MAIN ENTRY POINT FOR FIRMWARE
//! Sets up device hardware, creates system objects, creates all FreeRTOS
//! tasks (if enabled), and sets up the data terminal ready pin for console
//! communications. After all initialization, this function starts the scheduler.
//!
//! \return Returns void
//
//*****************************************************************************
int
main(void)
{
	//*************************************************
	//
    // Set the clocking to run at 50 MHz from the PLL.
	//
	//*************************************************
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_25MHZ |
                       SYSCTL_OSC_MAIN);

	//*************************************************
	//
    // Initialize the UART, configure it for 115,200,
	// 8-N-1 operation, configure the SSI ports, and
	// configure our I2C port
	//
	//*************************************************
    ConfigureUART();
    ConfigureSSI();
    ConfigureI2C();
#ifndef DEBUG
    //ConfigureWatchdog();
#endif

    //*************************************************
	//
    // Clear the RX and TX buffers if any junk is
    // inside them. Since we send an escape code to
    // clear the console, this information will not be
    // seen by the user.
	//
	//*************************************************
    UARTFlushRx();
    UARTFlushTx(true);
    UARTprintf("\033[0m");
	//*************************************************
	//
    // Create mutexes to guard the UART, SPI0, SPI1,
    // and I2C ports.
	//
	//*************************************************
    g_pUARTSemaphore = xSemaphoreCreateMutex();
    g_pSPI0Semaphore = xSemaphoreCreateMutex();
    g_pSPI1Semaphore = xSemaphoreCreateMutex();
    g_pI2CSemaphore = xSemaphoreCreateMutex();

	//*************************************************
	//
	// Load saved configuration from EEPROM.
    // Starts reading from memory address 0x100
	//
	//*************************************************
    UARTEchoSet(false);
    InitializeEEPROM();
    UARTEchoSet(true);
	//*************************************************
	//
	// Set the register 0x01 in Ethernet Controller 1
    // to 0x01 thereby starting the IC with the current
    // configuration.
	//
	//*************************************************
    EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x01,0x01);
   	UARTprintf("[BOOTING]: Started Ethernet Controller\n");

   	//Setup Ethernet Controller to handle additional cascaded layers
   	//Enable Micrel Auto MDI/MDI-X mode
   	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x59, 0x00);
   	//Enable unicast packet forwarding
   	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x83, 0xB0);
   	//Enable multicast packet forwarding
   	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x84, 0x70);
   	//Enable IP multicast packet forwarding
   	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x86, 0x30);
   	//Enable rapid aging
//   	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x03, 0x06);
   	//Enable rapid aging based on port state
//   	EthoControllerSingleWrite(ETHO_1_BASE_ADDR, ETHO_1_SSI_CS_BASE, ETHO_1_SSI_CS_PIN, 0x02, 0x01);
   	UARTprintf("[BOOTING]: Configured Port 5 for expansion\n");



	//*************************************************
	//
	// Create all FREERTOS Tasks and Associated Queues.
   	// Tasks can be independently disabled through the
   	// the #define statements in this file's associated
   	// header.
	//
	//*************************************************
	#if ENABLE_LED_MANAGER
			if(LEDManagerTaskInit() != 0)
			{
				while(1)
				{
					//This should never happen! Wait for someone to help us!
				}
			}
	#endif
	#if ENABLE_INTERPRETER
			if(InterpreterTaskInit() != 0)
			{
				while(1)
				{
					//
					//This should never happen! Wait for someone to help us!
					//
				}
			}
	#endif
	#if ENABLE_I2C_MANAGER
			if(I2CManagerTaskInit() != 0)
			{
				while(1)
				{
					//
					//This should never happen! Wait for someone to help us!
					//
				}
			}
	#endif
	#if ENABLE_LOGGER
			if(LoggerTaskInit() != 0)
			{
				while(1)
				{
					//
					//This should never happen! Wait for someone to help us!
					//
				}
			}
	#endif
	#if ENABLE_PORT_MONITOR
			if(PortManagerTaskInit() != 0)
			{
				while(1)
				{
					//
					//This should never happen! Wait for someone to help us!
					//
				}
			}
	#endif


	//****************************************************************
	//
	// Setup the Data Terminal Ready (DTR) pin and interrupt
	// NOTE: This must be done AFTER the all FreeRTOS queues
	// and tasks have been created since an xQueueSendFromISR
	// statement appears in interrupts related to this particular pin
	//
	//****************************************************************
    GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6);
    // Use internal pull-ups to prevent spurious triggering on startup
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // Attach our interrupt to this pin's state
    GPIOIntRegister(GPIO_PORTD_BASE, DTSTriggered);
    // We only care when the pin transitions active low. Checking for a constant active
    // low signal can result in continuous triggering of our interrupt!
    GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_FALLING_EDGE);
    // Enable the interrupt
    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_6);


	//****************************************************************
	//
	// Check to see fi the console window on the client's machine
    // has already been opened before this system intialized. In this
    // scenario, we want them to see updated information.
	//
	//****************************************************************
	if (GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_6) != GPIO_PIN_6) {
		// Create an LEDProps instance which feeds LED state information to our
		// LEDManager RTOS task. All properties of the LEDProps class must be set.

		LEDProps DTSPin;
		DTSPin.LEDID = CONSOLE_OPEN_LED;
		DTSPin.interval = CONSOLE_OPEN_LED_BLINKRATE;
		DTSPin.ClearLED = false;

		#if ENABLE_LED_MANAGER
		if(xQueueSend(g_pLEDQueue, &DTSPin, portMAX_DELAY) != pdPASS)
		{
			//
			// Error. The queue should never be full. If so print the
			// error message on UART and wait for ever.
			//
			UARTprintf("\nQueue full. This should never happen.\n");
			while(1)
			{
			}
		}
		#endif
		// Since our LED is already going to be ON, we want to detach the current interrupt and replace it.
		GPIOIntDisable(GPIO_PORTD_BASE, GPIO_PIN_6);
		// Remove this function from the ISR table as we will be replacing it with an ISR that deals with console disconnect events
		GPIOIntUnregister(GPIO_PORTD_BASE);
		// Register the function that tells the microcontroller a console port has been disconnected (DTR is off)
		GPIOIntRegister(GPIO_PORTD_BASE, DTSOFF);
		// Set our new interrupt to be triggered by a DTR transition from low to high.
		GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_RISING_EDGE);
		GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_6);

	}

	//****************************************************************
	//
	// Start the FreeRTOS scheduler, this statement should NOT return.
	//
	//****************************************************************
    vTaskStartScheduler();

    // In case the scheduler returns for some reason, print an error and loop
    // forever.
	UARTprintf("\n RTOS ERROR: Scheduler stopped. System resetting in 3 seconds\n");
    while(1)
    {

    }
}


