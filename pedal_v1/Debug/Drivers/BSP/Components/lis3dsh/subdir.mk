################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Components/lis3dsh/lis3dsh.c 

OBJS += \
./Drivers/BSP/Components/lis3dsh/lis3dsh.o 

C_DEPS += \
./Drivers/BSP/Components/lis3dsh/lis3dsh.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/lis3dsh/%.o Drivers/BSP/Components/lis3dsh/%.su Drivers/BSP/Components/lis3dsh/%.cyclo: ../Drivers/BSP/Components/lis3dsh/%.c Drivers/BSP/Components/lis3dsh/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/BSP/STM32F4-Discovery -I../Drivers/CMSIS/Include -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Drivers/BSP/Components/cs43l22 -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components-2f-lis3dsh

clean-Drivers-2f-BSP-2f-Components-2f-lis3dsh:
	-$(RM) ./Drivers/BSP/Components/lis3dsh/lis3dsh.cyclo ./Drivers/BSP/Components/lis3dsh/lis3dsh.d ./Drivers/BSP/Components/lis3dsh/lis3dsh.o ./Drivers/BSP/Components/lis3dsh/lis3dsh.su

.PHONY: clean-Drivers-2f-BSP-2f-Components-2f-lis3dsh

