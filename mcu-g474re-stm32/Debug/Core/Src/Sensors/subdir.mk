################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Sensors/adc_manager.c \
../Core/Src/Sensors/apps.c \
../Core/Src/Sensors/bs.c \
../Core/Src/Sensors/filters.c \
../Core/Src/Sensors/sas.c \
../Core/Src/Sensors/wss.c 

OBJS += \
./Core/Src/Sensors/adc_manager.o \
./Core/Src/Sensors/apps.o \
./Core/Src/Sensors/bs.o \
./Core/Src/Sensors/filters.o \
./Core/Src/Sensors/sas.o \
./Core/Src/Sensors/wss.o 

C_DEPS += \
./Core/Src/Sensors/adc_manager.d \
./Core/Src/Sensors/apps.d \
./Core/Src/Sensors/bs.d \
./Core/Src/Sensors/filters.d \
./Core/Src/Sensors/sas.d \
./Core/Src/Sensors/wss.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Sensors/%.o Core/Src/Sensors/%.su Core/Src/Sensors/%.cyclo: ../Core/Src/Sensors/%.c Core/Src/Sensors/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Inc/Control/Simulink" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Src/Control/Simulink" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Sensors

clean-Core-2f-Src-2f-Sensors:
	-$(RM) ./Core/Src/Sensors/adc_manager.cyclo ./Core/Src/Sensors/adc_manager.d ./Core/Src/Sensors/adc_manager.o ./Core/Src/Sensors/adc_manager.su ./Core/Src/Sensors/apps.cyclo ./Core/Src/Sensors/apps.d ./Core/Src/Sensors/apps.o ./Core/Src/Sensors/apps.su ./Core/Src/Sensors/bs.cyclo ./Core/Src/Sensors/bs.d ./Core/Src/Sensors/bs.o ./Core/Src/Sensors/bs.su ./Core/Src/Sensors/filters.cyclo ./Core/Src/Sensors/filters.d ./Core/Src/Sensors/filters.o ./Core/Src/Sensors/filters.su ./Core/Src/Sensors/sas.cyclo ./Core/Src/Sensors/sas.d ./Core/Src/Sensors/sas.o ./Core/Src/Sensors/sas.su ./Core/Src/Sensors/wss.cyclo ./Core/Src/Sensors/wss.d ./Core/Src/Sensors/wss.o ./Core/Src/Sensors/wss.su

.PHONY: clean-Core-2f-Src-2f-Sensors

