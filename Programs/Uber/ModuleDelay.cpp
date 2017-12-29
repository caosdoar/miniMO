#include "ModuleDelay.h"
#include "minimo.h"
#include "wavegen.h"
#include <avr/io.h>

#define PARAM_DELAY_FEEDBACK 0
#define PARAM_DELAY_BUFFLEN 1
#define PARAM_DELAY_LOOPLEN 2

static int delayIndex = 0;

static byte paramIndex = 0;
static bool paramInSync = true;
static byte parameters[] = { 200, 255, 0 };

void module_delay_setup()
{
	cli();
	//Timer Interrupt Generation -timer 0
	TCCR0A = (1 << WGM01) | (1 << WGM00);	// fast PWM
	TCCR0B = (1 << CS00);					// no prescale
	TIMSK  |= (1 << TOIE0);					// Enable Interrupt on overflow
	sei();
}

void module_delay_loop()
{
	byte sample = minimo_wavetable[delayIndex];
	OCR1B = sample;
	//int iw1 = 127 - sample;
	//int iw2 = 127 - audioInput;

	// TODO: ADC for later...
}

void module_delay_timer0_compa()
{
}

void module_delay_timer0_ovf()
{
}

void module_delay_pcint0()
{
}

void setParameter()
{
	/*byte value = 
	if (paramInSync)
	{
		parameters[paramIndex] = 
	}*/
}