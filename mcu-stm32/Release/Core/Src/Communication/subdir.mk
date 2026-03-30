################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Communication/Can.c \
../Core/Src/Communication/Serial.c 

OBJS += \
./Core/Src/Communication/Can.o \
./Core/Src/Communication/Serial.o 

C_DEPS += \
./Core/Src/Communication/Can.d \
./Core/Src/Communication/Serial.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Communication/%.o Core/Src/Communication/%.su Core/Src/Communication/%.cyclo: ../Core/Src/Communication/%.c Core/Src/Communication/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/ADC" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/APP" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Communication" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Controls" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/SAS" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/ADC" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/APP" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Controls" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Communication" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/SAS" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Safety" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Communication

clean-Core-2f-Src-2f-Communication:
	-$(RM) ./Core/Src/Communication/Can.cyclo ./Core/Src/Communication/Can.d ./Core/Src/Communication/Can.o ./Core/Src/Communication/Can.su ./Core/Src/Communication/Serial.cyclo ./Core/Src/Communication/Serial.d ./Core/Src/Communication/Serial.o ./Core/Src/Communication/Serial.su

.PHONY: clean-Core-2f-Src-2f-Communication

