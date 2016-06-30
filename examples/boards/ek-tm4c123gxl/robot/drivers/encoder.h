//***********************************************
// Encoder driver.
// autor: Marcin Gozdziewski
//***********************************************
#ifndef ENCODER_H
#define ENCODER_H
#include <stdbool.h>
#include <stdint.h>
#include "MCP23017.h"
#include "semphr.h"


#define ENCODER_LEFT        0x00
#define ENCODER_RIGHT       0x01

#define ENCODER_DIR_FWD     0
#define ENCODER_DIR_REV     1

#define ENCODER_SPEED_LESS      0
#define ENCODER_SPEED_GRATER    1

#define ENC_CHANNEL_1   0
#define ENC_CHANNEL_2   1

typedef struct
{
    GpioExpander* gpioExpander;
    uint8_t ch1_pin;
    uint8_t ch2_pin;
    // not change
    int64_t rotationsCounter;       // counter of halves of rotations
    int64_t prevRotationsCounter;   // for speed measurement
    uint64_t speed;                 // angular speed
    int8_t statesCounter;
    uint8_t prevState;
    xSemaphoreHandle rotationsCounterSem;
    xSemaphoreHandle speedSem;
}EncoderInstance;

// Finite State Machine
// States:
#define BOTH_CHANNELS_LOW       0
#define CHANNEL1_HIGH           1
#define CHANNEL2_HIGH           2
#define BOTH_CHANNELS_HIGH      3

bool initEncoder(EncoderInstance* encoder);
void doEncoderJob();
int64_t getRotationsCounter(EncoderInstance* encoder);
uint64_t getEncoderSpeed(EncoderInstance* encoder);
void measureEncoderSpeed(EncoderInstance* encoder, uint32_t period);


#endif // ENCODER_H
