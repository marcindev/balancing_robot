//***********************************************
// Encoder driver.
// autor: Marcin Gozdziewski
//***********************************************
#ifndef ENCODER_H
#define ENCODER_H
#include <stdbool.h>
#include <stdint.h>
#include "MCP23017.h"

#define ENC_CHANNEL_1	0
#define ENC_CHANNEL_2	1

typedef struct
{
	GpioExpander* gpioExpander;
	uint8_t ch1_pin;
	uint8_t ch2_pin;
	// not change
	int64_t rotationsCounter; // counter of halves of rotations
	int8_t statesCounter;
	uint8_t prevState;
}EncoderInstance;

// Finite State Machine
// States:
#define BOTH_CHANNELS_LOW		0
#define CHANNEL1_HIGH			1
#define CHANNEL2_HIGH			2
#define BOTH_CHANNELS_HIGH		3

bool initEncoder(EncoderInstance* encoder);
void doEncoderJob();


#endif // ENCODER_H
