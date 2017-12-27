#include <avr/io.h>
#include <avr/eeprom.h>
//#include <util/delay.h>

#include "minimo.h"

// Modules
#include "ModuleDCO.h"
#include "ModuleNoiseGen.h"

#define MINIMO_CLICK_GAP_MILLIS 200
#define MINIMO_CLICK_LONG_MILLIS 400
#define MINIMO_CLICK_SAMPLE_TIME 10
#define MINIMO_CLICK_GAP_COUNT (MINIMO_CLICK_GAP_MILLIS / MINIMO_CLICK_SAMPLE_TIME)
#define MINIMO_CLICK_LONG_COUNT (MINIMO_CLICK_LONG_MILLIS / MINIMO_CLICK_SAMPLE_TIME)

static int minimo_moduleType = 0;


void setup()
{
	setupMinimo();
	setupModule();
	setupSetReady();
}

void loop()
{
	if (!minimo_calibrating)
	{
		if (minimo_buttonProcessEnabled) processButton();

		switch (minimo_moduleType)
		{
			case MINIMOMOD_DCO: module_dco_loop(); break;
			case MINIMOMOD_NOISEGEN: module_noisegen_loop(); break;
		}
	}
}

ISR(TIMER0_COMPA_vect)
{
	switch (minimo_moduleType)
	{
		case MINIMOMOD_DCO: module_dco_timer0_compa(); break;
		case MINIMOMOD_NOISEGEN: module_noisegen_timer0_compa(); break;
	}
}

ISR(TIMER0_OVF_vect)
{
	switch (minimo_moduleType)
	{
		case MINIMOMOD_DCO: module_dco_timer0_ovf(); break;
		case MINIMOMOD_NOISEGEN: module_noisegen_timer0_ovf(); break;
	}
}

ISR(PCINT0_vect)
{
	minimo_buttonDown = PINB & 0x02;

	switch (minimo_moduleType)
	{
		case MINIMOMOD_DCO: module_dco_pcint0(); break;
		case MINIMOMOD_NOISEGEN: module_noisegen_pcint0(); break;
	}
}

void setupMinimo()
{
	PRR = (1 << PRUSI);                  // Disable USI to save power as we are not using it
	DIDR0 = (1 << ADC1D) | (1 << ADC3D); // PB2,PB3  //disable digital input in pins that do analog conversion

	pinMode(0, OUTPUT); // LED
	pinMode(1, INPUT);  // digital input (push button)
	pinMode(2, INPUT);  // analog- amplitude input (external input 2)
	pinMode(3, INPUT);  // analog- freq input (knob plus external input 1)
	pinMode(4, OUTPUT); // timer 1 in digital output 4 - outs 1 and 2

	checkVoltage();
	selectModule();

	minimo_loadCalibration();

	// set clock source for PWM -datasheet p94
	PLLCSR |= (1 << PLLE);					// Enable PLL (64 MHz)
	_delay_us(100);							// Wait for a steady state
	while (!(PLLCSR & (1 << PLOCK)));		// Ensure PLL lock
	PLLCSR |= (1 << PCKE);					// Enable PLL as clock source for timer 1
	
	cli();									// Interrupts OFF (disable interrupts globally)

	// PWM Generation -timer 1
	GTCCR  = (1 << PWM1B) | (1 << COM1B1);	// PWM, output on pb4, compare with OCR1B (see interrupt below), reset on match with OCR1C
	OCR1C  = 0xff;							// 255
	TCCR1  = (1 << CS10);					// no prescale

	//Timer Interrupt Generation -timer 0
	//TCCR0A = (1 << WGM01) | (1 << WGM00); // fast PWM
	//TCCR0B = (1 << CS00);                // no prescale
	//TIMSK = (1 << TOIE0);                // Enable Interrupt on overflow

	// Pin Change Interrupt
	GIMSK |= (1 << PCIE);					// Enable 
	PCMSK |= (1 << PCINT1);					// on pin 1

	sei();									// Interrupts ON (enable interrupts globally)
}

