/*
 * readSensors.c
 *
 *  Created on: Nov 20, 2018
 *      Author: vitaliyvoronok
 */

#include "cmsis_os.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "usart.h"
#include "lcd.h"
#include "BMP280.h"
#include "ringbuffer_dma.h"
#include <string.h>
#include "at_parser.h"
#include "weatherForecast.h"

#include "dataTypes.h"

#define QNH 1020

extern osMailQId mailWeatherHandle;
extern Weather currentWeather;
extern osMutexId weatherMutexHandle;

int8_t com_rslt;

double savePress[24] = {0};
uint8_t savePressSize = 24;
uint32_t hourInterval = 1000 * 60 * 60;
uint32_t lastPressTick = 0;

void savePressValue(double press);
float seaLevelPressure(double press, double altitude, double temp);

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
		double alt = BMP280_calculate_altitude(QNH * 100);

		weatherData = (Weather *) osMailAlloc(mailWeatherHandle, 5);

		if (HAL_GetTick() - lastPressTick > hourInterval) {
			double seaLevelPress = seaLevelPressure(press / 100, alt, temp);
			savePressValue(seaLevelPress);
			lastPressTick = HAL_GetTick();
		}

		double trend = savePress[23] - savePress[20];
		char *predict = calcZambretti(seaLevelPressure(press / 100, alt, temp), trend, 11);

		if (weatherData !=NULL) {
			weatherData->press = press / 1000.0;
			weatherData->temp = temp;
			strcpy(weatherData->weatherPredict, predict);
			osMailPut(mailWeatherHandle, weatherData);
		}

		status = osMutexWait(weatherMutexHandle, 20);
		if (status == osOK) {
			currentWeather.press = press;
			currentWeather.temp = temp;
			strcpy(currentWeather.weatherPredict, predict);
			osMutexRelease(weatherMutexHandle);
		}

		osDelay(20);
	}
	/* USER CODE END StartReadSensor */
}

void savePressValue(double press) {
	for (uint8_t i = 1; i < savePressSize; i++) {
		savePress[i-1] = savePress[i];
	}

	savePress[savePressSize - 1] = press;
}

float seaLevelPressure(double press, double altitude, double temp) {
  return press * pow(1 - 0.0065 * altitude / (temp + 0.0065 * altitude + 273.15), -5.275);
}
