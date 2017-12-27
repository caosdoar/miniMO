#include "minimo.h"
#include <avr/io.h>
#include <avr/eeprom.h>

volatile bool minimo_buttonDown;

bool minimo_calibrating = false;
static int minimo_calibrationMin = 0;
static int minimo_calibrationMax = 0;

bool minimo_buttonProcessEnabled = false;
unsigned char minimo_button_clickCount = 0;
bool minimo_button_longClick = false;

#define NUM_READINGS_EXP 2
#define NUM_READINGS (1 << NUM_READINGS_EXP)
static byte readings[NUM_READINGS]; // Buffer of readings from the analog input
static int readIndex = 0;
static int readTotal = 0;

int minimo_mapCalibratedInput(int value, int outputMin, int outputMax)
{
	return (value - minimo_calibrationMin) * (outputMax - outputMin) / (minimo_calibrationMax - minimo_calibrationMin) + outputMin;
}

void minimo_loadCalibration()
{
	minimo_calibrationMin = eeprom_read_word((uint16_t*)MINIMOMEM_SENSOR_MIN);
	minimo_calibrationMax = eeprom_read_word((uint16_t*)MINIMOMEM_SENSOR_MAX);
	if (minimo_calibrationMax == 0) minimo_calibrationMax = 1023;
	// TEST
	minimo_calibrationMin = 0;
	minimo_calibrationMax = 1023;
}

void minimo_calibrateInput(int pin)
{
	cli();

	minimo_calibrating = true;
	TIMSK = (0 << TOIE0);

	int firstRead = analogRead(pin);
	int delta = 0;
	int value = 0;
	minimo_calibrationMin = 0;
	minimo_calibrationMax = 1023;
	
	while (delta < 5)
	{
		value = analogRead(pin);
		delta = abs(value - firstRead);
	}

	int inRangeReads = 0;
	while (inRangeReads < 200)
	{
		value = analogRead(pin);
		if (value < minimo_calibrationMin) minimo_calibrationMin = value;
		else if (value > minimo_calibrationMax) minimo_calibrationMax = value;
		else ++inRangeReads;
		_delay_ms(10);
	}

	eeprom_update_word((uint16_t*)MINIMOMEM_SENSOR_MIN, minimo_calibrationMin);
	eeprom_update_word((uint16_t*)MINIMOMEM_SENSOR_MAX, minimo_calibrationMax);

	TIMSK = (1 << TOIE0);
	minimo_calibrating = false;

	sei();
}

/*
Reads the next input and returns an average
*/
int minimo_readInputSmooth(int pin)
{
	readTotal = readTotal - readings[readIndex];
	byte value = analogRead(pin) >> 2;
	readings[readIndex] = value;
	readTotal = readTotal + value;
	++readIndex;
	if (readIndex >= NUM_READINGS) readIndex = 0;
	return readTotal >> NUM_READINGS_EXP;
}
