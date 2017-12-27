#include "wavegen.h"

byte minimo_wavetable[256];

// Sine table
const byte PROGMEM sinetable[128] =
{
	  0,   0,   0,   0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,
	 10,  11,  12,  14,  15,  17,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,
	 37,  40,  42,  44,  47,  49,  52,  54,  57,  59,  62,  65,  67,  70,  73,  76,
	 79,  82,  85,  88,  90,  93,  97, 100, 103, 106, 109, 112, 115, 118, 121, 124,
	128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
	176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
	218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
	245, 246, 248, 249, 250, 250, 251, 252, 253, 253, 254, 254, 254, 255, 255, 255
};

void minimo_waveSine()
{
	for (int i = 0; i < 128; ++i)
	{
		minimo_wavetable[i] = pgm_read_byte_near(sinetable + i);
	}

	minimo_wavetable[128] = 255;

	for (int i = 129; i < 256; ++i) 
	{
		minimo_wavetable[i] = minimo_wavetable[256 - i];
	}
}

void minimo_waveTriangle()
{
	byte value = 0;
	for (int i = 0; i < 128; ++i)
	{
		minimo_wavetable[i] = value;
		value += 2;
	}
	for (int i = 128; i < 256; ++i) 
	{
		minimo_wavetable[i] = value;
		value -= 2;
	}
}

void minimo_waveSquare()
{
	for (int i = 0; i < 128; ++i)
	{
		minimo_wavetable[i] = 255;
	}
	for (int i = 128; i < 256; ++i) 
	{
		minimo_wavetable[i] = 1;
	}
}

void minimo_waveSawtooth()
{
	for (int i = 0; i < 256; ++i)
	{
    	minimo_wavetable[i] = i;
  	}
}

void minimo_waveZero()
{
	for (int i = 0; i < 256; ++i)
	{
		minimo_wavetable[i] = 1;
	}
}