//*****************************************************************************
//
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "global_defs.h"
#include "wdg.h"
#include "motorsTask.h"
#include "encodersTask.h"
#include "wheelsTask.h"
#include "wheel.h"
#include "logger.h"


#include "MCP23017.h"

#define WHEELS_TASK_STACK_SIZE		150        // Stack size in words
#define WHEELS_QUEUE_SIZE			 20
#define WHEELS_SUB_QUEUES			  5
#define WHEELS_ITEM_SIZE			  4			// bytes

#define WHEELS_NUMBER				  2
#define WHEEL_ROTATION			     80
#define DUTY_CYCLE_INCREMENT		  1
#define DUTY_CYCLE_TIMER_PERIOD		 50

typedef struct
{
	uint8_t encoderId;
	uint8_t motorId;
	uint8_t direction;
	float speed;
	TimerHandle_t dutyCycleTimer;
	bool stopDutyCycleTimer;
	uint8_t dutyCycle;
	int8_t dutyCycleIncr;
	bool isRunning;
	bool isSpeedReached;
	bool isRotationsReached;
}WheelData;

static WheelData wheelsData[WHEELS_NUMBER];

static MsgQueueId g_wheelsMainQueue;
static MsgQueueId g_motorsRxQueue;
static MsgQueueId g_encodersRxQueue;

static bool g_taskStarted = false;

static void initializeWheels();
static void initializeWheelsData();
static void initializeDutyCycleTimers();
static void doWheelsJob();
static void setDirection(uint8_t wheelId, uint8_t direction);
static void setDutyCycle(uint8_t wheelId, uint8_t dutyCycle);
static void startMotor(uint8_t wheelId);
static void stopMotor(uint8_t wheelId);
static bool getSpeedFromEncoder(uint8_t wheelId, float* speed);
static bool requestNotifAfterRotations(uint8_t wheelId, uint8_t direction, float rotations);
static bool requestNotifAfterSpeed(uint8_t wheelId, float speed);
static bool startDutyCycleTimer(uint8_t wheelId);
static void dutyCycleCallback(TimerHandle_t pxTimer);
static void handleMessages(void* msg);
static void handleStartTask(StartTaskMsgReq* request);
static void handleRun(WheelRunMsgReq* request);
static void handleSetSpeed(WheelSetSpeedMsgReq* request);
static void handleNotifyAfterRotations(EncoderNotifyAfterRotationsMsgRsp* response);
static void handleNotifyAfterSpeed(EncoderNotifyAfterSpeedMsgRsp* response);


static void wheelsTask()
{
	uint8_t wdgTaskID = registerToWatchDog();

	while(true)
	{
		void* msg;
		feedWatchDog(wdgTaskID, WDG_ASLEEP);
		bool result = msgReceive(g_wheelsMainQueue, &msg, 10);
		feedWatchDog(wdgTaskID, WDG_ALIVE);

		if(result)
		{
			handleMessages(msg);
		}

		doWheelsJob();
	}

}