void setupSetReady()
{
	//go for it!
	//digitalWrite(MINIMOPIN_LED, HIGH);		// turn LED ON
}

void setupModule()
{
	switch (minimo_moduleType)
	{
		case MINIMOMOD_DCO: module_dco_setup(); break;
		case MINIMOMOD_NOISEGEN: module_noisegen_setup(); break;
	}
}

/**
Module selection
*/
void selectModule()
{
	minimo_moduleType = eeprom_read_word((uint16_t*)MINIMOMEM_MODULE_TYPE);

	byte steps = 0;
	while (digitalRead(MINIMOPIN_BUTTON) == HIGH)
	{
		_delay_ms(300);
		if (digitalRead(MINIMOPIN_BUTTON) == HIGH) 
		{
			++steps;
			minimo_flashLed(1);
		}
	}

	if (steps > 0)
	{
		minimo_moduleType = steps - 1;
		while (minimo_moduleType > MINIMOMOD_COUNT) minimo_moduleType -= MINIMOMOD_COUNT;
		eeprom_update_word((uint16_t*)MINIMOMEM_MODULE_TYPE, minimo_moduleType);
		minimo_flashLed(steps);
	}

	//minimo_moduleType = MINIMOMOD_DCO;
}

/**
Checks if there is enough voltage to run the module
*/
void checkVoltage()
{
	/*
	voltage from 255 to 0; 46 is (approx)5v, 94 is 2.8, 104-106 is 2.5
	we measure a fixed value of 1.1 against Vcc, so the lower the measurement, the higher Vcc
	*/
	ADMUX = (0 << REFS1)|(0 << REFS0);		// Vcc as reference
	ADMUX |= (1 << ADLAR);					// Left adjust result (8 bit conversion stored in ADCH)
	ADMUX |= (1 << MUX3) | (1 << MUX2);		// 1.1v input
	_delay_ms(250);							// Wait for Vref to settle
	ADCSRA |= (1 << ADSC);					// Start conversion
	while (bit_is_set(ADCSRA, ADSC));		// wait while measuring
	//if (ADCH > 103) minimo_flashLed(8);		// approx 2.6
	//else minimo_flashLedSlow(1);
	ADMUX = 0;								// reset multiplexer settings
}

void processButton()
{
	if (minimo_button_longClick)
	{
		if (digitalRead(MINIMOPIN_BUTTON) == LOW)
		{
			minimo_button_clickCount = 0;
			minimo_button_longClick = false;
		}
	}
	else
	{
		// Reset click state
		minimo_button_clickCount = 0;

		// Only process if the button is pressed at this time
		if(digitalRead(MINIMOPIN_BUTTON) == HIGH)
		{
			cli(); // Avoid interrupts so delays values are correct

			++minimo_button_clickCount;
			bool keep = true;
			while (keep)
			{
				// How long is pressed
				byte count = 0;
				while(digitalRead(MINIMOPIN_BUTTON) == HIGH)
				{
					_delay_ms(MINIMO_CLICK_SAMPLE_TIME);
					++count;
					if (count >= MINIMO_CLICK_LONG_COUNT)
					{
						// If there is a long click we stop
						minimo_button_longClick = true;
						break;
					}
				}

				if (!minimo_button_longClick)
				{
					// Wait for the next click
					keep = false;
					for (int i = 0; i < MINIMO_CLICK_GAP_COUNT; ++i)
					{
						_delay_ms(MINIMO_CLICK_SAMPLE_TIME);
						if (digitalRead(MINIMOPIN_BUTTON) == HIGH)
						{
							++minimo_button_clickCount;
							keep = true;
							break;
						}
					}
				}
				else
				{
					break;
				}
			}

			minimo_flashLed(1);

			sei();
		}
	}
}