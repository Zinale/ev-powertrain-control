################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/stm32f7xx_hal_msp.c \
../Src/stm32f7xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32f7xx.c 

C_DEPS += \
./Src/main.d \
./Src/stm32f7xx_hal_msp.d \
./Src/stm32f7xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32f7xx.d 

OBJS += \
./Src/main.o \
./Src/stm32f7xx_hal_msp.o \
./Src/stm32f7xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32f7xx.o 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Can" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Scheduler" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Can" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Scheduler" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Tasks" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Inverter" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Src/Utils" -I"C:/Users/zinga/Documents/.UNIVPM/PolimarcheRacingTeam/Inverter/NUCLEO_controls/ControlUnit_Nucleo_F756ZG/Inc/Utils" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/stm32f7xx_hal_msp.cyclo ./Src/stm32f7xx_hal_msp.d ./Src/stm32f7xx_hal_msp.o ./Src/stm32f7xx_hal_msp.su ./Src/stm32f7xx_it.cyclo ./Src/stm32f7xx_it.d ./Src/stm32f7xx_it.o ./Src/stm32f7xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_stm32f7xx.cyclo ./Src/system_stm32f7xx.d ./Src/system_stm32f7xx.o ./Src/system_stm32f7xx.su

.PHONY: clean-Src

