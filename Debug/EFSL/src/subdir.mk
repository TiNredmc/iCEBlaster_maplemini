################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../EFSL/src/debug.c \
../EFSL/src/dir.c \
../EFSL/src/disc.c \
../EFSL/src/efs.c \
../EFSL/src/extract.c \
../EFSL/src/fat.c \
../EFSL/src/file.c \
../EFSL/src/fs.c \
../EFSL/src/ioman.c \
../EFSL/src/ls.c \
../EFSL/src/mkfs.c \
../EFSL/src/partition.c \
../EFSL/src/plibc.c \
../EFSL/src/time.c \
../EFSL/src/ui.c 

OBJS += \
./EFSL/src/debug.o \
./EFSL/src/dir.o \
./EFSL/src/disc.o \
./EFSL/src/efs.o \
./EFSL/src/extract.o \
./EFSL/src/fat.o \
./EFSL/src/file.o \
./EFSL/src/fs.o \
./EFSL/src/ioman.o \
./EFSL/src/ls.o \
./EFSL/src/mkfs.o \
./EFSL/src/partition.o \
./EFSL/src/plibc.o \
./EFSL/src/time.o \
./EFSL/src/ui.o 

C_DEPS += \
./EFSL/src/debug.d \
./EFSL/src/dir.d \
./EFSL/src/disc.d \
./EFSL/src/efs.d \
./EFSL/src/extract.d \
./EFSL/src/fat.d \
./EFSL/src/file.d \
./EFSL/src/fs.d \
./EFSL/src/ioman.d \
./EFSL/src/ls.d \
./EFSL/src/mkfs.d \
./EFSL/src/partition.d \
./EFSL/src/plibc.d \
./EFSL/src/time.d \
./EFSL/src/ui.d 


# Each subdirectory must supply rules for building sources it contributes
EFSL/src/%.o EFSL/src/%.su: ../EFSL/src/%.c EFSL/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../USB_DEVICE/Target -I../EFSL/config/ -I../EFSL/inc -I../EFSL/inc/interfaces -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../Core/Inc -I../USB_DEVICE/App -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-EFSL-2f-src

clean-EFSL-2f-src:
	-$(RM) ./EFSL/src/debug.d ./EFSL/src/debug.o ./EFSL/src/debug.su ./EFSL/src/dir.d ./EFSL/src/dir.o ./EFSL/src/dir.su ./EFSL/src/disc.d ./EFSL/src/disc.o ./EFSL/src/disc.su ./EFSL/src/efs.d ./EFSL/src/efs.o ./EFSL/src/efs.su ./EFSL/src/extract.d ./EFSL/src/extract.o ./EFSL/src/extract.su ./EFSL/src/fat.d ./EFSL/src/fat.o ./EFSL/src/fat.su ./EFSL/src/file.d ./EFSL/src/file.o ./EFSL/src/file.su ./EFSL/src/fs.d ./EFSL/src/fs.o ./EFSL/src/fs.su ./EFSL/src/ioman.d ./EFSL/src/ioman.o ./EFSL/src/ioman.su ./EFSL/src/ls.d ./EFSL/src/ls.o ./EFSL/src/ls.su ./EFSL/src/mkfs.d ./EFSL/src/mkfs.o ./EFSL/src/mkfs.su ./EFSL/src/partition.d ./EFSL/src/partition.o ./EFSL/src/partition.su ./EFSL/src/plibc.d ./EFSL/src/plibc.o ./EFSL/src/plibc.su ./EFSL/src/time.d ./EFSL/src/time.o ./EFSL/src/time.su ./EFSL/src/ui.d ./EFSL/src/ui.o ./EFSL/src/ui.su

.PHONY: clean-EFSL-2f-src