bool wheelsTaskInit()
{
	g_wheelsMainQueue = registerMainMsgQueue(Msg_WheelsTaskID, WHEELS_QUEUE_SIZE);

	if(g_wheelsMainQueue < 0)
	{
		logger(Error, Log_Wheels, "[wheelsTaskInit] Couldn't register main msg queue");
		return false;
	}

	g_motorsRxQueue = registerMsgQueue(WHEELS_SUB_QUEUES);

	if(g_motorsRxQueue < 0)
	{
		logger(Error, Log_Wheels, "[wheelsTaskInit] Couldn't register g_motorsRxQueue msg queue");
		return false;
	}

	g_encodersRxQueue= registerMsgQueue(WHEELS_SUB_QUEUES);

	if(g_encodersRxQueue < 0)
	{
		logger(Error, Log_Wheels, "[wheelsTaskInit] Couldn't register g_encodersRxQueue msg queue");
		return false;
	}

	initializeWheels(); // initializes encoders and motors

    if(xTaskCreate(wheelsTask, (signed portCHAR *)"Wheels", WHEELS_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_WHEELS_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeWheels()
{
	if(!motorsTaskInit())
	{
		while(1){ }
	}

	if(!encodersTaskInit())
	{
		while(1){ }
	}

	initializeWheelsData();
	initializeDutyCycleTimers();
}

void initializeWheelsData()
{
	for(int i = 0; i != WHEELS_NUMBER; ++i)
	{
		wheelsData[i].encoderId = i;
		wheelsData[i].motorId = i;
		wheelsData[i].isRunning = false;
		wheelsData[i].dutyCycleIncr = DUTY_CYCLE_INCREMENT;
		wheelsData[i].stopDutyCycleTimer = false;
		wheelsData[i].dutyCycle = 0;
		wheelsData[i].speed = 0.0;
		wheelsData[i].isRotationsReached = true;
		wheelsData[i].isSpeedReached = true;
	}
}

void initializeDutyCycleTimers()
{
	for(int i = 0; i != WHEELS_NUMBER; ++i)
	{
		wheelsData[i].dutyCycleTimer = xTimerCreate(
				"DutyCycleTimer",
				pdMS_TO_TICKS(DUTY_CYCLE_TIMER_PERIOD),
				pdTRUE,
				( void * ) i,
				dutyCycleCallback
		);

		if(wheelsData[i].dutyCycleTimer == NULL)
		{
			logger(Error, Log_Wheels, "[initializeDutyCycleTimers] Couldn't create timer");
		}

	}
}

void doWheelsJob()
{
	if(!g_taskStarted)
		return;

	for(int i = 0; i != WHEELS_NUMBER; ++i)
	{
		if(wheelsData[i].isRunning)
		{
			if(wheelsData[i].isRotationsReached || wheelsData[i].isSpeedReached)
			{
				wheelsData[i].stopDutyCycleTimer = true;
			}

			if(wheelsData[i].isRotationsReached)
				stopMotor(i);
		}
	}
}

void handleMessages(void* msg)
{
	uint8_t msgId = *((uint8_t*)msg);

	switch(msgId)
	{
	case START_TASK_MSG_REQ:
		handleStartTask((StartTaskMsgReq*) msg);
		break;
	case WHEEL_RUN_MSG_REQ:
		handleRun((WheelRunMsgReq*) msg);
		break;
	case WHEEL_SET_SPEED_MSG_REQ:
		handleSetSpeed((WheelSetSpeedMsgReq*) msg);
		break;
	case ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP:
		handleNotifyAfterRotations((EncoderNotifyAfterRotationsMsgRsp*) msg);
		break;
	case ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP:
		handleNotifyAfterSpeed((EncoderNotifyAfterSpeedMsgRsp*) msg);
		break;
	default:
		logger(Warning, Log_Wheels, "[handleMessages] Received not recognized message %d", msgId);
		break;
	}
}

void handleStartTask(StartTaskMsgReq* request)
{
	bool result = true;

	StartTaskMsgReq* startMotorsTaskReq = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*startMotorsTaskReq = INIT_START_TASK_MSG_REQ;
	msgSend(g_motorsRxQueue, getAddressFromTaskId(Msg_MotorsTaskID), &startMotorsTaskReq, MSG_WAIT_LONG_TIME);

	StartTaskMsgRsp* response;
	result &= msgReceive(g_motorsRxQueue, &response, MSG_WAIT_LONG_TIME);
	if(result)
	{
		result &= response->status;
		vPortFree(response);
	}

	StartTaskMsgReq* startEncodersTaskReq = (StartTaskMsgReq*) pvPortMalloc(sizeof(StartTaskMsgReq));
	*startEncodersTaskReq = INIT_START_TASK_MSG_REQ;
	msgSend(g_encodersRxQueue, getAddressFromTaskId(Msg_EncoderTaskID), &startEncodersTaskReq, MSG_WAIT_LONG_TIME);

	result &= msgReceive(g_encodersRxQueue, &response, MSG_WAIT_LONG_TIME);
	if(result)
	{
		result &= response->status;
		vPortFree(response);
	}

	response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = result;
	msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);

	g_taskStarted = result;
}


void handleRun(WheelRunMsgReq* request)
{
	if(!g_taskStarted)
	{
		logger(Warning, Log_Wheels, "[handleRun] task not started yet");
		return;
	}

	uint8_t wheelId = request->wheelId;

	if(wheelId >= WHEELS_NUMBER)
		return;

	if(!requestNotifAfterRotations(wheelId, request->direction, request->rotations))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't request notifications after rotations");
		return;
	}

	if(!requestNotifAfterSpeed(wheelId, wheelsData[wheelId].speed))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't request notifications after speed");
		return;
	}

	float currSpeed;

	if(!getSpeedFromEncoder(wheelId, &currSpeed))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't get speed from encoder");
		return;
	}

	if(currSpeed < wheelsData[wheelId].speed)
		wheelsData[wheelId].dutyCycleIncr = DUTY_CYCLE_INCREMENT;
	else
		wheelsData[wheelId].dutyCycleIncr = DUTY_CYCLE_INCREMENT * (-1);

	// start the motor if not yet running
	if(!wheelsData[wheelId].isRunning)
		startMotor(wheelId);

	if(!startDutyCycleTimer(wheelId))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't start duty cycle timer");
		return;
	}

	if(request->direction != wheelsData[wheelId].direction)
	{
		setDirection(wheelId, request->direction);
		logger(Debug, Log_Wheels, "[handleRun] direction changed");
	}

}

