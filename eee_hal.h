/**\file eee_hal.h
 * \brief <b>Function Prototypes for Hardware Abstraction Layer</b>
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

#ifndef EEE_HAL_H_
#define EEE_HAL_H_

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
//! Pauses execution of the microcontroller for the time specified in ui32Ms in
//! milliseconds by converting ui32Ms to MCU ticks. This function blocks all
//! other running tasks for the time specified. [USE SPARINGLY].
//!
//! \param ui32Ms the number of milliseconds to pause the MCU
//
//*****************************************************************************
void delayMs(uint32_t ui32Ms);
//*****************************************************************************
//
//! Pauses execution of the microcontroller for the time specified in ui32Us in
//! microseconds by converting ui32Us to MCU ticks. This function blocks all
//! other running tasks for the time specified. [USE SPARINGLY].
//!
//! \param ui32Us the number of microseconds to pause the MCU
//
//*****************************************************************************
void delayUs(uint32_t ui32Us);
//*****************************************************************************
//
//! Writes a single 8 bit value to a register within the EEPROM at the specified
//! address. This function does not handle page operations.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 17-bit address in EEPROM to write to
//! \param data the 8-bit data to save to EEPROM
//!
//! This function takes the address provided by the user and accompanying data
//! and performs the appropriate timing to not only store the information, but
//! also read back the information to ensure that the value has been properly
//! saved.
//!
//! \note Only the last 24 bits of information in the address variable are kept
//! for use with this chip. Of those 24, only 17 can be used to reference a
//! memory address as this chip only has a 131,072 sectors. Each sector stores
//! a single byte of information.
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMSingleWrite(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t address, uint8_t data);
//*****************************************************************************
//
//! Reads a single 8 bit value to a register within the EEPROM at the specified
//! address. This function does not handle page operations.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 17-bit address in EEPROM to write to
//!
//! This function obtains the 8 bits held at the address specified by the user
//! from the 25AA1024 EEPROM.
//!
//! \note Only the last 24 bits of information in the address variable are kept
//! for use with this chip. Of those 24, only 17 can be used to reference a
//! memory address as this chip only has a 131,072 sectors. Each sector stores
//! a single byte of information.
//!
//! \return Returns the value read from EEPROM as a unsigned integer
//
//*****************************************************************************
uint8_t EEPROMSingleRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t address);
//*****************************************************************************
//
//! Writes an array of 8-bit values to the EEPROM chip and handles overflow
//! to another page.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param start_address the 17-bit starting address in EEPROM to write to
//! \param data pointer-to-array of 8-bit values to write sequentially
//! \param array_length number of values held in the array. This value should be non-zero.
//!
//! This function takes the address provided by the user and accompanying data
//! and performs write operations for each 8-bit value in the given array. If
//! the next address were to overflow to another page, this function provides
//! safe methods for dealing with this.
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMBulkWrite(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t start_address, uint8_t *data, uint32_t array_length);
//*****************************************************************************
//
//! Reads an array of 8-bit values to the EEPROM chip and handles overflow
//! to another page.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param start_address the 17-bit starting address in EEPROM to write to
//! \param output pointer-to-array of 8-bit values to save results into
//! \param array_length number of values held in the array. This value should be non-zero.
//!
//! This function takes the address provided by the user and accompanying data
//! and performs read operations for each address up to the array size. If
//! the next address were to overflow to another page, this function provides
//! safe methods for dealing with this.
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMBulkRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t start_address, uint8_t *output, uint32_t array_length);
//*****************************************************************************
//
//! Reads a status register from the Ethernet Controller over SPI.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 8-bit address in the Ethernet Controller to read from
//!
//! This function takes the address provided by the user and performs the timing
//! needed to aquire the status information inside the specified register on a
//! MICREL KSZ8895MQX Ethernet Controller.
//!
//! \return Returns the data held at the specified register as an unsigned integer
//
//*****************************************************************************
uint32_t EthoControllerSingleRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint8_t address);
//*****************************************************************************
//
//! Reads multiple status register from the Ethernet Controller over SPI.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param start_address the 8-bit address in the Ethernet Controller to begin reading from
//! \param count the number of registers from the starting address to read.
//! \param output pointer-to-array value that feeds the result of the function back to the calling thread
//!
//! This function takes the address provided by the user and performs the timing
//! needed to aquire the status information inside the specified register on a
//! MICREL KSZ8895MQX Ethernet Controller. It then proceeds to get all successive registers
//! from the device until the count parameter has been reached. This information is then returned
//! as a pointer-to-array of uint32_t values.
//!
//! \note Ensure that the array passed to the "output" parameter is defined as having at least the size of count
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Success)
//
//*****************************************************************************
bool EthoControllerBulkRead(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint8_t start_address, uint8_t count, uint32_t *output);
//*****************************************************************************
//
//! Writes a status register from the Ethernet Controller over SPI.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the 8-bit address in the Ethernet Controller to read from
//! \param data the 8-bit data to save to the Ethernet Controller
//!
//! This function takes the address provided by the user and performs the timing
//! needed to write the status information inside the specified register on a
//! MICREL KSZ8895MQX Ethernet Controller.
//!
//! \return Returns the data held at the specified register as an unsigned integer
//
//*****************************************************************************
bool EthoControllerSingleWrite(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint8_t address, uint32_t data);
//*****************************************************************************
//
//! Erases the 25AA1024 memory area completely.
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMChipErase(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN);
//*****************************************************************************
//
//! Erases a page on the 25AA1024 (256 bytes)
//!
//! \param SSI_BASE the base address of the SSI port connected to the EEPROM
//! \param CS_PORT_BASE the base address of the port that the CS GPIO pin is on
//! \param CS_PIN the pin of the Chip Select (CS) on the port specified above
//! \param address the page [(0x00 - 0xFFFFF) % 256] to erase. Pages are 256 bytes!
//!
//!
//! \return Returns the result of the operation (0 = Failed, 1 = Succeeded)
//
//*****************************************************************************
bool EEPROMPageErase(uint32_t SSI_BASE, uint32_t CS_PORT_BASE, uint32_t CS_PIN, uint32_t address);



#endif /* EEE_HAL_H_ */
