
#include "encoder.h"

#define MAX_ENCODERS	3
#define STATES_NUM		4

#define ENCODERS_GPIOPORT	GPIOEXP_PORTB

static EncoderInstance* g_encoders[MAX_ENCODERS];
static uint8_t g_encodersNum = 0;

static bool isEncoderPin(EncoderInstance* encoder, uint8_t pin);
static uint8_t portStateToFsmState(EncoderInstance* encoder, uint8_t portState);

static uint8_t g_fsm[STATES_NUM][STATES_NUM] = {     // [currState][prevState]
			{ 0,-1, 1, 0 },
			{ 1, 0, 0,-1 },
			{-1, 0, 0, 1 },
			{ 0, 1,-1, 0 }
};

bool initEncoder(EncoderInstance* encoder)
{
	if(g_encodersNum == MAX_ENCODERS)
		return false;

	encoder->rotationsCounter = 0;
	encoder->prevState = BOTH_CHANNELS_LOW;
	encoder->statesCounter = 0;
	g_encoders[g_encodersNum++] = encoder;

	GpioExpInit(encoder->gpioExpander);
	GpioExpSetPinDirIn(encoder->gpioExpander, ENCODERS_GPIOPORT, encoder->ch1_pin | encoder->ch2_pin);
//	GpioExpPullupEnable(encoder->gpioExpander, ENCODERS_GPIOPORT, encoder->ch1_pin | encoder->ch2_pin);
	GpioExpIntOnChangePrevValEnable(encoder->gpioExpander, ENCODERS_GPIOPORT, encoder->ch1_pin | encoder->ch2_pin);
	uint8_t portVal = 0;

	GpioExpReadPort(encoder->gpioExpander, ENCODERS_GPIOPORT, &portVal); // to clear any pending flags

	return true;
}

void doEncoderJob()
{
	uint8_t fsmState = 0,
			triggerPins = 0,
			portState = 0;
	GpioExpander* gpioExp = gpioExp;
	bool isPortRead = false;

	if(g_encodersNum)
		gpioExp = g_encoders[0]->gpioExpander;
	else
		return;

	if(!GpioExpGetIntFlag(gpioExp, ENCODERS_GPIOPORT, &triggerPins))
		return;


	for(int i = 0; i != g_encodersNum; ++i)
	{
		if(!isEncoderPin(g_encoders[i], triggerPins))
			continue;

		if(!isPortRead)
		{
			if(!GpioExpIntReadPort(gpioExp, ENCODERS_GPIOPORT, &portState))
				return;

			isPortRead = true;
		}


		fsmState = portStateToFsmState(g_encoders[i], portState);

		int8_t increment = g_fsm[fsmState][g_encoders[i]->prevState];
		g_encoders[i]->prevState = fsmState;
		g_encoders[i]->statesCounter += increment;

		if(g_encoders[i]->statesCounter >= STATES_NUM * 2) 	// STATES_NUM * 2 = full rotation
		{
			g_encoders[i]->statesCounter = 0;
			g_encoders[i]->rotationsCounter++;
		}

		if(g_encoders[i]->statesCounter <= ((-1) * STATES_NUM * 2)) 	// STATES_NUM * 2 = full rotation
		{
			g_encoders[i]->statesCounter = 0;
			g_encoders[i]->rotationsCounter--;
		}
	}
}

inline bool isEncoderPin(EncoderInstance* encoder, uint8_t pin)
{
	return (pin & (encoder->ch1_pin)) || (pin & (encoder->ch2_pin));
}

inline uint8_t portStateToFsmState(EncoderInstance* encoder, uint8_t portState)
{
	if((!(portState & encoder->ch1_pin )) && (!(portState & encoder->ch2_pin)))
		return BOTH_CHANNELS_LOW;

	if((portState & encoder->ch1_pin ) && (!(portState & encoder->ch2_pin)))
		return CHANNEL1_HIGH;

	if((!(portState & encoder->ch1_pin )) && (portState & encoder->ch2_pin))
		return CHANNEL2_HIGH;

	if((portState & encoder->ch1_pin ) && (portState & encoder->ch2_pin))
		return BOTH_CHANNELS_HIGH;

	return 0;
}
