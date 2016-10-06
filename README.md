# MISL-100BaseT-Firmware
Firmware for the MISL 100BaseT switching layer. Provides the user with a CLI for serviceability and control. Built on the open-source FreeRTOS Real Time Operating System
                                                                                     
=====================================================================================================================================
Eagle Embedded Engineering
MISL 100BaseTX Switch Layer Firmware 
RELEASE v1.1.12
=====================================================================================================================================

==== KNOWN BUGS ====
- [1 - 9/22/2016] Cascading two 100BaseTX layers may require a flush of the dynamic MAC table in order to achieve connectivity between the two layers in question. As of yet, this appears to be a software bug but may have a hardware component (such as the capacitive coupling from the expansion port to P5 of the KSZ8895MLUB Ethernet controller). The recommmended fix for this bug is to simply connect/disconnect the cable until the dynamic MAC table, accessible through the command 'system show dyn-mac-table' shows entries for 'exp-port'. This should indicate that the system now recognizes the existence of other connected nodes on a cascaded layer.
- [2 - 9/10/2016] Use of arrow keys while typing results in PuTTY sending a carriage return (CR '\r') to the UART RX buffer and, by proxy, the UARTStdioIntHandler. Future iterations of this system may include the ability to access previously used commands through the use of the up and down arrows, however, a valid detection method that seperates this carriage return from the user pressing the <enter> key has not yet be developed.
- [3 - 9/22/2016] Upon reset, alignment of debug information may be slightly off due to the timing of characters received from the FTDI UART-to-USB converter. This will not cause any functional impairment for the user.
- [4 - 9/22/2016] If a command executes too quickly, the included progress bar routines may show a bar that is not entirely filled. In these cases, the function did in fact fully execute and the command-line should return a message saying so.

==== INTRODUCTION =====
An embedded real-time control package to allow observability and management of four 100BaseTX ports. This system utilizes a command-line
interface (CLI) that exposes numerous features of the on-board Ethernet Control for easy configuration. Each of the files included in this release is briefly described below in order to provide the reader with a high-level understanding of each module.

[1]==== COMMAND FUNCTIONS [command_functions] (.c/.h) ====
	This file contains all functions called at run-time by either CLI or I2C commands. All functions that are intended to be used by the command line adopt the format COM_<Function Name>, take an array of pointers-to-char, and return a boolean as the result of execution. This format must be followed in order to avoid incompatibility with	the UART interpreter task (described later in this document). Each function is described below
	
