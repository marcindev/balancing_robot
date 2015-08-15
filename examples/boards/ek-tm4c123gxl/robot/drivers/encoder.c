
#include "FreeRTOS.h"
#include "encoder.h"

#define MAX_ENCODERS	3
#define STATES_NUM		4

#define ENCODERS_GPIOPORT	GPIOEXP_PORTB

static EncoderInstance* g_encoders[MAX_ENCODERS];
static uint8_t g_encodersNum = 0;

static bool isEncoderPin(EncoderInstance* encoder, uint8_t pin);
static uint8_t portStateToFsmState(EncoderInstance* encoder, uint8_t portState);

static const int8_t g_fsm[STATES_NUM][STATES_NUM] = {     // [currState][prevState]
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
	encoder->prevRotationsCounter = 0;
	encoder->speed = 0;
	encoder->prevState = BOTH_CHANNELS_LOW;
	encoder->statesCounter = 0;
	encoder->rotationsCounterSem = xSemaphoreCreateMutex();
	encoder->speedSem = xSemaphoreCreateMutex();
	g_encoders[g_encodersNum++] = encoder;

	GpioExpInit(encoder->gpioExpander);
	GpioExpSetPinDirIn(encoder->gpioExpander, ENCODERS_GPIOPORT, encoder->ch1_pin | encoder->ch2_pin);
//	GpioExpPullupEnable(encoder->gpioExpander, ENCODERS_GPIOPORT, encoder->ch1_pin | encoder->ch2_pin);
	GpioExpIntOnChangePrevValEnable(encoder->gpioExpander, ENCODERS_GPIOPORT, encoder->ch1_pin | encoder->ch2_pin);
	uint8_t portVal = 0;

	GpioExpReadPort(encoder->gpioExpander, ENCODERS_GPIOPORT, &portVal); // to clear any pending flags

	return true;
}


int64_t getRotationsCounter(EncoderInstance* encoder)
{
	int64_t rotCntr = 0;

	xSemaphoreTake(encoder->rotationsCounterSem, portMAX_DELAY);
	rotCntr = encoder->rotationsCounter;
	xSemaphoreGive(encoder->rotationsCounterSem);

	return rotCntr;
}

uint64_t getEncoderSpeed(EncoderInstance* encoder)
{
	uint64_t speed = 0;

	xSemaphoreTake(encoder->speedSem, portMAX_DELAY);
	speed = encoder->speed;
	xSemaphoreGive(encoder->speedSem);

	return speed;
}

void measureEncoderSpeed(EncoderInstance* encoder, uint32_t period)
{
	int64_t currRotationsCounter = getRotationsCounter(encoder);
	int64_t rotDelta = currRotationsCounter  - encoder->prevRotationsCounter;
	encoder->prevRotationsCounter = currRotationsCounter;

	if(rotDelta < 0)
		rotDelta *= (-1);

	uint64_t speed = rotDelta * (1000 / period); // rotations per sec

	xSemaphoreTake(encoder->speedSem, portMAX_DELAY);
	encoder->speed = speed;
	xSemaphoreGive(encoder->speedSem);

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
			xSemaphoreTake(g_encoders[i]->rotationsCounterSem, portMAX_DELAY);
			g_encoders[i]->rotationsCounter++;
			xSemaphoreGive(g_encoders[i]->rotationsCounterSem);
		}

		if(g_encoders[i]->statesCounter <= ((-1) * STATES_NUM * 2)) 	// STATES_NUM * 2 = full rotation
		{
			g_encoders[i]->statesCounter = 0;
			xSemaphoreTake(g_encoders[i]->rotationsCounterSem, portMAX_DELAY);
			g_encoders[i]->rotationsCounter--;
			xSemaphoreGive(g_encoders[i]->rotationsCounterSem);
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


