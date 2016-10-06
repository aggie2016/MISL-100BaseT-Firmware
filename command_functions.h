/**\file command_functions.h
 * \brief <b>CLI function prototypes. New CLI functions should have a prototype placed here as well.</b>
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

#ifndef COMMAND_FUNCTIONS_H_
#define COMMAND_FUNCTIONS_H_

//***********************************************
// Flags for use with FreeRTOS vTaskDelayUntil
//***********************************************
//!	Delay flag used for tasks that need more than 40 milliseconds to execute
#define LONG_RUNNING_TASK_DLY 40
//!	Delay flag used for tasks that need more than approximately 10 milliseconds to execute
#define SHORT_TASK_DLY 10
//!	Delay flag used for tasks that need more than approximately 5 milliseconds to execute. Also recommended for looping/querying functions
#define VERY_SHORT_TASK_DLY 5

//!	Maximum number of logical ports on this PCB
#define PORT_COUNT 4

//!	Flag used by I2C to indicate that no communication back from the slave was received or should be received.
#define I2C_NOREPLY -1

//***********************************************
// System Setttings Flags
//***********************************************
//!	Base EEPROM Address of System Settings Flags
#define FLAG_BASE 0x1E
//!	Bit number for EEPROM reinitialization request
#define FLAG_EEPROM_REINIT_REQUESTED 	7
//!	Bit number for valid configuration saved flag
#define FLAG_CONFIG_SAVED 				6
//!	Bit number for valid VLAN table is saved flag
#define FLAG_CONFIG_VLAN_VALID 			5
//!	Bit number for valid user table is saved flag
#define FLAG_CONFIG_USERS_VALID			4

//!	Actions that can be passed to UpdateProgressBar to change the action that is performed
typedef enum ProgressBarActions {
	//!	Increase state of progress bar by 1%
	Increment,
	//!	Decrease state of progress bar by 1%
	Decrement,
	//!	Set progress bar back to 0%
	Reset,
	//!	Set progress bar to 100%
	Fill,
	//!	Fill progress bar with exclamation marks up to 100%
	FillError
}PBarActions;

typedef struct {
	uint16_t VLAN_ID;
	uint8_t PORT_REGISTRATION;
	bool isActive;
} VLANTableEntry;

//*****************************************************************************
//
//! Write to EEEPROM (for Console Window)
//! Writes an 8-bit value to the register address specified and reports on the
//! success or failure of the operation to the user's command line interface.
//! \param params[0] 8-bit register address to write to
//! \param params[1] 8-bit register data to write
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_WriteToEEPOM(char *params[20]);
//*****************************************************************************
//
//! Save Running Configuration to EEPROM (for Command-Line Interface)
//! Saves all registers from 0x00 to 0xFF inside the ethernet controller to
//! the corresponding EEPROM registers (0x100 - 0x1FF) an reports on the
//! success or failure of the operation.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SaveSwitchConfiguration(char *params[20]);
//*****************************************************************************
//
//! Reinitialize EEPROM (for Command-Line Interface)
//! Sets the flag in register 0x1E (bit 7) that controls whether or not the EEPROM
//! will be erased during reboot. This function will return successfully but
//! the effect of the command will not be seen until after the system has been
//! restarted.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ReinitializeEEPROM(char *params[20]);
//*****************************************************************************
//
//! Read from EEEPROM (for Console Window)
//! Reads an 8-bit value from the register address specified and reports on the
//! success or failure of the operation to the user's command line interface.
//! \param params[0] 8-bit register address to read from
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ReadFromEEPROM(char *params[20]);
//*****************************************************************************
//
//! Read 8-bits From Ethernet Controller (for Command-Line Interface)
//! Aquires the 8-bit value held at the register address specified and returns
//! it to the user's command-line. This function is mainly used for diagnostics
//! purposes.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \param params[0] 8-bit register address inside the KSZ8895MLUB
//!
//! \return Success or failure of the operation as a boolean.
//
//*****************************************************************************
bool COM_ReadFromEthernetController(char *params[20]);
//*****************************************************************************
//
//! Write 8-bit Value To Ethernet Controller (for Command-Line Interface)
//! Writes specified value to the register address specified and
//! reports on the success or failure of the operation to the user's command-line.
//!
//! \param params[0] 8-bit register address for KSZ8895MLUB
//! \param params[1] 8-bit data to write
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_WriteToEthernetController(char *params[20]);
//*****************************************************************************
//
//! Sets A Bit Inside Ethernet Controller (for Command-Line Interface)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the user's command-line.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active high
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetBitEthernetController(char *params[20]);
//*****************************************************************************
//
//! Clear A Bit Inside Ethernet Controller (for Command-Line Interface)
//! Writes an active low value to the register address and bit specified and
//! reports on the success or failure of the operation to the user's command-line.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active low
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ClearBitEthernetController(char *params[20]);
//*****************************************************************************
//
//! Set Self-Clearing Bit Inside Ethernet Controller (for Command-Line Interface)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the user's command-line
//! once the specified bit has returned to its original active low state
//!
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to pend on
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetSCBitEthernetController(char *params[20]);
//*****************************************************************************
//
//! Run Cable Diagnostics For Specified Port (for Command-Line Interface)
//! Activates the Time Domain Reflectometry (TDR) function built into the ethernet
//! controller for the port specified and writes the state (Short/Open/Normal) and
//! distance to fault (in meters) back to the user's command-line
//!
//! \param params[0] 8-bit base address of the port to test (see interpreter_task.h)
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_RunCableDiagnostics(char *params[20]);
//*****************************************************************************
//
//! Show Current Port Status (for Command-Line Interface)
//! Reads the registers specified by both "PortConfigMappings" (refer to "interpreter_task.h")
//! and reports the current value of each in a textual format. Aligns all printed
//!	text printed to command-line to increase legibility.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowPortStatus(char *params[20]);
//*****************************************************************************
//
//! Perform Soft Reset (for Command-Line Interface)
//! Issues a manual software reset of the Tiva C from the command-line. Requires
//! the user to enter the command twice before resetting.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ResetTivaC(char *params[20]);
//*****************************************************************************
//
//! Show Global Switch Status (for Command-Line Interface)
//! Reads the registers specified by both "PortConfigMappings"
//! and "GlobalConfigMappings" (refer to "interpreter_task.h")
//! and reports the current value of each in a textual format.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowRunningConfig(char *params[20]);
//*****************************************************************************
//
//! Delete Users Menu (for Command-Line Interface)
//! Provides the CLI with an easy-to-navigate selectable list of all current
//! users and allows selection of those to delete. In order to save any of the
//! checked users, the configuration must be saved to the EEPROM.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DeleteUsersMenu(char *params[20]);
//*****************************************************************************
//
//! List Currently Entered Users (for Command-Line Interface)
//! Goes through the "users" array and displays all current values to the CLI.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ListUsers(char *params[20]);
//*****************************************************************************
//
//! Add New User to System (for Command-Line Interface)
//! Issues prompts to the command-line to gather information for a new user. Once
//! all fields in the User_Data structure are filled, this function places the new
//! user at the next available slot in the "users" array. In order to prevent this
//! function from blocking the FreeRTOS tasks currently running, vTaskDelayUntil
//! is used while waiting for a new line character from the command-line.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_AddUser(char *params[20]);
//*****************************************************************************
//
//! Display Status of EEPROM Events (for Command-Line Interface)
//! Writes the current state (enabled/disabled) of all configured EEPROM events
//! to the CLI.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_EventStatus(char *params[20]);
//*****************************************************************************
//
//! Manage EEPROM Logged Events (for Command-Line Interface)
//! Displays a menu of checkboxes to allow for easy enabling/disabling of configured
//! system events to log to the EEPROM. The results of this menu will only be saved
//! when the user issues a configuration save command. This prevents the changes from
//! being lost on a reboot.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ManageEvents(char *params[20]);
//*****************************************************************************
//
//! List All EEPROM Logged Events (for Command-Line Interface)
//! Displays a dump of all events logged to the EEPROM. Since the current
//! iteration of this system does not have a way of saving or retriving a real-time
//! clock, the metric reported before the logged event is the number of FreeRTOS
//! ticks since the system was started. Maximum number of log entries that can be
//! stored/read is 400.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ListEvents(char *params[20]);
//*****************************************************************************
//
//! Delete All EEPROM Logged Events (for Command-Line Interface)
//! Erases all EEPROM Sectors that are allocated for log entries and resets
//! next log slot to the initial EEPROM_LOG_BASE value.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DeleteEvents(char *params[20]);
//*****************************************************************************
//
//! Logs Out From Switch CLI (for Command-Line Interface)
//! Logs the current user out of the command line and returns to a login prompt.
//! Also logs the UserLoggedOut event to EEPROM (if enabled).
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_Logout(char *params[20]);
//*****************************************************************************
//
//! Set Port Default VLAN Tag (for Command-Line Interface)
//! Writes to the appropriate registers on the KSZ8895MLUB to change the selected
//! port to the VLAN specified. Also turns on automatic tag insertion for all
//! frames that do not already have a VLAN specified (IMPORTANT!).
//!
//! \param params[0] 8-bit base port address (refer to "interpreter_task.h)
//! \param params[1] 12-bit VLAN ID to set this port to (MAX VLAN ID: 4096)
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetPortVLAN(char *params[20]);
//*****************************************************************************
//
//! Enable 802.1q VLAN Tagging (for Command-Line Interface)
//! Enables VLAN filtering for all active ports.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_EnableVLANS(char *params[20]);
//*****************************************************************************
//
//! Disable 802.1q VLAN Tagging (for Command-Line Interface)
//! Disables VLAN filtering for all active ports.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DisableVLANS(char *params[20]);
//*****************************************************************************
//
//! Delete Configuration (for Command-Line Interface)
//! Sets the flag for loading configuration and VLANs in EEPROM register 0x1E
//! back to a logic low level. When rebooting the switch, the configuration and
//! VLAN values will not be loaded.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_DeleteConfig(char *params[20]);
//*****************************************************************************
//
//! Append VLAN to VLAN Table (for Command-Line Interface)
//! Writes the specified VLAN entry to the KSZ8895MLUB's indirectly accessible
//! VLAN table. Also scans adjacent ports for matching VLAN tags to update VLAN
//! table with new membership information ("AssertVLANS()").
//!
//! \param params[0] 8-bit base port address (refer to "interpreter_task.h)
//! \param params[1] 12-bit VLAN ID to add to VLAN Table (MAX VLAN ID: 4096)
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_SetVLANEntry(char *params[20]);
//*****************************************************************************
//
//! Display VLAN Table (for Command-Line Interface)
//! Writes all active VLAN Table entries to the Command-Line Interface. This
//! function is a long-running task and takes approximately 20 seconds to fully
//! compile the VLAN table.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowVLANTable(char *params[20]);
//*****************************************************************************
//
//! Show Current Static MAC Table (for Command-Line Interface)
//! Displays all valid entries in the Micrel KSZ8895MLUB Ethernet Controller's
//! static MAC table. In order to obtains these values, the function accessess
//! the indirect registers in the KSZ8895MLUB. Information regarding these
//! registers can be found in the datasheet for this part.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowStaticMACTable(char *params[20]);
//*****************************************************************************
//
//! Show Current Dynamic MAC Table (for Command-Line Interface)
//! Displays all valid entries in the Micrel KSZ8895MLUB Ethernet Controller's
//! dynamic MAC table. In order to obtains these values, the function accessess
//! the indirect registers in the KSZ8895MLUB. Information regarding these
//! registers can be found in the datasheet for this part.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_ShowDynamicMACTable(char *params[20]);
//*****************************************************************************
//
//! Send an I2C Command (for Command-Line Interface)
//! Allows the user to modify other layers using the I2C interface. To do this,
//! refer to "i2c_task.h" for valid I2C commands and how to send parameters. This
//! command will only send additional parameters up to that defined in MAX_PARAMS.
//! Once a valid command has been sent, the command line will inform the user what
//! the result of the operation was (1: success, 0: failed).
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
bool COM_I2CSend(char *params[20]);
//*****************************************************************************
//
//! Write/Read from Ethernet Controller (for I2C Commands)
//! Reads or writes an 8-bit value from the register address specified and
//! reports on the success or failure of the operation to the user over I2C.
//! \param params[0] 8-bit register address to write/read from
//! \param params[1] 8-bit register data to write (only applicable in write mode)
//! \param params[2] 1-bit boolean value (1: write operation, 0: read operation)
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_WriteReadFromEthernetController(uint8_t params[20]);
//*****************************************************************************
//
//! Set A Bit Inside Ethernet Controller (for I2C Commands)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the I2C master.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active high
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_SetBitEthernetController(uint8_t params[20]);
//*****************************************************************************
//
//! Clears A Bit Inside Ethernet Controller (for I2C Commands)
//! Writes an active low value to the register address and bit specified and
//! reports on the success or failure of the operation to the I2C master.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active low
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_ClearBitEthernetController(uint8_t params[20]);
//*****************************************************************************
//
//! Set Self-Clearning Bit Inside Ethernet Controller (for I2C Commands)
//! Writes an active high value to the register address and bit specified and
//! reports on the success or failure of the operation to the I2C master after that
//! same bit returns to an active low state.
//! \param params[0] 8-bit base address of the port to change (see interpreter_task.h)
//! \param params[1] 8-bit register address to write to
//! \param params[2] 8-bit register bit to set active low
//!
//! \return Returns with the result of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_SetSCEthernetController(uint8_t params[20]);
//*****************************************************************************
//
//! Run Cable Diagnostics For Specified Port (for I2C Commands)
//! Activates the Time Domain Reflectometry (TDR) function built into the ethernet
//! controller for the port specified and writes the state (Short/Open/Normal) and
//! distance to fault (in meters) back to the I2C master.

//! \param params[0] 8-bit base address of the port to test (see interpreter_task.h)
//!
//! \return Returns cable state (Short/Open/Normal) for further analysis
//
//*****************************************************************************
uint8_t I2C_RunCableDiagnostics(uint8_t params[20]);
//*****************************************************************************
//
//! Save Running Configuration to EEPROM (for I2C Commands)
//! Saves all registers from 0x00 to 0xFF inside the ethernet controller to
//! the corresponding EEPROM registers (0x100 - 0x1FF) an reports on the
//! success or failure of the operation.
//!
//! \return Returns the results of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_SaveSwitchConfiguration(uint8_t params[20]);
//*****************************************************************************
//
//! Download Running Configuration (for I2C Commands)
//! Sequentially writes each value held in the Micrel KSZ8895MLUB's registers
//! 0x00 - 0xFF to the requesting I2C master.
//!
//! \return Returns the results of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_DownloadSwitchConfiguration(uint8_t params[20]);
//*****************************************************************************
//
//! Clear Running Configuration (for I2C Commands)
//! Sequentially writes a null (0x00) to each register in the Micrel 25AA1024
//! EEPROM from 0x100 - 0x1FF and reports the success or failure of the operation
//!
//! \return Returns the results of the operation as a boolean
//
//*****************************************************************************
uint8_t I2C_ClearSwitchConfiguration(uint8_t params[20]);
//*****************************************************************************
//
//! Update A Task Progress Bar (for Command-Line Interface)
//! Changes the current state of a progress bar by either incrementing, decrementing,
//! the current value. Once updated, the value of lastprogress is updated so that
//! the function can keep track of how much the progress bar has changed.
//
//*****************************************************************************
void UpdateProgressBar(int* lastprogress, PBarActions action, int newvalue);
//*****************************************************************************
//
//! Create A Task Progress Bar (for Command-Line Interface)
//! Prints the beginning of a progress bar to the CLI and returns an initilized
//! integer to the user. This value should be passed by reference to the
//! UpdateProgressBar function when a change to the progress bar is needed.
//!
//! Follows the function prototype required for all command-line function pointers
//! by returning the result of the operation as a boolean success or fail.
//!
//! \return Returns a value of zero to start empty progress bar.
//
//*****************************************************************************
int CreateProgressBar();

//*****************************************************************************
//
//! [DEPRECATED] Show Progress Bar (for Command-Line Interface)
//! Shows a simple progress bar for reporting task progress to the user.
//! [since v. 0.1.32] Replaced with CreateProgressBar and UpdateProgressBar to
//! remove unecessary writing to the CLI. Old progress bar erased the entire line
//! and redrew the progress bar (highly inefficient). The new functions only change
//! the part of the progress bar that is indicated by the developer.
//
//*****************************************************************************
void ShowProgress(int percent);

#endif /* COMMAND_FUNCTIONS_H_ */
