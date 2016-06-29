//***********************************************
// Motor driver.
// autor: Marcin Gozdziewski
//***********************************************
#ifndef MOTOR_H
#define MOTOR_H
#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include "MCP23017.h"


#define FORWARD			0x00
#define REVERSE			0x01
#define BREAK			0x02
#define COAST			0x03

#define MOTOR_LEFT		0x00
#define MOTOR_RIGHT		0x01

#define PWM_0			0x00
#define PWM_1			0x01

//*************************************************************************************
//MotorA       ENA  IN1  IN2
//Forward     High High Low
//Reverse     High Low  High
//Coast       Low   X    X     "X" means it can be high or low
//Brake       High Low  Low    (If both IN1 and IN2 are high, it will also brake)
//*************************************************************************************
typedef struct
{
	GpioExpander* gpioExpander;
	uint8_t pwm; // PWM_0 / PWM_1
	uint8_t portFwd;
	uint8_t pinFwd;
	uint8_t portRev;
	uint8_t pinRev;
	// to this point must be set
	// don't change below
	uint8_t direction;			// FORWARD or REVERSE
	uint8_t stopState;			// BREAK or COAST
	float dutyCycle;			// PWM duty cycle in percents
	uint32_t dutyCycleChangeTime;		// how fast can duty cycle change from one value to the other
	bool initialized;
	bool isRunning;
} MotorInstance;

void initializePwm(	uint32_t pwmFrequency);
void setPwmFrequency(uint32_t pwmFrequency);
bool initializeMotor(MotorInstance* motorInstance);
bool startMotor(MotorInstance* motorInstance);
bool stopMotor(MotorInstance* motorInstance);
bool isMotorRunning(MotorInstance* motorInstance);
bool setMotorDirection(MotorInstance* motorInstance, uint8_t direction);
void setMotorStopState(MotorInstance* motorInstance, uint8_t stopState);
void setMotorDutyCycle(MotorInstance* motorInstance, float dutyCycle);
void setMotorDutyCycleChangeTime(MotorInstance* motorInstance, uint32_t dutyCycleChangeTime);
uint8_t getMotorDirection(MotorInstance* motorInstance);
uint8_t getMotorStopState(MotorInstance* motorInstance);
float getMotorDutyCycle(MotorInstance* motorInstance);
uint8_t getMotorDutyCycleChangeTime(MotorInstance* motorInstance);

#endif // MOTOR_H
