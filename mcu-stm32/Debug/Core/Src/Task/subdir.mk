################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Task/Tasks.c 

OBJS += \
./Core/Src/Task/Tasks.o 

C_DEPS += \
./Core/Src/Task/Tasks.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Task/%.o Core/Src/Task/%.su Core/Src/Task/%.cyclo: ../Core/Src/Task/%.c Core/Src/Task/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Inverter" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Scheduler" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Task" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Inverter" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Scheduler" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Task" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/APP" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/APP" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Controls" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/ADC" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/ADC" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/SAS" -I"E:/UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/SAS" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Task

clean-Core-2f-Src-2f-Task:
	-$(RM) ./Core/Src/Task/Tasks.cyclo ./Core/Src/Task/Tasks.d ./Core/Src/Task/Tasks.o ./Core/Src/Task/Tasks.su

.PHONY: clean-Core-2f-Src-2f-Task

