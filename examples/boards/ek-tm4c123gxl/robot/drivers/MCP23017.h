//***********************************************
// Driver for MCP23017 gpio expander.
// autor: Marcin Gozdziewski
//***********************************************

#ifndef MCP23017_H
#define MCP23017_H


#include <stdbool.h>
#include <stdint.h>
#include <I2CWrapper.h>

//******************************
// API declarations
//******************************

#define GPIOEXP_PORTA	1
#define GPIOEXP_PORTB	2

#define GPIOEXP_PIN0	0x00
#define GPIOEXP_PIN1	0x02
#define GPIOEXP_PIN2	0x04
#define GPIOEXP_PIN3	0x08
#define GPIOEXP_PIN4	0x10
#define GPIOEXP_PIN5	0x20
#define GPIOEXP_PIN6	0x40
#define GPIOEXP_PIN7	0x80


// struct for storing single expander config

typedef struct
{
	I2CComInstance* i2cInstance;
	uint8_t hwAddress;
	// Don't modify below fields
	uint8_t	outPinsA,
			outPinsB,
			outPinsStateA,
			outPinsStateB;
} GpioExpander;

void GpioExpInit(GpioExpander* gpioExp);
bool GpioExpSetPinDirIn(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpSetPinDirOut(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpIntOnChangePrevValEnable(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpIntOnChangeDefValEnable(GpioExpander* gpioExp, uint8_t port, uint8_t pins, uint8_t pinsDefVals);
bool GpioExpIntOnChangeDisable(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpPullupEnable(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpPullupDisable(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpGetIntFlag(GpioExpander* gpioExp, uint8_t port, uint8_t* flagVal);
bool GpioExpIntReadPort(GpioExpander* gpioExp, uint8_t port, uint8_t* portVal);
bool GpioExpReadPort(GpioExpander* gpioExp, uint8_t port, uint8_t* portVal);
bool GpioExpSetPin(GpioExpander* gpioExp, uint8_t port, uint8_t pins);
bool GpioExpClearPin(GpioExpander* gpioExp, uint8_t port, uint8_t pins);



#endif // MCP23017_H
