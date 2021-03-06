################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/commandTask.c \
../Src/dma.c \
../Src/freertos.c \
../Src/gpio.c \
../Src/i2c.c \
../Src/main.c \
../Src/readSensors.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_hal_timebase_TIM.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/updateDisplay.c \
../Src/usart.c \
../Src/weatherForecast.c 

OBJS += \
./Src/commandTask.o \
./Src/dma.o \
./Src/freertos.o \
./Src/gpio.o \
./Src/i2c.o \
./Src/main.o \
./Src/readSensors.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_hal_timebase_TIM.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/updateDisplay.o \
./Src/usart.o \
./Src/weatherForecast.o 

C_DEPS += \
./Src/commandTask.d \
./Src/dma.d \
./Src/freertos.d \
./Src/gpio.d \
./Src/i2c.d \
./Src/main.d \
./Src/readSensors.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_hal_timebase_TIM.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/updateDisplay.d \
./Src/usart.d \
./Src/weatherForecast.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F446xx -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Inc" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/ATParser" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/ringbuffer_dma" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/BMP280" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/display" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/STM32F4xx_HAL_Driver/Inc" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/Users/vitaliyvoronok/Documents/TechMaker/WeatherStation_001/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


