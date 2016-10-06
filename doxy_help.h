/******************************************************************************************************************************************
 *  doxy_help.h
 *  Created on: May 20, 2016
 *      Author: Christopher R. Miller
 *      Company: Eagle Embedded Engineering
 *      Copyright (c) 2016
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
#ifndef DOXY_HELP_H_
#define DOXY_HELP_H_

/**
 * @mainpage EEE MISL Switch Firmware
 *
 * @section intro_sec Introduction
 * This software is designed to allow full customization of the MISL 100 Base TX Ethernet switch layer which includes the Texas Instruments
 * TM4C123GH6PM ARM Cortex M4F Tiva C Series Microcontroller, Microchip AA1024 EEPROM, and KSZ8895MLUB ethernet controller. For a brief overview
 * of the project, click <a href="http://esetwiki.net/index.php?title=Eagle_Embedded_Engineering">here</a>. This project was funded by the
 * Mobile Integrated Solutions Laboratory and the National Aeronautics and Space Administration.
 * @section board_specs Board Specifications
 * <p><b>Intelligence:</b> TI TM4C123GH6PM ARM Cortex M4F 80MHZ Microcontroller / Micrel KSZ8895MLUB Ethernet Controller</p>
 * <p><b>Connectivity:</b> USB/I2C Configuration, 4x RJ-45 Connections</p>
 * <p><b>Throughput:</b> 63 Mbps (100 Mbps MAX)</p>
 * <p><b>FLASH:</b> 256 KB (52 kB used by firmware) | <b>EEPROM:</b> 125 KB (AA1024) | <b>SRAM:</b> 32 kB (24 kB used by firmware)</p>
 *
 *
 * @section help_breakdown Documentation Breakdown
 * @subsection com_func_main FreeRTOS Integration
 * In order to provide a non-blocking solution for this new MISL layer, the project implements several tasks under the FreeRTOS real-time
 * operating system. Each task serves a dedicated role and blocks all access to peripherals while they are in use throgh the use of mutexes.
 * Tasks are moved from created to ready by placing objects into allocated queues. For the most part, all tasks in this firmware rely on
 * interrupt-driven queues. The interpreter task, as an example pends on the user placing a newline character in the UART RX buffer before
 * attempting to decipher the command issued. Once an entered string has been validated as a command, the appropriate function within the firmware
 * is executed. Since an RTOS is used, all long-running tasks are designed to pause briefly in order to allow other tasks to report back to the kernel.
 * If at any time a task is blocked from execution for more than two seconds, the Tiva C has been preprogrammed with an active watchdog timer that will
 * reset the system.
 * During boot, all saved configuration settings from EEPROM are loaded into local memory. Since this is a long running operation, status is continually
 * reported to the user's terminal window (if opened).
 * @subsection command_struc_main Main Files
 * #### Command Functions
 * This file contains all of the functions pointed to by commands in the "interpreter_task.h" file. Each function takes a fixed number of parameters from
 * a char-to-pointer array. Since the primary function of this firmware is to control the state and settings of each port, most functions included in this file
 * rely on the developer passing the base address of the target port as parameter one (param[0]). For additional information regarding the proper use of each command
 * outside, refer to command_functions.h.
 * #### EEE Device Programming Interface
 * In order to abstract away the commmunication process between devices on this board, the eee_api.c and eee_api.h files are included with simplified functions for writing and
 * reading to both the Microchip EEPROM and Micrel Ethernet Controller. Each of these functions requires the developer to specify the SSI base address, chip select port base address,
 * and chip select pin. All write functions verify the operation by comparing the value expected to the final value of the register specified.
 * @subsection eee_api_main EEE Board/Application Interface
*/
