################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (7-2018-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hal-m010.c \
../src/main.c \
../src/timers.c \
../src/usb.c 

C_DEPS += \
./src/hal-m010.d \
./src/main.d \
./src/timers.d \
./src/usb.d 

OBJS += \
./src/hal-m010.o \
./src/main.o \
./src/timers.o \
./src/usb.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o src/%.su src/%.cyclo: ../src/%.c src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DSTM32 -DSTM32F1 -DSTM32F103xE -c -I"C:/Users/hintiflo/Sockus/tmp/pinocchIO/CMSIS/core" -I"C:/Users/hintiflo/Sockus/tmp/pinocchIO/CMSIS/device" -O1 -ffunction-sections -Wall -Wswitch-default -Wswitch-enum -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-src

clean-src:
	-$(RM) ./src/hal-m010.cyclo ./src/hal-m010.d ./src/hal-m010.o ./src/hal-m010.su ./src/main.cyclo ./src/main.d ./src/main.o ./src/main.su ./src/timers.cyclo ./src/timers.d ./src/timers.o ./src/timers.su ./src/usb.cyclo ./src/usb.d ./src/usb.o ./src/usb.su

.PHONY: clean-src

