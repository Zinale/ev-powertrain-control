################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Tasks/data_logger.c \
../Core/Src/Tasks/inverters_manage.c \
../Core/Src/Tasks/readings_manage.c 

OBJS += \
./Core/Src/Tasks/data_logger.o \
./Core/Src/Tasks/inverters_manage.o \
./Core/Src/Tasks/readings_manage.o 

C_DEPS += \
./Core/Src/Tasks/data_logger.d \
./Core/Src/Tasks/inverters_manage.d \
./Core/Src/Tasks/readings_manage.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Tasks/%.o Core/Src/Tasks/%.su Core/Src/Tasks/%.cyclo: ../Core/Src/Tasks/%.c Core/Src/Tasks/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/APP" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/APP" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Controls" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Communication" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Communication" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/ADC" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/ADC" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/SAS" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/SAS" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Src/Safety" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/FRONT/front-PeacockElettrica/MCU/mcu-stm32/Core/Inc/Controls" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Tasks

clean-Core-2f-Src-2f-Tasks:
	-$(RM) ./Core/Src/Tasks/data_logger.cyclo ./Core/Src/Tasks/data_logger.d ./Core/Src/Tasks/data_logger.o ./Core/Src/Tasks/data_logger.su ./Core/Src/Tasks/inverters_manage.cyclo ./Core/Src/Tasks/inverters_manage.d ./Core/Src/Tasks/inverters_manage.o ./Core/Src/Tasks/inverters_manage.su ./Core/Src/Tasks/readings_manage.cyclo ./Core/Src/Tasks/readings_manage.d ./Core/Src/Tasks/readings_manage.o ./Core/Src/Tasks/readings_manage.su

.PHONY: clean-Core-2f-Src-2f-Tasks

