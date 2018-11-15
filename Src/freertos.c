/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2018 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     

#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "usart.h"
#include "lcd.h"
#include "BMP280.h"
#include "ringbuffer_dma.h"
#include <string.h>
#include "at_parser.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId communicationTaHandle;
osThreadId displayTaskHandle;
osThreadId sensorTaskHandle;
osMutexId weatherMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

osMailQId mailWeatherHandle;

typedef enum {
	normal,
} WeatherType;

/* Data structure for weather */
typedef struct {
	double temp;
	double press;
	WeatherType weatherPredict;
} Weather;

Weather currentWeather = {0};
int8_t com_rslt;

RingBuffer_DMA rx_buf;

#define BUF_SIZE 256
uint8_t rx[BUF_SIZE];
uint32_t rx_count = 0;

char cmd[128];
uint8_t icmd = 0;

uint8_t tx[100];
uint32_t lastTick = 0;

uint8_t isTransferWether = 0;
uint32_t transferInterval = 1000;

void Process_Command(char * command);
Weather weatherData(void);
uint8_t isEmptyWeatherStruct(Weather wData);
char getTemp(char *value);
char getPress(char *value);
char getPrediction(char *value);
char getWeather(char *value);
char execStartTransferWeather(char *value);
char execStopTransferWeather(char *value);
char setTransferInterval(char *value);
char getTransferInterval(char *value);
char testTransferInterval(char *value);

/* USER CODE END FunctionPrototypes */

void StartWaitCommand(void const * argument);
void StartUpdateDisplay(void const * argument);
void StartReadSensor(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Create the mutex(es) */
	/* definition and creation of weatherMutex */
	osMutexDef(weatherMutex);
	weatherMutexHandle = osMutexCreate(osMutex(weatherMutex));

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of communicationTa */
	osThreadDef(communicationTa, StartWaitCommand, osPriorityNormal, 0, 512);
	communicationTaHandle = osThreadCreate(osThread(communicationTa), NULL);

	/* definition and creation of displayTask */
	osThreadDef(displayTask, StartUpdateDisplay, osPriorityLow, 0, 512);
	displayTaskHandle = osThreadCreate(osThread(displayTask), NULL);

	/* definition and creation of sensorTask */
	osThreadDef(sensorTask, StartReadSensor, osPriorityHigh, 0, 512);
	sensorTaskHandle = osThreadCreate(osThread(sensorTask), NULL);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */

	osMailQDef(mailWeather, 16, Weather);
	mailWeatherHandle = osMailCreate(osMailQ(mailWeather), NULL);

	/* USER CODE END RTOS_QUEUES */
}

/* USER CODE BEGIN Header_StartWaitCommand */
/**
 * @brief  Function implementing the communicationTa thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartWaitCommand */
void StartWaitCommand(void const * argument)
{
	/* USER CODE BEGIN StartWaitCommand */
	/* Init RingBuffer_DMA object */
	RingBuffer_DMA_Init(&rx_buf, huart4.hdmarx, rx, BUF_SIZE);
	/* Start UART4 DMA Reception */
	HAL_UART_Receive_DMA(&huart4, rx, BUF_SIZE);

	at_register_command("TEMP", (at_callback)getTemp, 0, 0, 0);
	at_register_command("PRES", (at_callback)getPress, 0, 0, 0);
	at_register_command("PRED", (at_callback)getPrediction, 0, 0, 0);
	at_register_command("WEATH", (at_callback)getWeather, 0, 0, 0);
	at_register_command("START", 0, 0, 0, (at_callback)execStartTransferWeather);
	at_register_command("STOP", 0, 0, 0, (at_callback)execStopTransferWeather);
	at_register_command("INT", (at_callback)getTransferInterval, (at_callback)setTransferInterval, (at_callback)testTransferInterval, 0);

	/* Infinite loop */
	for(;;)
	{
		rx_count = RingBuffer_DMA_Count(&rx_buf);

		while (rx_count--) {
			/* Read out one byte from RingBuffer */
			uint8_t b = RingBuffer_DMA_GetByte(&rx_buf);
			if (b == '\n') { /* If \n process command */
				/* Terminate string with \0 */
				cmd[icmd] = 0;
				icmd = 0;
				/* Process command */
				Process_Command(cmd);
			} else if (b == '\r') { /* If \r skip */
				continue;
			} else { /* If regular character, put it into cmd[] */
				cmd[icmd++] = b;
			}
		}
		/* Send ping message every second */
		if (isTransferWether && HAL_GetTick() - lastTick > transferInterval) {
			Process_Command("AT+WEATH?");
			lastTick = HAL_GetTick();
		}

		osDelay(20);
	}
	/* USER CODE END StartWaitCommand */
}

/* USER CODE BEGIN Header_StartUpdateDisplay */
/**
 * @brief Function implementing the displayTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartUpdateDisplay */
