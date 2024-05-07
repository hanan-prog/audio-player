################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/ls016b8uy/ls016b8uy.c 

OBJS += \
./Drivers/BSP/Components/ls016b8uy/ls016b8uy.o 

C_DEPS += \
./Drivers/BSP/Components/ls016b8uy/ls016b8uy.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/ls016b8uy/%.o Drivers/BSP/Components/ls016b8uy/%.su Drivers/BSP/Components/ls016b8uy/%.cyclo: ../Drivers/BSP/Components/ls016b8uy/%.c Drivers/BSP/Components/ls016b8uy/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DARM_MATH_CM4 -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Drivers/DSP/Include -I../Drivers/BSP/STM32F429I-Discovery -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/STM32F4xx_HAL_Driver/Inc -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Middlewares/Third_Party/FatFs/src -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/Users/hanan/STM32Cube/Repository/STM32Cube_FW_F4_V1.28.0/Drivers/CMSIS/Include -I../../../Utilities/Log/lcd_log.h -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-ls016b8uy

clean-Drivers-2f-BSP-2f-Components-2f-ls016b8uy:
	-$(RM) ./Drivers/BSP/Components/ls016b8uy/ls016b8uy.cyclo ./Drivers/BSP/Components/ls016b8uy/ls016b8uy.d ./Drivers/BSP/Components/ls016b8uy/ls016b8uy.o ./Drivers/BSP/Components/ls016b8uy/ls016b8uy.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-ls016b8uy

