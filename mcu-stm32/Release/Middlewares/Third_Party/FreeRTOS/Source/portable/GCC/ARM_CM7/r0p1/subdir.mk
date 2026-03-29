################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c 

OBJS += \
./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.o 

C_DEPS += \
./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/%.o Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/%.su Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/%.cyclo: ../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/%.c Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/ADC" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/APP" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Controls" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Inverter" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/SAS" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/ADC" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/APP" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Controls" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Inverter" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/SAS" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Safety" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Tasks" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Tasks" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Safety" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-FreeRTOS-2f-Source-2f-portable-2f-GCC-2f-ARM_CM7-2f-r0p1

clean-Middlewares-2f-Third_Party-2f-FreeRTOS-2f-Source-2f-portable-2f-GCC-2f-ARM_CM7-2f-r0p1:
	-$(RM) ./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.cyclo ./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.d ./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.o ./Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-FreeRTOS-2f-Source-2f-portable-2f-GCC-2f-ARM_CM7-2f-r0p1

