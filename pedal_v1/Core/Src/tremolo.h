#ifndef TREMOLO_H
#define TREMOLO_H

#define _USE_MATH_DEFINES
//#define LFO_TABLE_SIZE 256

#include <stdint.h>
#include <math.h>

typedef struct {
	float T;
//	float lfo_table[LFO_TABLE_SIZE];
    float rate;		// LFO Freq (Hz)
    float depth;	// Tremolo depth, 0.0 to 1.0
    float phase;	// Phase of modulation wave
    
    float out;		// Output in -1f to 1f form
} Tremolo;

void Tremolo_Init(Tremolo *tr, float samplingFreqHz, float rate, float depth);
void Tremolo_Set_Rate(Tremolo *tr, float rate);
void Tremolo_Set_Depth(Tremolo *tr, float depth);
float Tremolo_Update(Tremolo *tr, float inp);

#endif
