################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/display/Fonts/font12.c \
../Drivers/display/Fonts/font16.c \
../Drivers/display/Fonts/font20.c \
../Drivers/display/Fonts/font24.c \
../Drivers/display/Fonts/font8.c 

OBJS += \
./Drivers/display/Fonts/font12.o \
./Drivers/display/Fonts/font16.o \
./Drivers/display/Fonts/font20.o \
./Drivers/display/Fonts/font24.o \
./Drivers/display/Fonts/font8.o 

C_DEPS += \
./Drivers/display/Fonts/font12.d \
./Drivers/display/Fonts/font16.d \
./Drivers/display/Fonts/font20.d \
./Drivers/display/Fonts/font24.d \
./Drivers/display/Fonts/font8.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/display/Fonts/%.o: ../Drivers/display/Fonts/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F446xx -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Inc" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/ATParser" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/ringbuffer_dma" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/BMP280" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/display" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/STM32F4xx_HAL_Driver/Inc" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


