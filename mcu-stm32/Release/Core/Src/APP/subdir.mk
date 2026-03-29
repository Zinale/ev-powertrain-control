################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/APP/APPS.c 

OBJS += \
./Core/Src/APP/APPS.o 

C_DEPS += \
./Core/Src/APP/APPS.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/APP/%.o Core/Src/APP/%.su Core/Src/APP/%.cyclo: ../Core/Src/APP/%.c Core/Src/APP/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/ADC" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/APP" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Controls" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Inverter" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/SAS" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/ADC" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/APP" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Controls" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Inverter" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/SAS" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Safety" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Tasks" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Tasks" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Safety" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-APP

clean-Core-2f-Src-2f-APP:
	-$(RM) ./Core/Src/APP/APPS.cyclo ./Core/Src/APP/APPS.d ./Core/Src/APP/APPS.o ./Core/Src/APP/APPS.su

.PHONY: clean-Core-2f-Src-2f-APP

