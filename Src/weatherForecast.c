/*
 * weatherForecast.c
 *
 *  Created on: Nov 15, 2018
 *      Author: vitaliyvoronok
 */
#include "weatherForecast.h"
#include <string.h>
#include <math.h>

typedef enum {
	Falling,
	FallingSlow,
	FallingFast,
	Steady,
	Rising,
	RisingSlow,
	RisingFast,
} PressTrend;

char *forecastDescription(char code);
char *forecastType(char code);
PressTrend getTrend_text(float trend);
char result[100] = {0};

char *calcZambretti(float press, float fTrend, uint8_t month) {
	char description = '0';
	char weatherType = '0';

	PressTrend trend = getTrend_text(fTrend);

  // FALLING
  if (trend == Falling || trend == FallingSlow || trend == FallingFast) {
    double zambretti = 0.0009746 * press * press - 2.1068 * press + 1138.7019; //y = 0.0009746x^2-2.1068x+1138.7019
    // A Winter falling generally results in a Z value higher by 1 unit.
    if (month < 4 || month > 9) {zambretti = zambretti + 1;} // + makes the forecast worst, - better!

    int16_t value = (round(zambretti));
    switch (value) {
      case 1:  description = 'A'; weatherType = 'A'; break;       //Settled Fine
      case 2:  description = 'B'; weatherType = 'A'; break;       //Fine Weather
      case 3:  description = 'D'; weatherType = 'C'; break;  //Fine Becoming Less Settled
      case 4:  description = 'H'; weatherType = 'C'; break;  //Fairly Fine Showers Later
      case 5:  description = 'O'; weatherType = 'C'; break;  //Showery Becoming unsettled
      case 6:  description = 'R'; weatherType = 'E'; break;        //Unsettled, Rain later
      case 7:  description = 'U'; weatherType = 'E'; break;        //Rain at times, worse later
      case 8:  description = 'V'; weatherType = 'E'; break;        //Rain at times, becoming very unsettled
      case 9:  description = 'X'; weatherType = 'E'; break;        //Very Unsettled, Rain
      default: description = '0'; weatherType = '0'; break;
    }
  }
  // STEADY
  if (trend == Steady) {
    double zambretti = 138.24 - 0.133 * press; // y = 138.24-0.1331x
    int16_t value = (round(zambretti));
    switch (value) {
      case 1:  description = 'A'; weatherType = 'A'; break;       //Settled Fine
      case 2:  description = 'B'; weatherType = 'A'; break;       //Fine Weather
      case 3:  description = 'E'; weatherType = 'C'; break;  //Fine, Possibly showers
      case 4:  description = 'K'; weatherType = 'C'; break;  //Fairly Fine, Showers likely
      case 5:  description = 'N'; weatherType = 'C'; break;  //Showery Bright Intervals
      case 6:  description = 'P'; weatherType = 'D'; break;      //Changeable some rain
      case 7:  description = 'S'; weatherType = 'E'; break;        //Unsettled, rain at times
      case 8:  description = 'W'; weatherType = 'E'; break;        //Rain at Frequent Intervals
      case 9:  description = 'X'; weatherType = 'E'; break;        //Very Unsettled, Rain
      case 10: description = 'Z'; weatherType = 'F'; break;     //Stormy, much rain
      default: description = '0'; weatherType = '0'; break;
    }
  }
  // RISING
  if (trend == Rising || trend == RisingSlow || trend == RisingFast) {
    double zambretti = 142.57 - 0.1376 * press; //y = 142.57-0.1376x
    //A Summer rising, improves the prospects by 1 unit over a Winter rising
    if (month < 4 || month > 9) zambretti = zambretti + 1; // Increasing values makes the forecast worst!
    int16_t value = (round(zambretti));
    switch (value) {
      case 1:  description = 'A'; weatherType = 'A'; break;       //Settled Fine
      case 2:  description = 'B'; weatherType = 'A'; break;       //Fine Weather
      case 3:  description = 'C'; weatherType = 'B'; break; //Becoming Fine
      case 4:  description = 'F'; weatherType = 'B'; break; //Fairly Fine, Improving
      case 5:  description = 'G'; weatherType = 'C'; break;  //Fairly Fine, Possibly showers, early
      case 6:  description = 'I'; weatherType = 'C'; break;  //Showery Early, Improving
      case 7:  description = 'J'; weatherType = 'D'; break;      //Changeable, Improving
      case 8:  description = 'L'; weatherType = 'D'; break;      //Rather Unsettled Clearing Later
      case 9:  description = 'M'; weatherType = 'D'; break;      //Unsettled, Probably Improving
      case 10: description = 'Q'; weatherType = 'B'; break; //Unsettled, short fine Intervals
      case 11: description = 'T'; weatherType = 'D'; break;      //Very Unsettled, Finer at times
      case 12: description = 'Y'; weatherType = 'F'; break;     //Stormy, possibly improving
      case 13: description = 'Z'; weatherType = 'F'; break;     //Stormy, much rain
      default: description = '0'; weatherType = '0'; break;
    }
  }

  sprintf(result, "%s,\n%s", forecastType(weatherType), forecastDescription(description));

  return result;
}

