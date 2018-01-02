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

// MiniMO inputs
#define MINIMOIN_AUDIO 0	// Input without knob
#define MINIMOIN_CONTROL 1	// Input connected to the knob

// MiniMO Module types
#define MINIMOMOD_DCO 0
#define MINIMOMOD_NOISEGEN 1
#define MINIMOMOD_SEQUENCER 2
#define MINIMOMOD_COUNT 3

// Analog inputs
extern byte minimo_analogInputs[2];

// Shared global data
extern volatile bool minimo_buttonDown;
extern bool minimo_calibrating;

// Button input state
extern bool minimo_buttonProcessEnabled;
extern unsigned char minimo_button_clickCount;
extern bool minimo_button_longClick;

// Parameters controlled by the knob input
extern byte minimo_paramIndex;
extern byte minimo_paramCount;
extern byte minimo_parameters[8];
extern bool minimo_paramInSync;

// Calibration
void minimo_loadCalibration();
void minimo_calibrateInput(int pin);

// Input processing
void minimo_prepareProcessInput();
void minimo_processInput();

// Input reading
int minimo_mapCalibratedInput(int value, int outputMin, int outputMax);
int minimo_readInputSmooth(int pin);

// Knob parameters
void minimo_setParamCount(byte count);
void minimo_setCurrentParam(byte index);
void minimo_cycleCurrentParam(); // Move to the next parameter
void minimo_setParam(byte index, byte value);
byte minimo_getParam(byte index);
void minimo_processParameters(); // Updates the parameter data

// Inline implementations

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

inline void minimo_setParamCount(byte count)
{
	minimo_paramCount = count;
}

inline void minimo_setCurrentParam(byte index)
{
	if (minimo_paramIndex != index)
	{
		minimo_paramIndex = index;
		minimo_paramInSync = false;
	}
}

inline void minimo_cycleCurrentParam()
{
	++minimo_paramIndex;
	if (minimo_paramIndex == minimo_paramCount)
	{
		minimo_paramIndex = 0;
	}
	minimo_paramInSync = false;
}

inline void minimo_setParam(byte index, byte value)
{
	minimo_parameters[index] = value;
}

inline byte minimo_getParam(byte index)
{
	return minimo_parameters[index];
}

inline void minimo_processParameters()
{
	if (!minimo_paramInSync)
	{
		byte lastValue = minimo_parameters[minimo_paramIndex];
		byte currValue = minimo_analogInputs[MINIMOIN_CONTROL];
		minimo_paramInSync = (lastValue == currValue);
	}
	else
	{
		minimo_parameters[minimo_paramIndex] = minimo_analogInputs[MINIMOIN_CONTROL];
	}
}
