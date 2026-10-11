################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Tasks/data_logger.c \
../Core/Src/Tasks/data_manager.c \
../Core/Src/Tasks/drive_logic.c 

OBJS += \
./Core/Src/Tasks/data_logger.o \
./Core/Src/Tasks/data_manager.o \
./Core/Src/Tasks/drive_logic.o 

C_DEPS += \
./Core/Src/Tasks/data_logger.d \
./Core/Src/Tasks/data_manager.d \
./Core/Src/Tasks/drive_logic.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Tasks/%.o Core/Src/Tasks/%.su Core/Src/Tasks/%.cyclo: ../Core/Src/Tasks/%.c Core/Src/Tasks/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Inc/Control/Simulink" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Src/Control/Simulink" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Tasks

clean-Core-2f-Src-2f-Tasks:
	-$(RM) ./Core/Src/Tasks/data_logger.cyclo ./Core/Src/Tasks/data_logger.d ./Core/Src/Tasks/data_logger.o ./Core/Src/Tasks/data_logger.su ./Core/Src/Tasks/data_manager.cyclo ./Core/Src/Tasks/data_manager.d ./Core/Src/Tasks/data_manager.o ./Core/Src/Tasks/data_manager.su ./Core/Src/Tasks/drive_logic.cyclo ./Core/Src/Tasks/drive_logic.d ./Core/Src/Tasks/drive_logic.o ./Core/Src/Tasks/drive_logic.su

.PHONY: clean-Core-2f-Src-2f-Tasks

