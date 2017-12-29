#include "ModuleNoiseGen.h"
#include "minimo.h"
#include <avr/io.h>

// Parameters
#define PARAM_NOISEGEN_FREQ 0
#define PARAM_NOISEGEN_GRAIN 1
#define PARAM_NOISEGEN_COUNT 2

static bool control = 1;

// Volume input smoothing
#define NUM_READINGS_EXP 2
#define NUM_READINGS (1 << NUM_READINGS_EXP)
static byte readings[NUM_READINGS]; // Buffer of readings from the analog input
static int readIndex = 0;
static int total = 0;
static byte volumeModulation = 255;

// RNG
static unsigned long y32 = 1; //pattern length: 32 bit

// Declarations
void setGrainDensity(int pin);
void setFrequency(int pin);
int readExtInput(int pin);
byte xorshift32();


void module_noisegen_setup()
{
	cli();
	TCCR0A = (1<<WGM01);	// Clear Timer on Compare (CTC) with OCR0A
	TCCR0B = (1<<CS01) ;	// prescale by 8
	OCR0A = 0;				// frequencies down to 3921hz for a value of 255 https://www.easycalculation.com/engineering/electrical/avr-timer-calculator.php
	TIMSK |= (1 << OCIE0A);	// Enable Interrupt on compare with OCR0A
	sei();

	minimo_setParamCount(PARAM_NOISEGEN_COUNT);
	minimo_setParam(PARAM_NOISEGEN_FREQ, 255);
	minimo_setParam(PARAM_NOISEGEN_GRAIN, 255);

	minimo_prepareProcessInput();
}

void module_noisegen_loop()
{
	minimo_processInput();
	minimo_processParameters();
	OCR1C = minimo_getParam(PARAM_NOISEGEN_GRAIN);
	OCR0A = 255 - minimo_getParam(PARAM_NOISEGEN_FREQ);
	//volumeModulation = minimo_readInputSmooth(1);
	volumeModulation = minimo_analogInputs[MINIMOIN_AUDIO];
}

void module_noisegen_timer0_compa()
{
	OCR1B = (xorshift32() * volumeModulation) >> 8;
}

void module_noisegen_timer0_ovf()
{
}

void module_noisegen_pcint0()
{
	if (minimo_buttonDown)
	{
		control = !control;
		if (control) minimo_flashLed(1);
		else minimo_flashLed(2);
	}
}

byte xorshift32()
{
	y32 ^= (y32 << 7);
	y32 ^= (y32 >> 5);
	y32 ^= (y32 << 3);
	return y32;
}