void handleSetSpeed(WheelSetSpeedMsgReq* request)
{
	if(!g_taskStarted)
		return;

	wheelsData[request->wheelId].speed = request->speed;
	vPortFree(request);
}

void handleNotifyAfterRotations(EncoderNotifyAfterRotationsMsgRsp* response)
{
	logger(Debug, Log_Wheels, "[handleNotifyAfterRotations] rotations reached (actual rotations = %f)",
			((float)response->actualRotations)/WHEEL_ROTATION);
	wheelsData[response->encoderId].isRotationsReached = true;
	vPortFree(response);
}

void handleNotifyAfterSpeed(EncoderNotifyAfterSpeedMsgRsp* response)
{
	logger(Debug, Log_Wheels, "[handleNotifyAfterSpeed] speed reached (actual speed = %f)",
			((float)response->actualSpeed)/WHEEL_ROTATION);
	wheelsData[response->encoderId].isSpeedReached = true;
	vPortFree(response);
}

void setDirection(uint8_t wheelId, uint8_t direction)
{
	MotorSetDirectionMsgReq* directionReq = (MotorSetDirectionMsgReq*) pvPortMalloc(sizeof(MotorSetDirectionMsgReq));

	*directionReq = INIT_MOTOR_SET_DIRECTION_MSG_REQ;
	directionReq->motorId = wheelsData[wheelId].motorId;
	directionReq->direction = direction;

	if(!msgSend(g_motorsRxQueue, getAddressFromTaskId(Msg_MotorsTaskID), (void**) &directionReq, portMAX_DELAY))
	{
		logger(Error, Log_Wheels, "[setDirection] couldn't send directionReq");
		return;
	}

	wheelsData[wheelId].direction = direction;
}