[1.1] -- Structure of New Commmands --
	If you wish provide additional functionality to the command line interface, all new functions must follow the syntax outlined below.
	Command Line Functions:
		Each function takes an array of pointer-to-char of size 20 (as defined by MAX_PARAMS) and returns a bool as the result of execution.
	
		bool COM_<FunctionName>(char *params[MAX_PARAMS])
		{
		    return true (or false);
		}
	
	I2C Functions:
		Unlike functions used by the CLI, I2C commands are formatted to accept and return 8-bit values for use over SCL and SDA lines. This allows the information recieved to the easily processed since all values recieved over I2C are done so using an 8-bit packet size. The commonly used return value for successful and unsuccesful commands is the same as that defined for boolean true and false (0x01: Successful, 0x00: Failed). Any additional information that you wish to return to the I2C master must be done so within the scope of the function itself.
		
		uint8_t I2C_<FunctionName>(uint8_t params[MAX_PARAMS])
		{
		    return true;		
		}


	[1.2] -- Playing Nicely with FreeRTOS --
	Since all of the executed code in this firmware (after boot) utitlizes FreeRTOS task, the use of long processing loops within the command-line
	is highly discouraged. Instead, inside long-running processes (i.e. COM_SaveSwitchConfiguration), use the built-in FreeRTOS xTaskGetTickCount and
	vTaskDelayUntil functions to briefly return control back to the kernel to allow other tasks to run. The firmware provides three levels of task delay
	length that can be used to specify the amount of computation time that will be returned to the kernel:
		- LONG_RUNNING_TASK_DLY (40 milliseconds)
		- SHORT_TASK_DLY	(10 milliseconds)
		- VERY_SHORT_TASK_DLY	(5  milliseconds)
	
	[1.3] -- Displaying Real-Time Progress --
	EEE Firmware v1.1.12 includes a brief set of functions that display a progress bar during the execution of a long-running task. To add a progress bar in
	lieu of standard notification text, use the CreateProgressBar function. This function will return a progress pointer that is used by the UpdateProgressBar
	functions. To increment the progress bar, pass the pointer along with the PBarAction parameter 'Increment'. No additional text should be printed to the command
	line while this bar is displaying progress.
	
	[1.4] -- Current CLI Functions --
	Commands currently supported by this version of the EEE 100BaseTX Firmware are listed below. For an expanded description of each function, refer directly
	to the source code or to the doxygen homepage stored under 'html/index.html'. 
		[1.4.1] COM_WriteToEEPROM
		[1.4.2]	COM_SaveSwitchConfiguration
		[1.4.3] COM_ReinitializeEEPROM
		[1.4.4] COM_ReadFromEEPROM
		[1.4.5] COM_ReadFromEthernetController
		[1.4.6] COM_WriteToEthernetController
		[1.4.7] COM_SetBitEthernetController
		[1.4.8] COM_ClearBitEthernetController
		[1.4.9] COM_SetSCBitEthernetController
		[1.4.10] COM_RunCableDiagnostics
		[1.4.11] COM_ShowPortStatus
		[1.4.12] COM_ResetTivaC
		[1.4.13] COM_ShowRunningConfig
		[1.4.14] COM_DeleteUsersMenu
		[1.4.15] COM_ListUsers
		[1.4.16] COM_AddUser
		[1.4.17] COM_EventStatus
		[1.4.18] COM_ManageEvents
		[1.4.19] COM_ListEvents
		[1.4.20] COM_DeleteEvents
		[1.4.21] COM_Logout
		[1.4.22] COM_SetPortVLAN
		[1.4.23] COM_EnableVLANS
		[1.4.24] COM_DisableVLANS
		[1.4.25] COM_DeleteConfig
		[1.4.26] COM_SetVLANEntry
		[1.4.27] COM_ShowVLANTable
		[1.4.28] COM_ShowStaticMACTable
		[1.4.29] COM_ShowDynamicMACTable
		[1.4.30] CreateProgressBar
		[1.4.31] UpdateProgressBar

[2]==== EEE HARDWARE ABSTRACTION LAYER [eee_hal] (.c/.h) ====
	These files contain functions that eliminate the need to directly manage the hardware on the embedded system. More specifically, the contain
	routines for controlling read/write/erase operations for the Microchip 25AA1024 EEPROM and the Micrel KSZ8895MLUB Ethernet Controller.
	
	[2.1] -- Included Functions --
	The following functions provide an easy interface with the hardware on the switch PCB. They are:
		[2.1.1] - EEPROMSingleWrite
		[2.1.2] - EEPROMSingleRead
		[2.1.3] - EEPROMBulkWrite
		[2.1.4] - EEPROMBulkRead
		[2.1.5] - EthoControllerSingleRead
		[2.1.6] - EthoControllerBulkRead
		[2.1.7] - EthoControllerSingleWrite
		[2.1.8] - EEPROMChipErase
		[2.1.9] - EEPROMPageErase
	
	[2.2] -- FreeRTOS Considerations --
	Since these functions are called quite often by numerous CLI and I2C commands, almost all of them use mutexes to block other tasks from
	utilzing the hardware until it becomes available again. This prevents interfering write (or read) operations.
	
	[2.3] -- Bulk Operations --
	In order to simplify the process of reading batches of information from both devices, Bulk read/write operations are included. Each of these
	functions takes a pointer to an destination array as well as the number of records to read. It is highly recommended that you allocate enough
	space inside your destination array to hold the desired number of records.

