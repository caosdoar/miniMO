/*
MiniMO Uber shared functionality
*/

#pragma once

#include "Arduino.h"
#include <util/delay.h>

// MiniMO memory addresses
#define MINIMOMEM_MODULE_TYPE 1
#define MINIMOMEM_SENSOR_MIN 3
#define MINIMOMEM_SENSOR_MAX 5

// MiniMO pins
#define MINIMOPIN_LED 0
#define MINIMOPIN_BUTTON 1
#define MINIMOPIN_IN0 3
#define MINIMOPIN_IN1 2

// MiniMO Module types
#define MINIMOMOD_DCO 0
#define MINIMOMOD_NOISEGEN 1
#define MINIMOMOD_COUNT 2

// Shared global data
extern volatile bool minimo_buttonDown;
extern bool minimo_calibrating;

// Button input state
extern bool minimo_buttonProcessEnabled;
extern unsigned char minimo_button_clickCount;
extern bool minimo_button_longClick;

void minimo_loadCalibration();
void minimo_calibrateInput(int pin);



int minimo_mapCalibratedInput(int value, int outputMin, int outputMax);
int minimo_readInputSmooth(int pin);

/**
Flashes a led with the standard gap
*/
inline void minimo_flashLed(int times)
{
	for (int i = 0; i < times; ++i)
	{
		digitalWrite(MINIMOPIN_LED, HIGH);
		_delay_ms(100);
		digitalWrite(MINIMOPIN_LED, LOW);
		_delay_ms(100);
	}
}

inline void minimo_flashLedSlow(int times)
{
	for (int i = 0; i < times; ++i)
	{
		digitalWrite(MINIMOPIN_LED, HIGH);
		_delay_ms(300);
		digitalWrite(MINIMOPIN_LED, LOW);
		_delay_ms(300);
	}
}

inline void minimo_flashLedFast(int times)
{
	for (int i = 0; i < times; ++i)
	{
		digitalWrite(MINIMOPIN_LED, HIGH);
		_delay_ms(50);
		digitalWrite(MINIMOPIN_LED, LOW);
		_delay_ms(50);
	}
}

