/*
MiniMO Waveform generation
*/

#pragma once

#include "Arduino.h"

extern byte minimo_wavetable[256];

void minimo_waveSine();
void minimo_waveTriangle();
void minimo_waveSquare();
void minimo_waveSawtooth();
void minimo_waveZero();
