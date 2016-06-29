#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	PidDirect = 0,
	PidReverse
} PidDirection;

typedef enum
{
	PidManual = 0,
	PidAuto
} PidMode;

typedef enum
{
	PidProportional = 0,
	PidIntegral,
	PidDerivative,
	PidSetPoint
} PidParameter;

typedef struct
{
	float setPoint,
		  kp,
		  ki,
		  kd;
	float samplePeriodSec;
	PidDirection direction;

	float iTerm,
		  lastInput;
	float outMin,
		  outMax;

} PidCtrlInstance;


void PidInitialize(PidCtrlInstance* pidCtrl,
				   uint32_t samplePeriodMs,
				   float setPoint,
				   float kp,
				   float ki,
				   float kd,
				   PidDirection dir
				   );

float PidCompute(PidCtrlInstance* pidCtrl, float input);
void PidSetParam(PidCtrlInstance* pidCtrl, PidParameter param, float value);
void PidSetSampPeriod(PidCtrlInstance* pidCtrl, uint32_t periodMs);
void PidSetDirection(PidCtrlInstance* pidCtrl, PidDirection direction);
void PidSetOutputLimits(PidCtrlInstance* pidCtrl, float min, float max);

#endif // PID_CONTROLLER_H
