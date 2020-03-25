#ifndef _tune_h_
#define _tune_h_
#include "mbed.h"
#include "bitcoin.h"
#include "motor.h"

#define C 0
#define D 2
#define E 4
#define F 5
#define G 7
#define A 9
#define B 11


extern char tune[49];
extern Thread melody;

extern void note_extraction();
extern void playMelody();
#endif