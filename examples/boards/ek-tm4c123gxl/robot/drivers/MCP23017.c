/* Driver for MCP23017 gpio expander.
 * autor: Marcin Gozdziewski
 */


#include <MCP23017.h>



// registers definitions for BANK0

#define GPIOEXP_BANK0_IODIRA	0x00	// I/O DIRECTION REGISTER
#define GPIOEXP_BANK0_IODIRB	0x01
#define GPIOEXP_BANK0_IPOLA		0x02	// INPUT POLARITY REGISTER
#define GPIOEXP_BANK0_IPOLB		0x03
#define GPIOEXP_BANK0_GPINTENA	0x04	// INTERRUPT-ON-CHANGE CONTROL REGISTER
#define GPIOEXP_BANK0_GPINTENB	0x05
#define GPIOEXP_BANK0_DEFVALA	0x06	// DEFAULT COMPARE REGISTER FOR INTERRUPT-ON-CHANGE
#define GPIOEXP_BANK0_DEFVALB	0x07
#define GPIOEXP_BANK0_INTCONA	0x08	// INTERRUPT CONTROL REGISTER
#define GPIOEXP_BANK0_INTCONB	0x09
#define GPIOEXP_BANK0_IOCON		0x0A	// IOCON – I/O EXPANDER CONFIGURATION REGISTER
#define GPIOEXP_BANK0_GPPUA		0x0C	// PULL-UP RESISTOR CONFIGURATION REGISTER
#define GPIOEXP_BANK0_GPPUB		0x0D
#define GPIOEXP_BANK0_INTFA		0x0E	// INTERRUPT FLAG REGISTER
#define GPIOEXP_BANK0_INTFB		0x0F
#define GPIOEXP_BANK0_INTCAPA	0x10	// INTERRUPT CAPTURE REGISTER
#define GPIOEXP_BANK0_INTCAPB	0x11
#define GPIOEXP_BANK0_GPIOA		0x12	// PORT REGISTER
#define GPIOEXP_BANK0_GPIOB		0x13
#define GPIOEXP_BANK0_OLATA		0x14	// OUTPUT LATCH REGISTER (OLAT)
#define GPIOEXP_BANK0_OLATB		0x15

// registers definitions for BANK1

#define GPIOEXP_BANK1_IODIRA	0x00	// I/O DIRECTION REGISTER
#define GPIOEXP_BANK1_IODIRB	0x10
#define GPIOEXP_BANK1_IPOLA		0x01	// INPUT POLARITY REGISTER
#define GPIOEXP_BANK1_IPOLB		0x11
#define GPIOEXP_BANK1_GPINTENA	0x02	// INTERRUPT-ON-CHANGE CONTROL REGISTER
#define GPIOEXP_BANK1_GPINTENB	0x12
#define GPIOEXP_BANK1_DEFVALA	0x03	// DEFAULT COMPARE REGISTER FOR INTERRUPT-ON-CHANGE
#define GPIOEXP_BANK1_DEFVALB	0x13
#define GPIOEXP_BANK1_INTCONA	0x04	// INTERRUPT CONTROL REGISTER
#define GPIOEXP_BANK1_INTCONB	0x14
#define GPIOEXP_BANK1_IOCON		0x05	// IOCON – I/O EXPANDER CONFIGURATION REGISTER
#define GPIOEXP_BANK1_GPPUA		0x06	// PULL-UP RESISTOR CONFIGURATION REGISTER
#define GPIOEXP_BANK1_GPPUB		0x16
#define GPIOEXP_BANK1_INTFA		0x07	// INTERRUPT FLAG REGISTER
#define GPIOEXP_BANK1_INTFB		0x17
#define GPIOEXP_BANK1_INTCAPA	0x08	// INTERRUPT CAPTURE REGISTER
#define GPIOEXP_BANK1_INTCAPB	0x18
#define GPIOEXP_BANK1_GPIOA		0x09	// PORT REGISTER
#define GPIOEXP_BANK1_GPIOB		0x19
#define GPIOEXP_BANK1_OLATA		0x0A	// OUTPUT LATCH REGISTER (OLAT)
#define GPIOEXP_BANK1_OLATB		0x1A

#define READ_MSG_SIZE	1
#define WRITE_MSG_SIZE	2


bool GpioExpGetCurrRegVal(GpioExpander* gpioExp, uint8_t regAddr, uint8_t* regVal);

void GpioExpInit(GpioExpander* gpioExp)
{
	if(!I2CComInitiated(gpioExp->i2cInstance))
		I2CComInit(gpioExp->i2cInstance);

	gpioExp->outPins = 0;
	gpioExp->outPinsState = 0;
}

bool GpioExpSetPinDirIn(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_IODIRA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_IODIRB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, &currRegVal))
		return false;

	msg[0] = regAddr;
	msg[1] = currRegVal | pins;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	gpioExp->outPins &= (~pins);

	return true;
}

bool GpioExpSetPinDirOut(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_IODIRA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_IODIRB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, &currRegVal))
		return false;

	msg[0] = regAddr;
	msg[1] = currRegVal & (~pins);

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	gpioExp->outPins = pins;

	return true;
}

