#include "overdrive.h"
#include <stdint.h>

void Overdrive_Init(Overdrive *od, float samplingFreqHz, float hpfCutoffFreqHz, float lpfCutoffFreqHz, float odPreGain) {
    od->T = 1.0f / samplingFreqHz;
    od->preGain = odPreGain;
    od->threshold = 1.0f/3.0f;
    // Input lpf
    od->hpfInpBufIn[0] = 0.0f; od->hpfInpBufIn[1] = 0.0f;
    od->hpfInpBufOut[0] = 0.0f; od->hpfInpBufOut[1] = 0.0f;
    od->hpfInpWcT = 2.0f * M_PI * hpfCutoffFreqHz * od->T;
    od->hpfInpOut = 0.0f;

    // Output lpf
    od->lpfOutWcT = 2.0f * M_PI * lpfCutoffFreqHz * od->T;
    od->lpfOutDamp = 1.0f;

    od->Q = -0.5;

}


float Overdrive_Update(Overdrive *od, float inp) {
	// Get new sample and push everything down by one spot
	// First order input buffer
	od->hpfInpBufIn[1] = od->hpfInpBufIn[0];
	od->hpfInpBufIn[0] = inp;

	// Second order output buffer
	od->hpfInpBufOut[1] = od->hpfInpBufOut[0]; // Shift down by one
	// Do the IIR filter math
	od->hpfInpBufOut[0] = (2.0f * (od->hpfInpBufIn[0] - od->hpfInpBufIn[1]) + (2.0f - od->hpfInpWcT) * od->hpfInpBufOut[1])/(2.0f + od->hpfInpWcT);
	od->hpfInpOut = od->hpfInpBufOut[0];
    
    // Overdrive
    float clipIn = od->preGain * od->hpfInpOut;

//     Symmetrical clipping

    float absClipIn = fabs(clipIn);
    float signClipIn = (clipIn >= 0.0f) ? 1.0f : -1.0f;
    float clipOut = 0.0f;

    // If within threshold, amplify by 2
    if (absClipIn < od->threshold) {
        clipOut = 2.0f * clipIn;
    // If over threshold, but below twice threshold, soft clip
    } else if (absClipIn >= od->threshold && absClipIn < (2.0f * od->threshold)) {
        clipOut = signClipIn * (3.0f - (2.0f - 3.0f*absClipIn)*(2.0f - 3.0f*absClipIn)) / 3.0f;
    // If entirely out of threshold, clip the signal to 2/3
    } else {
        clipOut = signClipIn * 2.0f * od->threshold;
    }

//    // Asym clipping
//    static const float d = 8.0f;
//
//    float clipOut = od->Q / (1.0f - expf(d*od->Q));
//
//    if (fabs(clipIn - od->Q) >= 0.00001f) {
//    	clipOut += (clipIn - od->Q)/(1.0f - expf(-d*(clipIn - od->Q)));
//    }
    
    // Lowpass filter the output 3rd degree
    od->lpfOutBufIn[2] = od->lpfOutBufIn[1];
    od->lpfOutBufIn[1] = od->lpfOutBufIn[0];
    od->lpfOutBufIn[0] = clipOut;

    od->lpfOutBufOut[2] = od->lpfOutBufOut[1];
    od->lpfOutBufOut[1] = od->lpfOutBufOut[0];
    od->lpfOutBufOut[0] = od->lpfOutWcT * od->lpfOutWcT * (od->lpfOutBufIn[0] + 2.0f * od->lpfOutBufIn[1] + od->lpfOutBufIn[2])
                        - 2.0f * (od->lpfOutWcT * od->lpfOutWcT - 4.0f) * od->lpfOutBufOut[1]
                        - (4.0f - 4.0f * od->lpfOutDamp * od->lpfOutWcT + od->lpfOutWcT * od->lpfOutWcT) * od->lpfOutBufOut[2];

    od->lpfOutBufOut[0] /= (4.0f + 4.0f * od->lpfOutDamp * od->lpfOutWcT + od->lpfOutWcT * od->lpfOutWcT);

    od->lpfOutOut = od->lpfOutBufOut[0];

    od->out = od->lpfOutOut;

    // Ensure the signal is within -1f to 1f range
    if (od->out > 1.0f) {
    	od->out = 1.0f;
    } else if (od->out < -1.0f) {
    	od->out = -1.0f;
    }

    return od->out;
}
