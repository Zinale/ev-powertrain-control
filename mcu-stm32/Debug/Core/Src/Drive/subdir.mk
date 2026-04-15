################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Drive/BaseControlMotor.c \
../Core/Src/Drive/ErrorRecovery.c \
../Core/Src/Drive/Inverter.c \
../Core/Src/Drive/Regen.c 

OBJS += \
./Core/Src/Drive/BaseControlMotor.o \
./Core/Src/Drive/ErrorRecovery.o \
./Core/Src/Drive/Inverter.o \
./Core/Src/Drive/Regen.o 

C_DEPS += \
./Core/Src/Drive/BaseControlMotor.d \
./Core/Src/Drive/ErrorRecovery.d \
./Core/Src/Drive/Inverter.d \
./Core/Src/Drive/Regen.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Drive/%.o Core/Src/Drive/%.su Core/Src/Drive/%.cyclo: ../Core/Src/Drive/%.c Core/Src/Drive/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Communication" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Communication" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Drive" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Sensors" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Drive" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Sensors" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Drive

clean-Core-2f-Src-2f-Drive:
	-$(RM) ./Core/Src/Drive/BaseControlMotor.cyclo ./Core/Src/Drive/BaseControlMotor.d ./Core/Src/Drive/BaseControlMotor.o ./Core/Src/Drive/BaseControlMotor.su ./Core/Src/Drive/ErrorRecovery.cyclo ./Core/Src/Drive/ErrorRecovery.d ./Core/Src/Drive/ErrorRecovery.o ./Core/Src/Drive/ErrorRecovery.su ./Core/Src/Drive/Inverter.cyclo ./Core/Src/Drive/Inverter.d ./Core/Src/Drive/Inverter.o ./Core/Src/Drive/Inverter.su ./Core/Src/Drive/Regen.cyclo ./Core/Src/Drive/Regen.d ./Core/Src/Drive/Regen.o ./Core/Src/Drive/Regen.su

.PHONY: clean-Core-2f-Src-2f-Drive