void setDutyCycle(uint8_t wheelId, uint8_t dutyCycle)
{
	MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));

	*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
	request->motorId = wheelsData[wheelId].motorId;
	request->dutyCycle = 10;//dutyCycle;

	if(!msgSend(g_motorsRxQueue, getAddressFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
	{
		logger(Error, Log_Wheels, "[setDutyCycle] couldn't send request");
		return;
	}

	wheelsData[wheelId].dutyCycle = dutyCycle;
}

void startMotor(uint8_t wheelId)
{
	MotorStartMsgReq* request = (MotorStartMsgReq*) pvPortMalloc(sizeof(MotorStartMsgReq));

	*request = INIT_MOTOR_START_MSG_REQ;
	request->motorId = wheelsData[wheelId].motorId;

	if(!msgSend(g_motorsRxQueue, getAddressFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
	{
		logger(Error, Log_Wheels, "[startMotor] couldn't send request");
		return;
	}

	wheelsData[wheelId].isRunning = true;
}

void stopMotor(uint8_t wheelId)
{
	MotorStopMsgReq* request = (MotorStopMsgReq*) pvPortMalloc(sizeof(MotorStopMsgReq));

	*request = INIT_MOTOR_STOP_MSG_REQ;
	request->motorId = wheelsData[wheelId].motorId;

	if(!msgSend(g_motorsRxQueue, getAddressFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
	{
		logger(Error, Log_Wheels, "[stopMotor] couldn't send request");
		return;
	}

	wheelsData[wheelId].isRunning = false;
	wheelsData[wheelId].dutyCycle = 0;
}

bool getSpeedFromEncoder(uint8_t wheelId, float* speed)
{
	// Ask encoder for current speed
	EncoderGetSpeedMsgReq* getSpeedReq = (EncoderGetSpeedMsgReq*) pvPortMalloc(sizeof(EncoderGetSpeedMsgReq));

	if(!getSpeedReq)
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't allocate getSpeedReq");
		return false; // out of memory
	}

	*getSpeedReq = INIT_ENCODER_GET_SPEED_MSG_REQ;
	getSpeedReq->encoderId = wheelsData[wheelId].encoderId;

	if(!msgSend(g_encodersRxQueue, getAddressFromTaskId(Msg_EncoderTaskID), (void**) &getSpeedReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't send getSpeedReq");
		return false;
	}

	EncoderGetSpeedMsgRsp* getSpeedRsp;
	if(!msgReceive(g_encodersRxQueue, &getSpeedRsp, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't receive getSpeedRsp");
		return false;
	}

	*speed = getSpeedRsp->speed / (float)WHEEL_ROTATION;

	vPortFree(getSpeedRsp);

	return true;
}

bool requestNotifAfterRotations(uint8_t wheelId, uint8_t direction, float rotations)
{
	// Send request for rotations notification from encoder
	EncoderNotifyAfterRotationsMsgReq* rotationsReq = (EncoderNotifyAfterRotationsMsgReq*) pvPortMalloc(sizeof(EncoderNotifyAfterRotationsMsgReq));

	if(!rotationsReq)
	{
		logger(Error, Log_Wheels, "[requestNotifAfterRotations] couldn't allocate rotationsReq");
		return false; // out of memory
	}


	*rotationsReq = INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ;
	rotationsReq->encoderId = wheelsData[wheelId].encoderId;
	rotationsReq->direction = direction;
	rotationsReq->rotations = (int64_t)(rotations * (float) WHEEL_ROTATION);

	if(!msgSend(g_wheelsMainQueue, getAddressFromTaskId(Msg_EncoderTaskID), (void**) &rotationsReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't send rotationsReq");
		return false;
	}

	wheelsData[wheelId].isRotationsReached = false;

	return true;
}

bool requestNotifAfterSpeed(uint8_t wheelId, float speed)
{
	// Send request for speed notification from encoder
	EncoderNotifyAfterSpeedMsgReq* speedNotifReq = (EncoderNotifyAfterSpeedMsgReq*) pvPortMalloc(sizeof(EncoderNotifyAfterSpeedMsgReq));

	if(!speedNotifReq)
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't allocate speedReq");
		return false; // out of memory
	}

	*speedNotifReq = INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ;
	speedNotifReq->encoderId = wheelsData[wheelId].encoderId;
	speedNotifReq->speed = (uint64_t)(speed * (float)WHEEL_ROTATION);

	if(!msgSend(g_wheelsMainQueue, getAddressFromTaskId(Msg_EncoderTaskID), (void**) &speedNotifReq, MSG_WAIT_LONG_TIME))
	{
		logger(Error, Log_Wheels, "[handleRun] couldn't send speedReq");
		return false;
	}

	wheelsData[wheelId].isSpeedReached = false;

	return true;
}

bool startDutyCycleTimer(uint8_t wheelId)
{
    if( xTimerStart( wheelsData[wheelId].dutyCycleTimer, 0 ) != pdPASS )
    {
    	logger(Error, Log_Encoders, "[startDutyCycleTimer] Couldn't start timer");
    	return false;
    }

    wheelsData[wheelId].stopDutyCycleTimer = false;

    return true;
}

void dutyCycleCallback(TimerHandle_t pxTimer)
{
	uint8_t wheelId = (uint8_t) pvTimerGetTimerID( pxTimer );

	if(wheelsData[wheelId].stopDutyCycleTimer)
	{
		xTimerStop( pxTimer, 0 );
		return;
	}

	uint8_t newDutyCycle = wheelsData[wheelId].dutyCycle + wheelsData[wheelId].dutyCycleIncr;

	if(newDutyCycle > 100)
	{
		xTimerStop( pxTimer, 0 );
		return;
	}

	setDutyCycle(wheelId,  newDutyCycle);
}