void StartUpdateDisplay(void const * argument)
{
	/* USER CODE BEGIN StartUpdateDisplay */

	Weather *weatherData;
	osEvent evt;

	LCD_Init();

	/* Infinite loop */
	for(;;)
	{
		evt = osMailGet(mailWeatherHandle, osWaitForever);
		if (evt.status == osEventMail) {
			/* Obtain data from mail */
			weatherData = (Weather *) evt.value.p;

			LCD_SetCursor(0, 0);
			LCD_Printf("Temp : %6.2f C       \n", weatherData->temp);
			LCD_Printf("Press: %6.2f kPa\n", weatherData->press);
			LCD_Printf("Weather Predict  : %s", "Normal");

			/* Free the cell in mail queue */
			osMailFree(mailWeatherHandle, weatherData);
		}
		osDelay(40);

	}
	/* USER CODE END StartUpdateDisplay */
}

/* USER CODE BEGIN Header_StartReadSensor */
/**
 * @brief Function implementing the sensorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartReadSensor */
void StartReadSensor(void const * argument)
{
	/* USER CODE BEGIN StartReadSensor */
	bmp280_t bmp280;
	Weather *weatherData;

	osStatus status;
	double press, temp;

	do {
		bmp280.i2c_handle = &hi2c1;
		bmp280.dev_addr = BMP280_I2C_ADDRESS1;
		com_rslt = BMP280_init(&bmp280);
		com_rslt += BMP280_set_power_mode(BMP280_NORMAL_MODE);
		com_rslt += BMP280_set_work_mode(BMP280_STANDARD_RESOLUTION_MODE);
		com_rslt += BMP280_set_standby_durn(BMP280_STANDBY_TIME_1_MS);
		if (com_rslt != SUCCESS) {
			osThreadYield();
		}
	} while(com_rslt != SUCCESS);

	/* Infinite loop */
	for(;;)
	{
		BMP280_read_temperature_double(&temp);
		BMP280_read_pressure_double(&press);

		weatherData = (Weather *) osMailAlloc(mailWeatherHandle, 5);

		if (weatherData !=NULL) {
			weatherData->press = press / 1000.0;
			weatherData->temp = temp;
			osMailPut(mailWeatherHandle, weatherData);
		}

		status = osMutexWait(weatherMutexHandle, 20);
		if (status == osOK) {
			currentWeather.press = press;
			currentWeather.temp = temp;
			osMutexRelease(weatherMutexHandle);
		}

		osDelay(20);
	}
	/* USER CODE END StartReadSensor */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

char getTemp(char *value) {
	Weather wData = weatherData();
	if (isEmptyWeatherStruct(wData)) {
		return AT_ERROR;
	}

	char result[50] = {0};
	sprintf(result, "Temp: %.2f\r\n", wData.temp);

	strcpy(value, result);
	return AT_OK;
}

char getPress(char *value) {
	Weather wData = weatherData();
	if (isEmptyWeatherStruct(wData)) {
		return AT_ERROR;
	}

	char result[100] = {0};
	sprintf(result, "Press: %.2f\r\n", wData.press);

	strcpy(value, result);
	return AT_OK;
}

char getPrediction(char *value) {
	Weather wData = weatherData();
	if (isEmptyWeatherStruct(wData)) {
		return AT_ERROR;
	}

	char result[100] = {0};
	sprintf(result, "Prediction: %s\r\n", "Normal");

	strcpy(value, result);
	return AT_OK;
}

char getWeather(char *value) {
	Weather wData = weatherData();
	if (isEmptyWeatherStruct(wData)) {
		return AT_ERROR;
	}

	char result[100] = {0};
	sprintf(result, "Temp: %.2f, Press: %.2f, Prediction: %s\r\n", wData.temp, wData.press, "Normal");

	strcpy(value, result);
	return AT_OK;
}

char execStartTransferWeather(char *value) {
	isTransferWether = 1;
	return AT_OK;
}

char execStopTransferWeather(char *value) {
	isTransferWether = 0;
	return AT_OK;
}

char setTransferInterval(char *value) {
	uint32_t result = atoi(value);

	if (result < 1000 || result > 30000) {
		return AT_ERROR;
	}

	transferInterval = result;

	return AT_OK;
}

char getTransferInterval(char *value) {
	char result[100] = {0};
	sprintf(result, "Interval: %d\r\n", transferInterval);

	strcpy(value, result);
	return AT_OK;
}

char testTransferInterval(char *value) {
	strcpy(value, "Interval: 1000 - 30000\r\n");
	return AT_OK;
}

Weather weatherData(void) {
	Weather result = {0};
	osStatus status = osMutexWait(weatherMutexHandle, 20);

	if (status == osOK) {
		result.temp = currentWeather.temp;
		result.press = currentWeather.temp;
		result.weatherPredict = currentWeather.weatherPredict;
		osMutexRelease(weatherMutexHandle);
	}

	return result;
}

uint8_t isEmptyWeatherStruct(Weather wData) {
	return 0 == wData.temp && 0 == wData.press && 0 == wData.weatherPredict;
}

void Process_Command(char * command) {
	char result[100] = {0};
	char ret = at_parse_line(command, result);

	if (AT_OK == ret) {
		if (strlen(result) == 0) {
			sprintf((char *) tx, "AT_OK\r\n");
		} else {
			sprintf((char *) tx, result);
		}
	} else {
		sprintf((char *) tx, "AT_ERROR\r\n");
	}

	HAL_UART_Transmit(&huart4, tx, strlen((char *) tx), 100);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
