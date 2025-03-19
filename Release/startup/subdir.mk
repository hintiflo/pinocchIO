################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (7-2018-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../startup/startup_stm32.s 

C_SRCS += \
../startup/sysmem.c 

S_DEPS += \
./startup/startup_stm32.d 

C_DEPS += \
./startup/sysmem.d 

OBJS += \
./startup/startup_stm32.o \
./startup/sysmem.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.s startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"
startup/%.o startup/%.su startup/%.cyclo: ../startup/%.c startup/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DSTM32 -DSTM32F1 -DSTM32F103xE -c -I"C:/Users/hintiflo/Sockus/tmp/pinocchIO/CMSIS/core" -I"C:/Users/hintiflo/Sockus/tmp/pinocchIO/CMSIS/device" -O1 -ffunction-sections -Wall -Wswitch-default -Wswitch-enum -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-startup

clean-startup:
	-$(RM) ./startup/startup_stm32.d ./startup/startup_stm32.o ./startup/sysmem.cyclo ./startup/sysmem.d ./startup/sysmem.o ./startup/sysmem.su

.PHONY: clean-startup

