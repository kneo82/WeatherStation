/*
 * commandTask.c
 *
 *  Created on: Nov 20, 2018
 *      Author: vitaliyvoronok
 */

#include "main.h"
#include "dataTypes.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "usart.h"
#include "lcd.h"
#include "cmsis_os.h"
#include "ringbuffer_dma.h"
#include <string.h>
#include "at_parser.h"
#include "weatherForecast.h"
#include "dataTypes.h"

extern Weather currentWeather;
extern osMutexId weatherMutexHandle;

RingBuffer_DMA rx_buf;
#define BUF_SIZE 256
uint8_t rx[BUF_SIZE];
uint32_t rx_count = 0;

char cmd[128];
uint8_t icmd = 0;

uint8_t tx[150];
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

	char result[150] = {0};
	sprintf(result, "Press: %.2f\r\n", wData.press);

	strcpy(value, result);
	return AT_OK;
}

char getPrediction(char *value) {
	Weather wData = weatherData();
	if (isEmptyWeatherStruct(wData)) {
		return AT_ERROR;
	}

	char result[150] = {0};
	sprintf(result, "Prediction: %s\r\n", wData.weatherPredict);

	strcpy(value, result);
	return AT_OK;
}

char getWeather(char *value) {
	Weather wData = weatherData();
	if (isEmptyWeatherStruct(wData)) {
		return AT_ERROR;
	}

	char result[150] = {0};
	sprintf(result, "Temp: %.2f, Press: %.2f, Prediction: %s\r\n", wData.temp, wData.press, wData.weatherPredict);

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
	sprintf(result, "Interval: %lu\r\n", transferInterval);

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
		strcpy(result.weatherPredict, currentWeather.weatherPredict);
		osMutexRelease(weatherMutexHandle);
	}

	return result;
}

uint8_t isEmptyWeatherStruct(Weather wData) {
	return 0 == wData.temp && 0 == wData.press && strlen(wData.weatherPredict) == 0;
}

void Process_Command(char * command) {
	char result[150] = {0};
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