bool GpioExpIntOnChangePrevValEnable(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t intEnAddr = 0,
			confIntAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		intEnAddr = GPIOEXP_BANK0_GPINTENA;
		confIntAddr = GPIOEXP_BANK0_INTCONA;
		break;
	case GPIOEXP_PORTB:
		intEnAddr = GPIOEXP_BANK0_GPINTENB;
		confIntAddr = GPIOEXP_BANK0_INTCONB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, confIntAddr, &currRegVal))
		return false;

	msg[0] = confIntAddr;
	msg[1] = currRegVal & (~pins);

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	if(!GpioExpGetCurrRegVal(gpioExp, intEnAddr, &currRegVal))
		return false;

	msg[0] = intEnAddr;
	msg[1] = currRegVal | pins;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	return true;
}

bool GpioExpIntOnChangeDefValEnable(GpioExpander* gpioExp, uint8_t port, uint8_t pins, uint8_t pinsDefVals)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t intEnAddr = 0,
			defValAddr = 0,
			confIntAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		intEnAddr = GPIOEXP_BANK0_GPINTENA;
		defValAddr = GPIOEXP_BANK0_DEFVALA;
		confIntAddr = GPIOEXP_BANK0_INTCONA;
		break;
	case GPIOEXP_PORTB:
		intEnAddr = GPIOEXP_BANK0_GPINTENB;
		defValAddr = GPIOEXP_BANK0_DEFVALB;
		confIntAddr = GPIOEXP_BANK0_INTCONB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, defValAddr, &currRegVal))
		return false;

	msg[0] = defValAddr;
	msg[1] = pinsDefVals;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;


	if(!GpioExpGetCurrRegVal(gpioExp, confIntAddr, &currRegVal))
		return false;

	msg[0] = confIntAddr;
	msg[1] = currRegVal | pins;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	if(!GpioExpGetCurrRegVal(gpioExp, intEnAddr, &currRegVal))
		return false;

	msg[0] = intEnAddr;
	msg[1] = currRegVal | pins;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	return true;
}

bool GpioExpIntOnChangeDisable(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t intEnAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		intEnAddr = GPIOEXP_BANK0_GPINTENA;
		break;
	case GPIOEXP_PORTB:
		intEnAddr = GPIOEXP_BANK0_GPINTENB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, intEnAddr, &currRegVal))
		return false;

	msg[0] = intEnAddr;
	msg[1] = currRegVal & (~pins);

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	return true;
}

bool GpioExpPullupEnable(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_GPPUA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_GPPUB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, &currRegVal))
		return false;

	msg[0] = regAddr;
	msg[1] = currRegVal | pins;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	return true;
}

bool GpioExpPullupDisable(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_GPPUA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_GPPUB;
		break;
	default:
		return false;
	}

	uint8_t currRegVal = 0;

	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, &currRegVal))
		return false;

	msg[0] = regAddr;
	msg[1] = currRegVal & (~pins);

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	return true;
}

bool GpioExpGetIntFlag(GpioExpander* gpioExp, uint8_t port, uint8_t* flagVal)
{
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK1_INTFA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK1_INTFB;
		break;
	default:
		return false;
	}


	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, flagVal))
		return false;

	return true;
}

bool GpioExpIntReadPort(GpioExpander* gpioExp, uint8_t port, uint8_t* portVal)
{
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_INTCAPA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_INTCAPB;
		break;
	default:
		return false;
	}

	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, portVal))
		return false;

	return true;
}

bool GpioExpReadPort(GpioExpander* gpioExp, uint8_t port, uint8_t* portVal)
{
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_GPIOA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_GPIOB;
		break;
	default:
		return false;
	}

	if(!GpioExpGetCurrRegVal(gpioExp, regAddr, portVal))
		return false;

	return true;
}

bool GpioExpSetPin(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_OLATA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_OLATB;
		break;
	default:
		return false;
	}

	pins &= gpioExp->outPins;

	msg[0] = regAddr;
	msg[1] = gpioExp->outPinsState | pins;

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	gpioExp->outPinsState = msg[1];

	return true;
}

bool GpioExpClearPin(GpioExpander* gpioExp, uint8_t port, uint8_t pins)
{
	uint8_t msg[WRITE_MSG_SIZE];
	uint8_t regAddr = 0;

	switch(port)
	{
	case GPIOEXP_PORTA:
		regAddr = GPIOEXP_BANK0_OLATA;
		break;
	case GPIOEXP_PORTB:
		regAddr = GPIOEXP_BANK0_OLATB;
		break;
	default:
		return false;
	}

	pins &= gpioExp->outPins;

	msg[0] = regAddr;
	msg[1] = gpioExp->outPinsState & (~pins);

	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, msg, WRITE_MSG_SIZE))
		return false;

	gpioExp->outPinsState = msg[1];

	return true;
}

bool GpioExpGetCurrRegVal(GpioExpander* gpioExp, uint8_t regAddr, uint8_t* regVal)
{
	if(!I2CComSend(gpioExp->i2cInstance, gpioExp->hwAddress, &regAddr, READ_MSG_SIZE))
		return false;

	if(!I2CComReceive(gpioExp->i2cInstance, gpioExp->hwAddress, regVal, READ_MSG_SIZE))
		return false;

	return true;
}


