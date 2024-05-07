################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Utilities/Log/lcd_log.c 

OBJS += \
./Utilities/Log/lcd_log.o 

C_DEPS += \
./Utilities/Log/lcd_log.d 


# Each subdirectory must supply rules for building sources it contributes
Utilities/Log/%.o Utilities/Log/%.su Utilities/Log/%.cyclo: ../Utilities/Log/%.c Utilities/Log/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM4 -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/DSP/Include -I../Drivers/BSP/STM32F429I-Discovery -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/STM32F4xx_HAL_Driver/Inc -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Middlewares/Third_Party/FatFs/src -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/CMSIS/Include -I../../../Utilities/Log/lcd_log.h -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Utilities-2f-Log

clean-Utilities-2f-Log:
	-$(RM) ./Utilities/Log/lcd_log.cyclo ./Utilities/Log/lcd_log.d ./Utilities/Log/lcd_log.o ./Utilities/Log/lcd_log.su

.PHONY: clean-Utilities-2f-Log

