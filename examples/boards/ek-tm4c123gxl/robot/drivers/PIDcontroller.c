#include "PIDcontroller.h"



void PidInitialize(PidCtrlInstance* pidCtrl,
				   uint32_t samplePeriodMs,
				   float setPoint,
				   float kp,
				   float ki,
				   float kd,
				   PidDirection dir
				   )
{
	pidCtrl->samplePeriodSec = (float)samplePeriodMs / 1000.0f;
	pidCtrl->setPoint = setPoint;
	pidCtrl->kp = kp;
	pidCtrl->ki = ki * pidCtrl->samplePeriodSec;
	pidCtrl->kd = kd / pidCtrl->samplePeriodSec;
	pidCtrl->outMin = -100000.0f;
	pidCtrl->outMax = 100000.0f;
}

float PidCompute(PidCtrlInstance* pidCtrl, float input)
{

	float error = pidCtrl->setPoint - input;
	pidCtrl->iTerm += (pidCtrl->ki * error);

	if(pidCtrl->iTerm > pidCtrl->outMax)
	{
		pidCtrl->iTerm = pidCtrl->outMax;
	}
	else if(pidCtrl->iTerm < pidCtrl->outMin)
	{
		pidCtrl->iTerm = pidCtrl->outMin;
	}

	float dInput = input - pidCtrl->lastInput;

	float output = pidCtrl->kp * error + pidCtrl->iTerm - pidCtrl->kd * dInput;

	if(output > pidCtrl->outMax)
	{
		output = pidCtrl->outMax;
	}
	else if(output < pidCtrl->outMin)
	{
		output = pidCtrl->outMin;
	}

	pidCtrl->lastInput = input;

	return output;
}

void PidSetParam(PidCtrlInstance* pidCtrl, PidParameter param, float value)
{
	switch(param)
	{
	case PidProportional:
		pidCtrl->kp = value;
		break;
	case PidIntegral:
		pidCtrl->ki = value * pidCtrl->samplePeriodSec;
		break;
	case PidDerivative:
		pidCtrl->kd = value / pidCtrl->samplePeriodSec;
		break;
	default:
		break;
	}

}

void PidSetSampPeriod(PidCtrlInstance* pidCtrl, uint32_t periodMs)
{
	float newPeriodSec = (float)periodMs / 1000.0f;

	float ratio = newPeriodSec / pidCtrl->samplePeriodSec;
	pidCtrl->ki *= ratio;
	pidCtrl->kd /= ratio;

	pidCtrl->samplePeriodSec = newPeriodSec;
}

void PidSetDirection(PidCtrlInstance* pidCtrl, PidDirection direction)
{
	pidCtrl->direction = direction;
}

void PidSetOutputLimits(PidCtrlInstance* pidCtrl, float min, float max)
{
	if(min > max)
		return;

	pidCtrl->outMin = min;
	pidCtrl->outMax = max;

	if(pidCtrl->iTerm > max) pidCtrl->iTerm = max;
	else if(pidCtrl->iTerm < min) pidCtrl->iTerm = min;

}


