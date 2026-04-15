################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Tasks/data_logger.c \
../Core/Src/Tasks/data_manager.c \
../Core/Src/Tasks/motors_manager.c 

OBJS += \
./Core/Src/Tasks/data_logger.o \
./Core/Src/Tasks/data_manager.o \
./Core/Src/Tasks/motors_manager.o 

C_DEPS += \
./Core/Src/Tasks/data_logger.d \
./Core/Src/Tasks/data_manager.d \
./Core/Src/Tasks/motors_manager.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Tasks/%.o Core/Src/Tasks/%.su Core/Src/Tasks/%.cyclo: ../Core/Src/Tasks/%.c Core/Src/Tasks/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Communication" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Communication" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Drive" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Sensors" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Drive" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Sensors" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Tasks

clean-Core-2f-Src-2f-Tasks:
	-$(RM) ./Core/Src/Tasks/data_logger.cyclo ./Core/Src/Tasks/data_logger.d ./Core/Src/Tasks/data_logger.o ./Core/Src/Tasks/data_logger.su ./Core/Src/Tasks/data_manager.cyclo ./Core/Src/Tasks/data_manager.d ./Core/Src/Tasks/data_manager.o ./Core/Src/Tasks/data_manager.su ./Core/Src/Tasks/motors_manager.cyclo ./Core/Src/Tasks/motors_manager.d ./Core/Src/Tasks/motors_manager.o ./Core/Src/Tasks/motors_manager.su

.PHONY: clean-Core-2f-Src-2f-Tasks

