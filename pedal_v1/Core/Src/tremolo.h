#ifndef TREMOLO_H
#define TREMOLO_H

#define _USE_MATH_DEFINES

#include <stdint.h>
#include <math.h>

typedef struct {
    float rate;
    float depth;
    
    float out;
} Tremolo

#endif