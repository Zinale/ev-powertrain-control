################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Safety/MCU_State.c \
../Core/Src/Safety/device_state.c \
../Core/Src/Safety/watchdog.c 

OBJS += \
./Core/Src/Safety/MCU_State.o \
./Core/Src/Safety/device_state.o \
./Core/Src/Safety/watchdog.o 

C_DEPS += \
./Core/Src/Safety/MCU_State.d \
./Core/Src/Safety/device_state.d \
./Core/Src/Safety/watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Safety/%.o Core/Src/Safety/%.su Core/Src/Safety/%.cyclo: ../Core/Src/Safety/%.c Core/Src/Safety/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Communication" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Communication" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Safety" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Safety" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Tasks" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Tasks" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Drive" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Src/Sensors" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Drive" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-stm32/Core/Inc/Sensors" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Safety

clean-Core-2f-Src-2f-Safety:
	-$(RM) ./Core/Src/Safety/MCU_State.cyclo ./Core/Src/Safety/MCU_State.d ./Core/Src/Safety/MCU_State.o ./Core/Src/Safety/MCU_State.su ./Core/Src/Safety/device_state.cyclo ./Core/Src/Safety/device_state.d ./Core/Src/Safety/device_state.o ./Core/Src/Safety/device_state.su ./Core/Src/Safety/watchdog.cyclo ./Core/Src/Safety/watchdog.d ./Core/Src/Safety/watchdog.o ./Core/Src/Safety/watchdog.su

.PHONY: clean-Core-2f-Src-2f-Safety