char *forecastDescription(char code) {
  char *descriptor;

  switch (code) {
  case 'A': descriptor = "Settled Fine Weather"; break;
  case 'B': descriptor = "Fine Weather"; break;
  case 'C': descriptor = "Becoming Fine"; break;
  case 'D': descriptor = "Fine, Becoming Less Settled"; break;
  case 'E': descriptor = "Fine, Possibly showers"; break;
  case 'F': descriptor = "Fairly Fine, Improving"; break;
  case 'G': descriptor = "Fairly Fine, Possibly showers early"; break;
  case 'H': descriptor = "Fairly Fine, Showers Later"; break;
  case 'I': descriptor = "Showery Early, Improving"; break;
  case 'J': descriptor = "Changeable Improving"; break;
  case 'K': descriptor = "Fairly Fine, Showers likely"; break;
  case 'L': descriptor = "Rather Unsettled Clearing Later"; break;
  case 'M': descriptor = "Unsettled, Probably Improving"; break;
  case 'N': descriptor = "Showery Bright Intervals"; break;
  case 'O': descriptor = "Showery Becoming Unsettled"; break;
  case 'P': descriptor = "Changeable some rain"; break;
  case 'Q': descriptor = "Unsettled, short fine Intervals"; break;
  case 'R': descriptor = "Unsettled, Rain later"; break;
  case 'S': descriptor = "Unsettled, rain at times"; break;
  case 'T': descriptor = "Very Unsettled, Finer at times"; break;
  case 'U': descriptor = "Rain at times, Worse later"; break;
  case 'V': descriptor = "Rain at times, becoming very unsettled"; break;
  case 'W': descriptor = "Rain at Frequent Intervals"; break;
  case 'X': descriptor = "Very Unsettled, Rain"; break;
  case 'Y': descriptor = "Stormy, possibly improving"; break;
  case 'Z': descriptor = "Stormy, much rain"; break;
   default: descriptor = "Unknown"; break;
  }

  return descriptor;
}

char *forecastType(char code) {
  char *descriptor;

  switch (code) {
  case 'A': descriptor = "Sunny"; break;
  case 'B': descriptor = "Mostly Sunny"; break;
  case 'C': descriptor = "Expectrain"; break;
  case 'D': descriptor = "Cloudy"; break;
  case 'E': descriptor = "Rain"; break;
  case 'F': descriptor = "Storms"; break;
   default: descriptor = "Unknown"; break;
  }

  return descriptor;
}

PressTrend getTrend_text(float trend) {
	PressTrend result = Steady; // Default weather state

	if      (trend > 3.5)                    {result = RisingFast;}
	else if (trend > 1.5   && trend <= 3.5)  {result = Rising;}
	else if (trend > 0.25  && trend <= 1.5)  {result = RisingSlow;}
	else if (trend > -0.25 && trend < 0.25)  {result = Steady;}
	else if (trend >= -1.5 && trend < -0.25) {result = FallingSlow;}
	else if (trend >= -3.5 && trend < -1.5)  {result = Falling;}
	else if (trend <= -3.5)                  {result = FallingFast;}

	return result;
}
