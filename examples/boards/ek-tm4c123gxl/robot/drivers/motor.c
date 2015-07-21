
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "motor.h"

#define PWM_PERIOD_IN_CYCLES	100

extern SemaphoreHandle_t g_pwmCounterSem;
static I2cManager g_i2cManager;
static uint32_t g_pwmFrequency;
uint32_t g_pwmCounter;
const uint32_t g_pwmPeriod = PWM_PERIOD_IN_CYCLES;
static bool g_pwmInitialized = false;

static void initPwmTimer();
static void startPwmTimer();
static void stopPwmTimer();


void initializePwm(	uint32_t pwmFrequency)
{
	g_pwmFrequency = pwmFrequency;
	g_pwmCounter = 0;

	initPwmTimer();
	startPwmTimer();

	g_pwmInitialized = true;
}

void setPwmFrequency(uint32_t pwmFrequency)
{
	stopPwmTimer();
	g_pwmFrequency = pwmFrequency;
	startPwmTimer();
}

bool initializeMotor(MotorInstance* motorInstance)
{
	bool result = true;

	GpioExpander* gpioExpander = motorInstance->gpioExpander;
	gpioExpander->i2cManager = &g_i2cManager;

	motorInstance->dutyCycle = 10;
	motorInstance->isRunning = false;
	motorInstance->stopState = COAST;
	motorInstance->direction = FORWARD;

	GpioExpInit(gpioExpander);

	result &= GpioExpSetPinDirOut(gpioExpander, motorInstance->portEna, motorInstance->pinEna);
	result &= GpioExpSetPinDirOut(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
	result &= GpioExpSetPinDirOut(gpioExpander, motorInstance->portRev, motorInstance->pinRev);

	if(!result)
		return false;

	result &= GpioExpClearPin(gpioExpander, motorInstance->portEna, motorInstance->pinEna);
	result &= GpioExpClearPin(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
	result &= GpioExpClearPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);

	motorInstance->initialized = result;

	return result;
}

bool startMotor(MotorInstance* motorInstance)
{
	if(!motorInstance->initialized)
		return false;

	bool result = true;

	GpioExpander* gpioExpander = motorInstance->gpioExpander;

	switch(motorInstance->direction)
	{
	case FORWARD:
		result &= GpioExpSetPin(gpioExpander,
								motorInstance->portEna,
								motorInstance->pinEna
								| motorInstance->pinFwd);

		result &= GpioExpClearPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);
		break;
	case REVERSE:
		result &= GpioExpSetPin(gpioExpander,
								motorInstance->portEna,
								motorInstance->pinEna
								| motorInstance->pinRev);
		result &= GpioExpClearPin(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
		break;
	default:
		result = false;
		break;
	}

	if(result)
		motorInstance->isRunning = true;

	return result;
}

bool stopMotor(MotorInstance* motorInstance)
{
	if(!motorInstance->initialized)
		return false;

	bool result = true;

	GpioExpander* gpioExpander = motorInstance->gpioExpander;

	switch(motorInstance->stopState)
	{
	case BREAK:
		result &= GpioExpSetPin(gpioExpander, motorInstance->portEna, motorInstance->pinEna);
		result &= GpioExpClearPin(gpioExpander,
								  motorInstance->portFwd,
								  motorInstance->pinFwd |
								  motorInstance->pinRev);
		result &= GpioExpClearPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);
		break;
	case COAST:
		result &= GpioExpClearPin(gpioExpander, motorInstance->portEna, motorInstance->pinEna);
		break;
	default:
		result = false;
		break;
	}

	if(result)
		motorInstance->isRunning = false;

	return result;
}

bool isMotorRunning(MotorInstance* motorInstance)
{
	return motorInstance->isRunning;
}

void setMotorDirection(MotorInstance* motorInstance, uint8_t direction)
{
	motorInstance->direction = direction;
}

void setMotorStopState(MotorInstance* motorInstance, uint8_t stopState)
{
	motorInstance->stopState = stopState;
}

void setMotorDutyCycle(MotorInstance* motorInstance, uint8_t dutyCycle)
{
	motorInstance->dutyCycle = dutyCycle;
}

void setMotorDutyCycleChangeTime(MotorInstance* motorInstance, uint32_t dutyCycleChangeTime)
{
	motorInstance->dutyCycleChangeTime = dutyCycleChangeTime;
}

uint8_t getMotorDirection(MotorInstance* motorInstance)
{
	return motorInstance->direction;
}

uint8_t getMotorStopState(MotorInstance* motorInstance)
{
	return motorInstance->stopState;
}

uint8_t getMotorDutyCycle(MotorInstance* motorInstance)
{
	return motorInstance->dutyCycle;
}

uint8_t getMotorDutyCycleChangeTime(MotorInstance* motorInstance)
{
	return motorInstance->dutyCycleChangeTime;
}

void Timer3BIntHandler(void)
{
	static BaseType_t xHigherPriorityTaskWoken;

    TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);

    g_pwmCounter++;

    if(g_pwmCounter > g_pwmPeriod)
    	g_pwmCounter = 0;

    xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR( g_pwmCounterSem, &xHigherPriorityTaskWoken );

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void initPwmTimer()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

	// Configure Timer3B as a 16-bit periodic timer.
	TimerConfigure(TIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);

}

void startPwmTimer()
{
	// Set the Timer3B load value to 1ms.
	TimerLoadSet(TIMER3_BASE, TIMER_B, SysCtlClockGet() / g_pwmFrequency);

	// Configure the Timer3B interrupt for timer timeout.
	TimerIntEnable(TIMER3_BASE, TIMER_TIMB_TIMEOUT);

	// Enable the Timer3B interrupt on the processor (NVIC).
	IntEnable(INT_TIMER3B);

	// Enable Timer3B.
	TimerEnable(TIMER3_BASE, TIMER_B);
}

void stopPwmTimer()
{
	// Clear the timer interrupt flag.
	TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);

	// Disable the Timer0B interrupt.
	IntDisable(INT_TIMER3B);

	// Turn off Timer0B interrupt.
	TimerIntDisable(TIMER3_BASE, TIMER_TIMB_TIMEOUT);

	// Clear any pending interrupt flag.
	TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);

}
