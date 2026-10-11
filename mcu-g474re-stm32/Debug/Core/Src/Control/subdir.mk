################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Control/drive_profiles.c \
../Core/Src/Control/error_recovery.c \
../Core/Src/Control/inverter.c \
../Core/Src/Control/motor_control.c \
../Core/Src/Control/plausibility.c 

OBJS += \
./Core/Src/Control/drive_profiles.o \
./Core/Src/Control/error_recovery.o \
./Core/Src/Control/inverter.o \
./Core/Src/Control/motor_control.o \
./Core/Src/Control/plausibility.o 

C_DEPS += \
./Core/Src/Control/drive_profiles.d \
./Core/Src/Control/error_recovery.d \
./Core/Src/Control/inverter.d \
./Core/Src/Control/motor_control.d \
./Core/Src/Control/plausibility.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Control/%.o Core/Src/Control/%.su Core/Src/Control/%.cyclo: ../Core/Src/Control/%.c Core/Src/Control/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Inc/Control/Simulink" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Src/Control/Simulink" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Control

clean-Core-2f-Src-2f-Control:
	-$(RM) ./Core/Src/Control/drive_profiles.cyclo ./Core/Src/Control/drive_profiles.d ./Core/Src/Control/drive_profiles.o ./Core/Src/Control/drive_profiles.su ./Core/Src/Control/error_recovery.cyclo ./Core/Src/Control/error_recovery.d ./Core/Src/Control/error_recovery.o ./Core/Src/Control/error_recovery.su ./Core/Src/Control/inverter.cyclo ./Core/Src/Control/inverter.d ./Core/Src/Control/inverter.o ./Core/Src/Control/inverter.su ./Core/Src/Control/motor_control.cyclo ./Core/Src/Control/motor_control.d ./Core/Src/Control/motor_control.o ./Core/Src/Control/motor_control.su ./Core/Src/Control/plausibility.cyclo ./Core/Src/Control/plausibility.d ./Core/Src/Control/plausibility.o ./Core/Src/Control/plausibility.su

.PHONY: clean-Core-2f-Src-2f-Control

