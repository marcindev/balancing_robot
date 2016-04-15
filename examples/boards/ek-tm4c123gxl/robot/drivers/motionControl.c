#include "motionControl.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "messages.h"
#include "msgSystem.h"
#include "PIDcontroller.h"

#define DEF_SAMPLE_PERIOD		100
#define MS_CTRL_QUEUE_SIZE			  2
#define MS_CTRL_ITEM_SIZE			  4			// bytes

#define MOTOR_DIR_FWD				0
#define MOTOR_DIR_REV				1

#define MOTOR_LEFT					0
#define MOTOR_RIGHT					1

#define MOTORS_NUM 					2

static xQueueHandle g_MCtrlQueue;

typedef struct
{
	uint32_t period;

} MCtrlConfig;


typedef struct
{
	uint8_t id;
	uint8_t prevDutyCycle;
	uint8_t direction;
	float pidOffset;
} MCtrlMotor;

typedef struct
{
	MCtrlMotor motors[MOTORS_NUM];
} MCtrlStatus;

static MCtrlConfig config;
static PidCtrlInstance* g_pidCtrl;
MCtrlStatus g_status;

static bool getInclination(float* inclination);
static bool updateMotors(float pidOutput);
static bool setMotorDirection(uint8_t motorId, uint8_t direction);
static bool setDutyCycle(uint8_t motorId, uint8_t dutyCycle);
static void initTimer();
static void startTimer(uint32_t periodMs);
static void stopTimer();


void MCtrlInitialize()
{
	g_MCtrlQueue = xQueueCreate(MS_CTRL_QUEUE_SIZE, MS_CTRL_ITEM_SIZE);

	initTimer();
	config.period = DEF_SAMPLE_PERIOD;

	g_pidCtrl = (PidCtrlInstance*) pvPortMalloc(sizeof(PidCtrlInstance));

	if(!g_pidCtrl)
	{
		logger(Error, Log_MotionCtrl, "[MCtrlInitilize] Out of memory");
		return;
	}

	ZeroBuffer(g_pidCtrl, sizeof(PidCtrlInstance));

	PidInitialize(g_pidCtrl, config.period, 0.0f, 0.0f, 0.0f, 0.0f, PidDirect);

	startTimer(config.period);

	logger(Info, Log_MotionCtrl, "[MCtrlInitilize] Motion control initialized");
}

void MCtrlDoJob()
{

	updateMotors(PidCompute(g_pidCtrl, getInclination()));

}


bool getInclination(float* inclination)
{
	MpuGetDataMsgReq* getMpuDataReq = (MpuGetDataMsgReq*) pvPortMalloc(sizeof(MpuGetDataMsgReq));

	if(!getMpuDataReq)
	{
		logger(Error, Log_MotionCtrl, "[getInclination] couldn't allocate getMpuDataReq");
		return false;
	}

	*getMpuDataReq = INIT_MPU_GET_DATA_MSG_REQ;

	if(!msgSend(g_MCtrlQueue, getAddressFromTaskId(Msg_MpuTaskID), &getMpuDataReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_MotionCtrl, "[getInclination] couldn't send getMpuDataReq");
		return false;
	}

	MpuGetDataMsgRsp* getMpuDataRsp;

	if(!msgReceive(g_MCtrlQueue, &getMpuDataRsp, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_MotionCtrl, "[getInclination] couldn't receive getMpuDataRsp");
		return false;
	}

	*inclination = getMpuDataRsp->accelX;

	vPortFree(getMpuDataRsp);
	getMpuDataRsp = NULL;

	return true;
}

bool updateMotors(float pidOutput)
{
	for(int id = 0; id != MOTORS_NUM; ++id)
	{
		uint8_t newDir = (pidOutput >= 0.0f ? MOTOR_DIR_FWD : MOTOR_DIR_REV);

		if(newDir != g_status.motors[id].direction)
		{
			setMotorDirection(id, newDir);
			g_status.motors[id].direction = newDir;
		}

		float pidOutputCorr = pidOutput + g_status.motors[id].pidOffset;

		uint8_t dutyCycle = (uint8_t) (pidOutputCorr < 0.0f ? ((-1) * pidOutputCorr) : pidOutputCorr);

		setDutyCycle(id, dutyCycle);
	}
}


bool setMotorDirection(uint8_t motorId, uint8_t direction)
{
	MotorSetDirectionMsgReq* setDirReq = (MotorSetDirectionMsgReq*) pvPortMalloc(sizeof(MotorSetDirectionMsgReq));

	if(!setDirReq)
	{
		logger(Error, Log_MotionCtrl, "[setMotorDirection] couldn't allocate setDirReq");
		return false;
	}

	*setDirReq = INIT_MOTOR_SET_DIRECTION_MSG_REQ;

	setDirReq->motorId = motorId;
	setDirReq->direction = direction;

	if(!msgSend(g_MCtrlQueue, getAddressFromTaskId(Msg_MotorsTaskID), &setDirReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_MotionCtrl, "[setMotorDirection] couldn't send setDirReq");
		return false;
	}

	return true;
}

bool setDutyCycle(uint8_t motorId, uint8_t dutyCycle)
{
	MotorSetDutyCycleMsgReq* setDutyCycleReq = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));

	if(!setDutyCycleReq)
	{
		logger(Error, Log_MotionCtrl, "[setDutyCycle] couldn't allocate setDutyCycleReq");
		return false;
	}

	*setDutyCycleReq = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;

	setDutyCycleReq->motorId = motorId;
	setDutyCycleReq->dutyCycle = dutyCycle;

	if(!msgSend(g_MCtrlQueue, getAddressFromTaskId(Msg_MotorsTaskID), &setDutyCycleReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_MotionCtrl, "[setDutyCycle] couldn't send setDutyCycleReq");
		return false;
	}

	return true;
}


void initTimer()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);

}

void startTimer(uint32_t periodMs)
{
	TimerLoadSet(TIMER0_BASE, TIMER_B, (SysCtlClockGet() / 1000) * periodMs);
	TimerEnable(TIMER0_BASE, TIMER_B);
}

void stopTimer()
{
	TimerDisable(TIMER0_BASE, TIMER_B);
}

