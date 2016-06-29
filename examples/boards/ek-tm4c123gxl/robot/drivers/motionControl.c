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

#define DEF_SAMPLE_PERIOD		10
#define MS_CTRL_QUEUE_SIZE			  2

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

typedef enum
{
	MctrlInclination = 0,
	MctrlPidOut
} DataType;

typedef struct
{
	uint8_t id;
	uint8_t prevDutyCycle;
	uint8_t direction;
	bool isRunning;
	float pidOffset;
} MCtrlMotor;

typedef struct
{
	MCtrlMotor motors[MOTORS_NUM];
	float inclination;
	float pidOut;
} MCtrlStatus;

static MCtrlConfig g_mCtrlconfig;
static PidCtrlInstance* g_pidCtrl;
static MCtrlStatus g_status;
static bool g_initialized;

static bool getInclination(float* inclination);
static float complementaryFilter(float accAngle, float gyroRate, bool isValid);
static bool updateMotors(float pidOutput);
static bool startMotor(uint8_t motorId);
static bool stopMotor(uint8_t motorId);
static bool startMotors();
static bool setMotorDirection(uint8_t motorId, uint8_t direction);
static bool setDutyCycle(uint8_t motorId, uint8_t dutyCycle);
static void initTimer();
static void startTimer(uint32_t periodMs);
static void stopTimer();


void MCtrlInitialize()
{

	g_MCtrlQueue = registerMsgQueue(MS_CTRL_QUEUE_SIZE);

	initTimer();
	g_mCtrlconfig.period = DEF_SAMPLE_PERIOD;

	g_pidCtrl = (PidCtrlInstance*) pvPortMalloc(sizeof(PidCtrlInstance));

	if(!g_pidCtrl)
	{
		logger(Error, Log_MotionCtrl, "[MCtrlInitilize] Out of memory");
		return;
	}

	ZeroBuffer(g_pidCtrl, sizeof(PidCtrlInstance));

	PidInitialize(g_pidCtrl, g_mCtrlconfig.period, 0.0f, 0.0f, 0.0f, 0.0f, PidDirect);

	PidSetOutputLimits(g_pidCtrl, -100.0f, 100.0f);

	if(!startMotors())
		logger(Error, Log_MotionCtrl, "[MCtrlInitilize] Couldn't start motors");

	logger(Info, Log_MotionCtrl, "[MCtrlInitilize] Motors started");

	startTimer(g_mCtrlconfig.period);

	g_initialized = true;

	logger(Info, Log_MotionCtrl, "[MCtrlInitilize] Motion control initialized");
}

void MCtrlDoJob()
{
	if(!g_initialized)
		return;

	if(!getInclination(&g_status.inclination))
		logger(Error, Log_MotionCtrl, "[MCtrlDoJob] couldn't get inclination");

	g_status.pidOut = PidCompute(g_pidCtrl, g_status.inclination);

	updateMotors(g_status.pidOut);

}

void MCtrlSetPeriod(uint32_t period)
{
	stopTimer();
	g_mCtrlconfig.period = period;
	PidSetSampPeriod(g_pidCtrl, period);
	startTimer(period);
}

void MCtrlSetPidParam(uint8_t param, float value)
{
	PidSetParam(g_pidCtrl, (PidParameter) param, value);
}

void MCtrlSetPidDir(uint8_t direction)
{
	PidSetDirection(g_pidCtrl, (PidDirection) direction);
}

float MCtrlGetData(uint8_t param)
{
	switch((DataType) param)
	{
	case MctrlInclination:
		return g_status.inclination;
	case MctrlPidOut:
		return g_status.pidOut;
	default:
		break;
	}

	return 0.0f;
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

	*inclination = complementaryFilter(getMpuDataRsp->accelX,
									   getMpuDataRsp->gyroX,
									   getMpuDataRsp->isAccValid);

	vPortFree(getMpuDataRsp);
	getMpuDataRsp = NULL;

	return true;
}

float complementaryFilter(float accAngle, float gyroRate, bool isValid)
{
	float angle = gyroRate * ((float)g_mCtrlconfig.period / 1000.0f);

	if(!isValid)
		return angle;

	return 0.98 * angle + 0.02 * accAngle;
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

		if(pidOutputCorr >  (-1.0f) && pidOutputCorr < 1.0f)
		{
			if(g_status.motors[id].isRunning)
				stopMotor(id);
		}
		else
		{
			float dutyCycle = (pidOutputCorr < 0.0f ? ((-1) * pidOutputCorr) : pidOutputCorr);

			setDutyCycle(id, dutyCycle);

			if(!g_status.motors[id].isRunning)
				startMotor(id);

		}
	}
}

bool startMotors()
{
	for(int id = 0; id != MOTORS_NUM; ++id)
	{
		if(!startMotor(id))
			return false;
	}

	return true;
}

bool startMotor(uint8_t motorId)
{
	MotorStartMsgReq* startMotorReq = (MotorStartMsgReq*) pvPortMalloc(sizeof(MotorStartMsgReq));

	if(!startMotorReq)
	{
		logger(Error, Log_MotionCtrl, "[startMotor] couldn't allocate startMotorReq");
		return false;
	}

	*startMotorReq = INIT_MOTOR_START_MSG_REQ;

	startMotorReq->motorId = motorId;

	if(!msgSend(g_MCtrlQueue, getAddressFromTaskId(Msg_MotorsTaskID), &startMotorReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_MotionCtrl, "[startMotor] couldn't send startMotorReq");
		return false;
	}

	g_status.motors[motorId].isRunning = true;

	return true;
}

bool stopMotor(uint8_t motorId)
{
	MotorStopMsgReq* stopMotorReq = (MotorStopMsgReq*) pvPortMalloc(sizeof(MotorStopMsgReq));

	if(!stopMotorReq)
	{
		logger(Error, Log_MotionCtrl, "[stopMotor] couldn't allocate stopMotorReq");
		return false;
	}

	*stopMotorReq = INIT_MOTOR_STOP_MSG_REQ;

	stopMotorReq->motorId = motorId;

	if(!msgSend(g_MCtrlQueue, getAddressFromTaskId(Msg_MotorsTaskID), &stopMotorReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_MotionCtrl, "[stopMotor] couldn't send stopMotorReq");
		return false;
	}

	g_status.motors[motorId].isRunning = false;

	return true;
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

