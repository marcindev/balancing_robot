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
	PidProportional,
	PidIntegral,
	PidDerivative
} PidParameter;

typedef struct
{

} PidCtrlInstance;


void PidInitialize(PidCtrlInstance* pidCtrl,
				   uint32_t samplePeriod,
				   float setPoint,
				   float kp,
				   float ki,
				   float kd,
				   PidDirection dir
				   );

float PidCompute(PidCtrlInstance* pidCtrl, float input);
void PidSetParam(PidCtrlInstance* pidCtrl, PidParameter param, float value);
void PidSetSampPeriod(PidCtrlInstance* pidCtrl, uint32_t period);

#endif // PID_CONTROLLER_H
