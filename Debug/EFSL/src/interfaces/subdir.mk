################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EFSL/src/interfaces/helper.c \
../EFSL/src/interfaces/stm32.c 

OBJS += \
./EFSL/src/interfaces/helper.o \
./EFSL/src/interfaces/stm32.o 

C_DEPS += \
./EFSL/src/interfaces/helper.d \
./EFSL/src/interfaces/stm32.d 


# Each subdirectory must supply rules for building sources it contributes
EFSL/src/interfaces/%.o EFSL/src/interfaces/%.su: ../EFSL/src/interfaces/%.c EFSL/src/interfaces/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../USB_DEVICE/Target -I../EFSL/config/ -I../EFSL/inc -I../EFSL/inc/interfaces -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc -I../USB_DEVICE/App -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-EFSL-2f-src-2f-interfaces

clean-EFSL-2f-src-2f-interfaces:
	-$(RM) ./EFSL/src/interfaces/helper.d ./EFSL/src/interfaces/helper.o ./EFSL/src/interfaces/helper.su ./EFSL/src/interfaces/stm32.d ./EFSL/src/interfaces/stm32.o ./EFSL/src/interfaces/stm32.su

.PHONY: clean-EFSL-2f-src-2f-interfaces

