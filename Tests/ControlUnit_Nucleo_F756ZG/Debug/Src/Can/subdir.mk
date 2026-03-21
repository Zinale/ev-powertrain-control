################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/Can/Can.c 

C_DEPS += \
./Src/Can/Can.d 

OBJS += \
./Src/Can/Can.o 


# Each subdirectory must supply rules for building sources it contributes
Src/Can/%.o Src/Can/%.su Src/Can/%.cyclo: ../Src/Can/%.c Src/Can/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Can" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Scheduler" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Can" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Scheduler" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Utils" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Utils" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-Can

clean-Src-2f-Can:
	-$(RM) ./Src/Can/Can.cyclo ./Src/Can/Can.d ./Src/Can/Can.o ./Src/Can/Can.su

.PHONY: clean-Src-2f-Can

