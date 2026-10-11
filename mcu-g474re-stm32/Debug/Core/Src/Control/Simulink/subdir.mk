################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Control/Simulink/driveController.c \
../Core/Src/Control/Simulink/driveController_data.c \
../Core/Src/Control/Simulink/rtGetNaN.c \
../Core/Src/Control/Simulink/rt_nonfinite.c 

OBJS += \
./Core/Src/Control/Simulink/driveController.o \
./Core/Src/Control/Simulink/driveController_data.o \
./Core/Src/Control/Simulink/rtGetNaN.o \
./Core/Src/Control/Simulink/rt_nonfinite.o 

C_DEPS += \
./Core/Src/Control/Simulink/driveController.d \
./Core/Src/Control/Simulink/driveController_data.d \
./Core/Src/Control/Simulink/rtGetNaN.d \
./Core/Src/Control/Simulink/rt_nonfinite.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Control/Simulink/%.o Core/Src/Control/Simulink/%.su Core/Src/Control/Simulink/%.cyclo: ../Core/Src/Control/Simulink/%.c Core/Src/Control/Simulink/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G474xx -c -I../Core/Inc -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Inc/Control/Simulink" -I"E:/UNIVPM/PolimarcheRacingTeam/EV-POWERTRAIN-CONTROL/ev-powertrain-control/mcu-g474re-stm32/Core/Src/Control/Simulink" -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Control-2f-Simulink

clean-Core-2f-Src-2f-Control-2f-Simulink:
	-$(RM) ./Core/Src/Control/Simulink/driveController.cyclo ./Core/Src/Control/Simulink/driveController.d ./Core/Src/Control/Simulink/driveController.o ./Core/Src/Control/Simulink/driveController.su ./Core/Src/Control/Simulink/driveController_data.cyclo ./Core/Src/Control/Simulink/driveController_data.d ./Core/Src/Control/Simulink/driveController_data.o ./Core/Src/Control/Simulink/driveController_data.su ./Core/Src/Control/Simulink/rtGetNaN.cyclo ./Core/Src/Control/Simulink/rtGetNaN.d ./Core/Src/Control/Simulink/rtGetNaN.o ./Core/Src/Control/Simulink/rtGetNaN.su ./Core/Src/Control/Simulink/rt_nonfinite.cyclo ./Core/Src/Control/Simulink/rt_nonfinite.d ./Core/Src/Control/Simulink/rt_nonfinite.o ./Core/Src/Control/Simulink/rt_nonfinite.su

.PHONY: clean-Core-2f-Src-2f-Control-2f-Simulink

