/*
 * updateDisplay.c
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

extern osMailQId mailWeatherHandle;

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
			LCD_Printf("Weather Predict  : %s                              \n", weatherData->weatherPredict);

			/* Free the cell in mail queue */
			osMailFree(mailWeatherHandle, weatherData);
		}
		osDelay(40);

	}
	/* USER CODE END StartUpdateDisplay */
}
