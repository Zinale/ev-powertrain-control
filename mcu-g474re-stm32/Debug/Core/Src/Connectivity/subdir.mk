################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Connectivity/Can.c \
../Core/Src/Connectivity/Serial.c 

OBJS += \
./Core/Src/Connectivity/Can.o \
./Core/Src/Connectivity/Serial.o 

C_DEPS += \
./Core/Src/Connectivity/Can.d \
./Core/Src/Connectivity/Serial.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Connectivity/%.o Core/Src/Connectivity/%.su Core/Src/Connectivity/%.cyclo: ../Core/Src/Connectivity/%.c Core/Src/Connectivity/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Inc/Control/Simulink" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Src/Control/Simulink" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Connectivity

clean-Core-2f-Src-2f-Connectivity:
	-$(RM) ./Core/Src/Connectivity/Can.cyclo ./Core/Src/Connectivity/Can.d ./Core/Src/Connectivity/Can.o ./Core/Src/Connectivity/Can.su ./Core/Src/Connectivity/Serial.cyclo ./Core/Src/Connectivity/Serial.d ./Core/Src/Connectivity/Serial.o ./Core/Src/Connectivity/Serial.su

.PHONY: clean-Core-2f-Src-2f-Connectivity

