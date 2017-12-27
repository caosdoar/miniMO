#include "ModuleDCO.h"
#include "minimo.h"
#include "wavegen.h"

// Sensor reading
static int sensorValue = 0;
static int sensorMin;
static int sensorMax;
static bool sensorCalibrating = false;

// Volume data
static bool volumeInSync = true;
static byte volumeSyncValue = 7;
static byte volumeFromKnob = 0;
static byte volumeFromInput = 0;
static byte volume = 0;

// Frequency data
static bool freqInSync = true;
static byte freqSyncValue = 255;
static int frequency = 0;
static byte freqRangeIndex = 1;
static int freqRangeMin = 0;
static int freqRangeMax = 0;

static unsigned int phase = 0;
static byte waveIndex = 0;

void dco_processButton();
void dco_setVolume(int pin);
void dco_setFrequency(int pin);
void dco_setFrequencyRange(int rangeIndex);
void dco_setWave(int wave);

void module_dco_setup()
{
	cli();
	//Timer Interrupt Generation -timer 0
	TCCR0A = (1 << WGM01) | (1 << WGM00);	// fast PWM
	TCCR0B = (1 << CS00);					// no prescale
	TIMSK  |= (1 << TOIE0);					// Enable Interrupt on overflow
	sei();

	volumeFromKnob = 255;
	volumeFromInput = 255;

	freqRangeIndex = 1;
	dco_setFrequencyRange(freqRangeIndex);
  	frequency = 1830;

  	waveIndex = 0;
  	dco_setWave(waveIndex);

  	minimo_buttonProcessEnabled = true;
}

void module_dco_loop()
{
	dco_processButton();

	if (minimo_buttonDown)
	{
		dco_setVolume(MINIMOPIN_IN0);
	}
	else
	{
		dco_setFrequency(MINIMOPIN_IN0);
	}
	
	volumeFromInput = minimo_readInputSmooth(1);
	volume = (volumeFromKnob * volumeFromInput) >> 8;
}

void module_dco_timer0_compa()
{
}

void module_dco_timer0_ovf()
{
	byte sample = (minimo_wavetable[phase >> 8] * volume) >> 8;
	phase += frequency;
	OCR1B = sample;
}

void module_dco_pcint0()
{
}

void dco_processButton()
{
	if (!minimo_button_longClick)
	{
		if (minimo_button_clickCount == 1)
		{
			++waveIndex;
			if (waveIndex == 5) waveIndex = 0;
			dco_setWave(waveIndex);
		}
		else if (minimo_button_clickCount == 2)
		{
			++freqRangeIndex;
			if (freqRangeIndex == 3) freqRangeIndex = 0;
			dco_setFrequencyRange(freqRangeIndex);
		}
		else if (minimo_button_clickCount == 3)
		{
			minimo_calibrateInput(MINIMOPIN_IN0);
		}
	}
	else
	{

	}
	/*while (minimo_buttonDown == 1)
	{
		++buttonDelay;
		_delay_ms(10);
		if (button_delay > 10 & !buttonDoubleClick)
		{
			buttonLongPress = true;
			minimo_flashLed(1);
			setVolume(MINIMOPIN_IN0);
		}
	}

	buttonDoubleClick = false;
	if (buttonDelay > 0)
	{
		bool hold = true;
		while (hold)
		{
			bool prevInputValue = minimo_buttonDown;
			_delay_ms(1);
			button_delay
		}
	}*/
}

void dco_setVolume(int pin)
{
	freqInSync = false;

	byte value = analogRead(pin) >> 2;
	if (!volumeInSync)
	{
		if (value == volumeSyncValue)
		{
			volumeInSync = true;
		}
	}
	else
	{
		volumeSyncValue = value;
		volumeFromKnob = value;
	}
}

void dco_setFrequency(int pin)
{
	volumeInSync = false;

	byte value = analogRead(pin) >> 2;
	if (!freqInSync)
	{
		if (value == freqSyncValue)
		{
			freqInSync = true;
		}
	}
	else
	{
		freqSyncValue = value;
		frequency = minimo_mapCalibratedInput(value, freqRangeMin, freqRangeMax);
	}
}

void dco_setFrequencyRange(int rangeIndex)
{
	switch (rangeIndex)
	{
		case 0: // A1
			freqRangeMin = 1;
			freqRangeMax = 114;
			break;

		case 1: // A5
			freqRangeMin = 114;
			freqRangeMax = 1830;
			break;

		case 2: // A6
			freqRangeMin = 1830;
			freqRangeMax = 3660;
			break;
	}
}

void dco_setWave(int wave)
{
	switch (wave)
	{
		case 0: minimo_waveSine(); break;
		case 1: minimo_waveTriangle(); break;
		case 2: minimo_waveSquare(); break;
		case 3: minimo_waveSawtooth(); break;
		case 4: minimo_waveZero(); break;
	}
}
