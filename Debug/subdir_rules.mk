################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
command_functions.obj: ../command_functions.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="command_functions.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

eee_hal.obj: ../eee_hal.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="eee_hal.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

event_logger.obj: ../event_logger.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="event_logger.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos_init.obj: ../freertos_init.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="freertos_init.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

i2c_task.obj: ../i2c_task.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="i2c_task.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

interpreter_task.obj: ../interpreter_task.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="interpreter_task.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

led_manager.obj: ../led_manager.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="led_manager.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

led_task.obj: ../led_task.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="led_task.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

port_monitor_task.obj: ../port_monitor_task.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="port_monitor_task.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup_ccs.obj: ../startup_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_16.3.0.STS/include" --include_path="C:/Users/aggie2016/workspace_v6_1_3/EEESwitchFirmware" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/examples/boards/ek-tm4c123gxl" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/driverlib" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/utils" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/include" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS" --include_path="C:/ti/TivaWare_C_Series-2.1.2.111/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --advice:power="all" -g --gcc --define=ccs="ccs" --define=PART_TM4C123GH6PM --define=TARGET_IS_TM4C123_RB1 --define=EEE_SWITCH_FIRMWARE --display_error_number --diag_wrap=off --diag_warning=225 --gen_func_subsections=on --abi=eabi --ual --preproc_with_compile --preproc_dependency="startup_ccs.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


