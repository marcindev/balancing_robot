
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "motor.h"
#include "logger.h"

#define PWM_PERIOD_IN_CYCLES	100

static I2cManager g_i2cManager;
static uint32_t g_pwmFrequency;
const uint32_t g_pwmPeriod = PWM_PERIOD_IN_CYCLES;
static bool g_pwmInitialized = false;

static void initPwm();
static void startPwm();
static void stopPwm();


void initializePwm(	uint32_t pwmFrequency)
{
	g_pwmFrequency = pwmFrequency;
	initPwm();
	startPwm();
	g_pwmInitialized = true;
	logger(Info, Log_Motors, "[initializePwm] PWM initialized");
}

void setPwmFrequency(uint32_t pwmFrequency)
{
	g_pwmFrequency = pwmFrequency;

    // Set the PWM period to 250Hz.  To calculate the appropriate parameter
    // use the following equation: N = (1 / f) * SysClk.  Where N is the
    // function parameter, f is the desired frequency, and SysClk is the
    // system clock frequency.

	uint32_t cycles = (1 / g_pwmFrequency) * SysCtlClockGet()/64;

    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, cycles);

    logger(Info, Log_Motors, "[setPwmFrequency] PWM frequency was set");

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

	result &= GpioExpSetPinDirOut(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
	result &= GpioExpSetPinDirOut(gpioExpander, motorInstance->portRev, motorInstance->pinRev);

	if(!result)
		return false;

	setMotorDutyCycle(motorInstance, motorInstance->dutyCycle);

	result &= GpioExpClearPin(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
	result &= GpioExpClearPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);

	motorInstance->initialized = result;

	logger(Info, Log_Motors, "[initializeMotor] Motor initialized");

	return result;
}

bool startMotor(MotorInstance* motorInstance)
{
	if(!motorInstance->initialized)
		return false;

	bool result = true;

	result = setMotorDirection(motorInstance, motorInstance->direction);

	switch(motorInstance->pwm)
	{
	case PWM_0:
		PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
		break;
	case PWM_1:
		PWMOutputState(PWM1_BASE, PWM_OUT_1_BIT, true);
		break;
	default:
		result = false;
	}

	if(result)
	{
		motorInstance->isRunning = true;
	}

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
		switch(motorInstance->pwm)
		{
		case PWM_0:
			PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
			break;
		case PWM_1:
			PWMOutputState(PWM1_BASE, PWM_OUT_1_BIT, true);
			break;
		default:
			result = false;
		}

		result &= GpioExpClearPin(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
		result &= GpioExpClearPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);
		break;
	case COAST:
		switch(motorInstance->pwm)
		{
		case PWM_0:
			PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, false);
			break;
		case PWM_1:
			PWMOutputState(PWM1_BASE, PWM_OUT_1_BIT, false);
			break;
		default:
			result = false;
		}
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

bool setMotorDirection(MotorInstance* motorInstance, uint8_t direction)
{
	motorInstance->direction = direction;

	if(!motorInstance->initialized)
		return false;

	bool result = true;

	GpioExpander* gpioExpander = motorInstance->gpioExpander;

	switch(motorInstance->direction)
	{
	case FORWARD:
		result &= GpioExpSetPin(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
		result &= GpioExpClearPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);
		break;
	case REVERSE:
		result &= GpioExpClearPin(gpioExpander, motorInstance->portFwd, motorInstance->pinFwd);
		result &= GpioExpSetPin(gpioExpander, motorInstance->portRev, motorInstance->pinRev);
		break;
	default:
		result = false;
		break;
	}

	return result;
}

void setMotorStopState(MotorInstance* motorInstance, uint8_t stopState)
{
	motorInstance->stopState = stopState;
}

void setMotorDutyCycle(MotorInstance* motorInstance, uint8_t dutyCycle)
{
	motorInstance->dutyCycle = dutyCycle;

	switch(motorInstance->pwm)
	{
	case PWM_0:
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0,
	                     ((float)PWMGenPeriodGet(PWM1_BASE, PWM_OUT_0))
						 * (((float)motorInstance->dutyCycle) / 100.0f) );
		break;
	case PWM_1:
	    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1,
	                     ((float)PWMGenPeriodGet(PWM1_BASE, PWM_OUT_1))
						 * (((float)motorInstance->dutyCycle) / 100.0f) );
		break;
	default:
		break;
	}

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


void initPwm()
{
    // Set the PWM clock to the system clock.
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    // The PWM peripheral must be enabled for use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    GPIOPinConfigure(GPIO_PD0_M1PWM0);
    GPIOPinConfigure(GPIO_PD1_M1PWM1);

    // Configure the GPIO pad for PWM function on pins PD0 and PD1.
    GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_1);

    // Configure the PWM1 to count up/down without synchronization.
    PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN |
                    PWM_GEN_MODE_NO_SYNC);

    setPwmFrequency(g_pwmFrequency);

    // set PWM duty cycle to 0
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, 0);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_1, 0);
}

void startPwm()
{
    PWMGenEnable(PWM1_BASE, PWM_GEN_0);
}

void stopPwm()
{
	PWMGenDisable(PWM1_BASE, PWM_GEN_0);
}
