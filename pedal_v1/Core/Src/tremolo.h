#ifndef TREMOLO_H
#define TREMOLO_H

#define _USE_MATH_DEFINES

#include <stdint.h>
#include <math.h>

typedef struct {
	float T;
    float rate;		// LFO Freq (Hz)
    float depth;	// Tremolo depth, 0.0 to 1.0
    float phase;	// Phase of modulation wave
    
    float out;		// Output in -1f to 1f form
} Tremolo;

void Tremolo_Init(Tremolo *tr, float samplingFreqHz, float rate, float depth);
//void Tremolo_Set_Rate();
//void Tremolo_Set_Depth();
float Tremolo_Update(Tremolo *tr, float inp);

#endif
