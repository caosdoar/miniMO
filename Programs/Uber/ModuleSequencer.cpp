#include "ModuleSequencer.h"
#include "minimo.h"
#include "wavegen.h"

#define SEQ_MAX_STEPS 16
static byte steps_len[SEQ_MAX_STEPS] = {127, 0, 0, 0, 127, 63, 0, 0, 127, 63, 31, 0, 127, 63, 31, 15};
static unsigned int steps_freq[SEQ_MAX_STEPS] = {1830, 1830, 1830, 1830, 3660, 3660, 3660, 3660, 7320, 7320, 7320, 7320, 14640, 14640, 14640, 14640};

// About frequencies!
// The phase advances once in 256 cycles
// that is: 8Mhzs / 256 = 31250 advances
// A sine cycle is 256 sampes 
// and we divide phase by 256 to get the index sample so:
// frequency = 256 -> 122 hzs
// The formula is:
// frequency = hz * 256 / 122

// Useful macro to transform frequncies in hertzs to the phase advance number
#define HZ_TO_FREQ(x) (x * 256 / 122)

static unsigned int frequencies[16] = 
{  
	HZ_TO_FREQ(110), 
	HZ_TO_FREQ(147), 
	HZ_TO_FREQ(165), 
	HZ_TO_FREQ(196), 
	HZ_TO_FREQ(220), 
	HZ_TO_FREQ(294), 
	HZ_TO_FREQ(330), 
	HZ_TO_FREQ(392), 
	HZ_TO_FREQ(440), 
	HZ_TO_FREQ(587), 
	HZ_TO_FREQ(659), 
	HZ_TO_FREQ(880)
};

static bool editing = false;

// Signal data
static int frequency = 0;
static byte volume = 0;
static unsigned int phase = 0;

// Play data
static unsigned long step_counter = 0;
static byte tempo = 1;

// Edit data
static byte edit_step_index = 0;

void seq_loop_play();
void seq_loop_edit();

void module_seq_setup()
{
	minimo_prepareProcessInput();

	cli();
	//Timer Interrupt Generation -timer 0
	TCCR0A = (1 << WGM01) | (1 << WGM00);	// fast PWM
	TCCR0B = (1 << CS00);					// no prescale
	TIMSK  |= (1 << TOIE0);					// Enable Interrupt on overflow
	sei();

  	minimo_waveSine();

  	minimo_buttonProcessEnabled = true;
}

void module_seq_loop()
{
	minimo_processInput();

	if (!editing)
	{
		seq_loop_play();
	}
	else
	{
		seq_loop_edit();
	}
}

void module_seq_timer0_compa()
{
}

void module_seq_timer0_ovf()
{
	byte sample = (minimo_wavetable[phase >> 8] * volume) >> 8;
	phase += frequency;
	step_counter += tempo;
	OCR1B = sample;
}

void module_seq_pcint0()
{
}

void seq_loop_play()
{
	tempo = minimo_analogInputs[MINIMOIN_CONTROL];

	// 8Mhz >> 18 ~= 8 bpm
	byte step_index = (step_counter >> 18) & 0x0F;
	byte step_subindex = step_counter >> 10;
	byte len = steps_len[step_index];
	if (len > step_subindex)
	{
		volume = 255;
		frequency = steps_freq[step_index];
	}
	else
	{
		volume = 0;
	}

	// Input
	if (!minimo_button_longClick)
	{
		if (minimo_button_clickCount == 1)
		{
			// Go to edit mode
			edit_step_index = (step_counter >> 18) & 0x0F;
			frequency = steps_freq[edit_step_index];
			editing = true;
		}
	}
}

void seq_loop_edit()
{
	byte len = steps_len[edit_step_index];

	byte freq_index = minimo_analogInputs[MINIMOIN_CONTROL] >> 4;
	frequency = frequencies[freq_index];
	steps_freq[edit_step_index] = frequency;

	byte step_subindex = step_counter >> 10;
	if (len > step_subindex)
	{
		volume = 255;
	}
	else
	{
		volume = 0;
	}

	if (!minimo_button_longClick)
	{
		if (minimo_button_clickCount == 1)
		{
			if (len < 31) steps_len[edit_step_index] = 31;
			else if (len < 63) steps_len[edit_step_index] = 63;
			else if (len < 127) steps_len[edit_step_index] = 127;
			else if (len < 255) steps_len[edit_step_index] = 255;
			else steps_len[edit_step_index] = 0;
		}
		else if (minimo_button_clickCount == 2)
		{
			++edit_step_index;
			if (edit_step_index >= SEQ_MAX_STEPS) edit_step_index = 0;
		}
	}
	else
	{
		if (minimo_button_clickCount == 1)
		{
			editing = false;
		}
	}
}
