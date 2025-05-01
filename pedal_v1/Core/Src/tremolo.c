#include "tremolo.h"
#include <stdint.h>

void Tremolo_Init(Tremolo *tr, float samplingFreqHz, float rate, float depth) {
	tr->T = 1.0f / samplingFreqHz;
	tr->rate = rate;
	tr->depth = depth;
	tr->phase = 0.0f;
	tr->out = 0.0f;

//	for (int i = 0; i < LFO_TABLE_SIZE; i++) {
//		tr->lfo_table[i] = 0.5f * tr->depth * sinf(2.0f * M_PI * (float)i / LFO_TABLE_SIZE) + 0.5f;
//	}
}

float Tremolo_Update(Tremolo *tr, float inp) {
	float lfo = 0.5f*tr->depth*sinf(2.0f*M_PI*tr->rate*tr->phase) + (1.0f - 0.5f*tr->depth);
	tr->out = inp*lfo;
	tr->phase += tr->T;

	if (tr->phase*tr->rate >= 1.0f) {
		tr->phase = 0.0f;
	}

	// Ensure the signal is within -1f to 1f range
	if (tr->out > 1.0f) {
		tr->out = 1.0f;
	} else if (tr->out < -1.0f) {
		tr->out = -1.0f;
	}

	return tr->out;
}

void Tremolo_Set_Rate(Tremolo *tr, float rate) {
	tr->rate = rate;
}

void Tremolo_Set_Depth(Tremolo *tr, float depth) {
	tr->depth = depth;
}