[3] === System Event Logger [event_logger] (.c/.h) ===
	The logger function runs as a FreeRTOS task to allow asynchronous storage of system notifications. Each entry is stamped with the current system
	time since this version of the firmware/hardware does not include a mechanism for determining and updating a real-time clock. Future versions of 
	this system that utilize an embedded TCP/IP stack are planned to communicate with a remote time server.
	
	[3.1] -- Format of Log Entry --
	Each stored log entry consists of the following format:
		[4 bytes] System Time Stamp
		[1 byte] Event Code
	A maximum of 400 entries are stored before the system begins overwriting old entries.
	
	[3.2] -- Currently Supported/Logged Events --
	A maximum of 32 events can be logged by the system. Currently, these 10 are used to notify an authorized user what has happened since his last login.
		[3.2.1] - SystemRestarted
		[3.2.2] - StackOverflow
		[3.2.3] - EEPROMWriteOP
		[3.2.4] - EEPROMReadOP
		[3.2.5] - EEPROMIOException
		[3.2.6] - EthoControllerReadOP
		[3.2.7] - EthoControllerWriteOP
		[3.2.8] - EthoControllerIOException
		[3.2.9] - UserLoggedIn
		[3.2.10] - UserLoggedOut

[4] === I2C Command Interpreter [i2c_task] (.c/.h) ===
	In order to allow other layers on the MISL stack to control and observe the operations of this layer, a subset of the commands in the CLI have been
	exposed over I2C. Each command has a unique hexadecimal code in addition to several other parameters including "static parameter count", "custom
	parameter count", "return value count", and a list of statically defined parameters that are passed to a specified function.

	[4.1] -- Setting the I2C Slave Address --
	The default slave address of each MISL switch layer is 0x1A. This can be changed directly in the "freertos_init.h" file. The firmware will then need
	to be reflashed to the layer before the changes take effect.
	
	[4.2] -- Loopback Control --
	For the purposes of demonstration, the layer can be controlled over I2C as a slave device. An optional loopback mode can be configured to allow direct
	control of the layer from the command line using I2C Codes. For further explanation of this, refer to i2c_task.hM/`
	
[5] === UART Interpreter Task [interpreter_task] (.c/.h) ====
	This switch layer uses an independent RTOS task that takes direct input from the UART0 interrupt and tokenizes the input. This information is then checked 
	word by word to see if a valid command string has been entered. If the currently checked word matches a branch of the linked list (that makes up the command-line), 
	the task saves all entered parameters and moves down the tree to the next valid word. This process is illustrated below:
	
	[5.1] -- Linked List Architecture --
		(1) COMMAND [parameters = 2, executable = false, sub-menu = OPTIONS]
			|- OPTIONS [parameters = 0, executable = false, sub-menu = SETTINGS]
				|- SETTINGS [parameters = 2, executable = true, function_pointer = Run()]
				
	[5.2] -- Commands Available --
	For a list of all available commands, refer to interpreter_task.h. 
	
	[5.3] -- Ethernet Controller Settings --
	In order to make this firmware easily portable to other Micrel Ethernet Controllers, the interpreter_task.h file contains definitions for each major set of registers. 
	When porting, ensure that each of these definitions matches the data sheet for your Ethernet controller. At the time of this firmware's creation, the ports were mapped
	inversely to the order of the Ethernet Controller (to conform with the PCB design). In essence, this means that port 4 was mapped to port 1, port 3 was mapped to port 2,
	port 2 was mapped to port 3, and port 1 was mapped to port 4. 
	In addition, a section of this header file contains register mappings to convert each of their expected values into strings that the user can understand. This provides an
	easy to understand status interface for each port as well as the entire system.
	
	[5.4] -- Expanding upon the Command Line --
	Should a need arise to add functionality to the command line interface, simply copy the structure format below and add it to the interpreter task.h file. Each filed in the structure
	array must be filled in to avoid runtime errors. Otherwise, to inform the interpreter of the new commmand's existence, simply link it to one of the existing parent commands by passing a
	reference as indicated below.
		STRUCTURE FORMAT:
			typedef struct Command {
				//Command string that will be entered by the user. Should be a SINGLE (possibly hyphenated) word.
				const char * const text;
				//Help text that will be displayed when the user appends a '?' after the currently entered command word
				const char * const help;
				//Designates this command as an executable (terminating) command. Ensure that functions labeled this way have function pointers attached to them
				const bool isExecutable;
				//Number of statically defined (in the command's definition) or custom (user entered parameters) 
				const int paramsRequired;
				//Designates this command as requiring custom input from the command line. The user will not enter the command text defined but their own input. 
				const bool paramsUserProvided;
				//A function to call when the command is issued. COMMAND MUST BE TERMINATING_COMMMAND TO CALL FUNCTION.
				bool (*func)(char**);
				//Predefined function parameters that are passed to the function pointer when called. If any custom user input is passed to the command line, it will be combined with these values
				const char *functionParams[15];
				//A sub-menu to link to this command. If this value is not null, you must set the isExecutable value to HAS_CHILD.
				const struct Command *childCommand;
				//A permissions level that restricts use of this command to authorized users.
				const PermLevel permissionsRequired;
			} Command;
			
		EXAMPLE (No custom input):
		static const Command Sub_Menu[3] = {
		{"command-text3", 	"some help text 3", TERMINATING_COMMMAND, 	3,	false, 	COM_FUNCTIONHERE, 	{"Some parameter", 0x00, 0xFF},	NO_CHILD_MENU,	ReadOnlyUser},
		{"command-text4", 	"some help text 4", TERMINATING_COMMMAND, 	3,	false, 	COM_FUNCTIONHERE, 	{"Some parameter", 0x00, 0xFF},	NO_CHILD_MENU,	Administrator},
		{0,0,0,0,0,0,0}
		};
		
		static const Command Sample_Options[3] = {
		{"command-text", 	"some help text",  	HAS_CHILD, 				3,	false, 	NotImplementedFunction, {"Some parameter", 0x00, 0xFF},	Sub_Menu,		ReadOnlyUser},
		{"command-text2", 	"some help text 2", TERMINATING_COMMMAND, 	3,	false, 	COM_FUNCTIONHERE, 		{"Some parameter", 0x00, 0xFF},	NO_CHILD_MENU,	ModifySystem},
		{0,0,0,0,0,0,0}
		};
		
		//Results in the creation of the following commands:
		//command-text2
		//command-text command-text3
		//command-text command-text4
		
		EXAMPLE (custom input):
		static const Command Custom_Sub_Menu[2] = {
		{"<hexadecimal value>", 	"a number between 0x00 and 0xFF", TERMINATING_COMMMAND, 	1,	true, 	COM_FUNCTIONHERE, 	EMPTY_STATIC_PARAMS,	NO_CHILD_MENU,	ReadOnlyUser},
		{0,0,0,0,0,0,0}
		};
		
		static const Command Sample_Options[2] = {
		{"command-text", 	"some help text",  	HAS_CHILD, 				3,	false, 	NotImplementedFunction, {"Some parameter", 0x00, 0xFF},	Custom_Sub_Menu,		ReadOnlyUser},
		{0,0,0,0,0,0,0}
		};
		
		//Results in the creation of the following commands:
		//command-text 0x00
		//command-text 0x01
		...
		//command-text 0xFF
		
		
[6] === LED MANAGER TASK [led_manager] (.c/.h) ====
This simple RTOS task generates an LED task on demand that blinks a designated status LED. This task can also kill spawned tasks on-demand. To change which status LEDs (ports and pins) are used, modify the header file accordingly.

[7] === LED TASK [led_task] (.c/.h) ===
Any RTOS task using this template will be spawned/destroyed by the LED MANAGER. Each of these tasks blinks a single LED at a provided interval. There is no need to modify this task as it does not directly interface with any other part of the firmware.

[8] === PORT MONITORING TASK [port_monitor_task] (.c/.h) ===
This task checks each port's interrupt flags at a pre-defined interval by querying the appropriate registers in the Ethernet Controller.
Adapting this task to other controllers requires changing the settings held in "interpreter_task.h". This task will check each port
sequentially and report to the user if any status change has been detected. 

[9] === FREE RTOS INITIALIZATION [freertos_init] (.c/.h) ===
This file is the main entry point for the EEE 100BaseTX firmware. Each interface is individually configured before loading any stored configuration from EEPROM. Each task can be enabled or disabled from freertos_init.h for debugging. 
		
		
		
			
		
