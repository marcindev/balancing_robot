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
#include "encoder.h"
#include "encodersTask.h"
#include "encoderSamplerTask.h"
#include "logger.h"

#define ENCODERS_TASK_STACK_SIZE	 110        // Stack size in words
#define ENCODERS_QUEUE_SIZE			 100

#define ENCODERS_NUMBER				  2

#define ENCODER_L_CH1_PIN		GPIOEXP_PIN4
#define ENCODER_L_CH2_PIN		GPIOEXP_PIN5
#define ENCODER_R_CH1_PIN		GPIOEXP_PIN6
#define ENCODER_R_CH2_PIN		GPIOEXP_PIN7

#define ENC_NOTIF_TIMER_PERIOD		10
#define SPEED_TIMER_PERIOD			100

typedef struct
{
	TimerHandle_t notifTimer;
	int64_t startCounter;
	int64_t threshold;
	int8_t expectedDirection;
	int8_t notifReceiver;
	bool isActive;
}RotationNotifData;

typedef struct
{
	uint64_t threshold;
	int8_t greaterOrLess;
	int8_t notifReceiver;
	bool isActive;
}SpeedNotifData;



static MsgQueueId g_encodersQueue;
static EncoderInstance* g_encoders[ENCODERS_NUMBER];
static RotationNotifData rotationNotifData[ENCODERS_NUMBER];
static SpeedNotifData speedNotifData[ENCODERS_NUMBER];
static TimerHandle_t speedTimer;

static void initializeEncoders();
static void initEncoderSamplerTask();
static void initializeSpeedTimer();
static void startSpeedMeasurement();
static void initializeNotifTimers();
static void notifyAfterRotationsCallback(TimerHandle_t pxTimer);
static void measureSpeedCallback(TimerHandle_t pxTimer);
static void checkSpeedNotif(uint8_t encoderId);
static void handleMessages(void* msg);
static void handleStartTask(StartTaskMsgReq* request);
static void handleGetCounter(EncoderGetCounterMsgReq* request);
static void handleGetSpeed(EncoderGetSpeedMsgReq* request);
static void handleNotifyAfterRotations(EncoderNotifyAfterRotationsMsgReq* request);
static void handleNotifyAfterSpeed(EncoderNotifyAfterSpeedMsgReq* request);


static void encodersTask()
{

	while(true)
	{
		void* msg;
		if(msgReceive(g_encodersQueue, &msg, MSG_WAIT_LONG_TIME))
		{
			handleMessages(msg);
		}

	}

}

bool encodersTaskInit()
{
	g_encodersQueue = registerMainMsgQueue(Msg_EncoderTaskID, ENCODERS_QUEUE_SIZE);

	if(g_encodersQueue < 0)
	{
		logger(Error, Log_Encoders, "[encodersTaskInit] Couldn't register main msg queue");
	}

    if(xTaskCreate(encodersTask, (signed portCHAR *)"Encoders", ENCODERS_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_ENCODERS_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeEncoders()
{

	I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
	GpioExpander* gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));
	if(!i2cManager || !gpioExpander)
	{
		logger(Error, Log_Encoders, "[initializeEncoders] Out of memory");
		return;
	}

	ZeroBuffer(gpioExpander, sizeof(GpioExpander));
	gpioExpander->i2cManager = i2cManager;
	gpioExpander->hwAddress		= GPIO_EXPANDER1_ADDRESS;

	EncoderInstance* encoderLeft = (EncoderInstance*) pvPortMalloc(sizeof(EncoderInstance));
	if(!encoderLeft)
	{
		logger(Error, Log_Encoders, "[initializeEncoders] Out of memory");
		return;
	}
	encoderLeft->gpioExpander = gpioExpander;
	encoderLeft->ch1_pin = ENCODER_L_CH1_PIN;
	encoderLeft->ch2_pin = ENCODER_L_CH2_PIN;
	g_encoders[ENCODER_LEFT] = encoderLeft;

	EncoderInstance* encoderRight = (EncoderInstance*) pvPortMalloc(sizeof(EncoderInstance));
	if(!encoderLeft)
	{
		logger(Error, Log_Encoders, "[initializeEncoders] Out of memory");
		return;
	}

	encoderRight->gpioExpander = gpioExpander;
	encoderRight->ch1_pin = ENCODER_R_CH1_PIN;
	encoderRight->ch2_pin = ENCODER_R_CH2_PIN;
	g_encoders[ENCODER_RIGHT] = encoderRight;

	for(int i = 0; i != ENCODERS_NUMBER; ++i)
	{
		initEncoder(g_encoders[i]);
	}

	logger(Info, Log_Encoders, "[initializeEncoders] Encoders initialized");
}

void initEncoderSamplerTask()
{
	if(!encoderSamplerTaskInit())
	{
		while(1){ }
	}
}

void startSpeedMeasurement()
{
	if( xTimerStart( speedTimer, 0 ) != pdPASS )
	{
		logger(Error, Log_Encoders, "[startSpeedMeasurement] Couldn't start timer");
	}

	logger(Debug, Log_Encoders, "[startSpeedMeasurement] timer started");
}

void initializeSpeedTimer()
{

	speedTimer = xTimerCreate(
			"SpeedTimer",
			pdMS_TO_TICKS(SPEED_TIMER_PERIOD),
			pdTRUE,
			( void * ) 0,
			measureSpeedCallback
	);

	if(speedTimer == NULL)
	{
		logger(Error, Log_Encoders, "[initializeSpeedTimer] Couldn't create timer");
	}

	logger(Info, Log_Encoders, "[initializeSpeedTimer] timer created");
}


void initializeNotifTimers()
{

	for(int i = 0; i != ENCODERS_NUMBER; ++i)
	{
		ZeroBuffer(&rotationNotifData[i], sizeof(RotationNotifData));
		ZeroBuffer(&speedNotifData[i], sizeof(SpeedNotifData));

		rotationNotifData[i].notifTimer = xTimerCreate(
				"EncNotifTimer",
				pdMS_TO_TICKS(ENC_NOTIF_TIMER_PERIOD),
				pdTRUE,
				( void * ) i,
				notifyAfterRotationsCallback
		);

		if(rotationNotifData[i].notifTimer == NULL)
		{
			logger(Error, Log_Encoders, "[initializeNotifTimers] Couldn't create timer");
		}

	}
}

void handleMessages(void* msg)
{
	switch(*((uint8_t*)msg))
	{
	case START_TASK_MSG_REQ:
		handleStartTask((StartTaskMsgReq*) msg);
		break;
	case ENCODER_GET_COUNTER_MSG_REQ:
		handleGetCounter((EncoderGetCounterMsgReq*) msg);
		break;
	case ENCODER_GET_SPEED_MSG_REQ:
		handleGetSpeed((EncoderGetSpeedMsgReq*) msg);
		break;
	case ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ:
		handleNotifyAfterRotations((EncoderNotifyAfterRotationsMsgReq*) msg);
		break;
	case ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ:
		handleNotifyAfterSpeed((EncoderNotifyAfterSpeedMsgReq*) msg);
		break;
	default:
		// Received not-recognized message
		logger(Warning, Log_Encoders, "[handleMessages] Received not-recognized message");
		break;
	}
}

void handleStartTask(StartTaskMsgReq* request)
{
	initializeEncoders();
	initializeNotifTimers();
	initEncoderSamplerTask();
	initializeSpeedTimer();
	startSpeedMeasurement();

	StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = true;
	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
}


void handleGetCounter(EncoderGetCounterMsgReq* request)
{

	if(request->encoderId >= ENCODERS_NUMBER)
		return;

	int64_t counter = getRotationsCounter(g_encoders[request->encoderId]);

	EncoderGetCounterMsgRsp* response = (EncoderGetCounterMsgRsp*) pvPortMalloc(sizeof(EncoderGetCounterMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_ENCODER_GET_COUNTER_MSG_RSP;

	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);

	vPortFree(request);
}

void handleGetSpeed(EncoderGetSpeedMsgReq* request)
{

	if(request->encoderId >= ENCODERS_NUMBER)
		return;

	uint64_t speed = getEncoderSpeed(g_encoders[request->encoderId]);

	EncoderGetSpeedMsgRsp* response = (EncoderGetSpeedMsgRsp*) pvPortMalloc(sizeof(EncoderGetSpeedMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_ENCODER_GET_SPEED_MSG_RSP;

	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);

	vPortFree(request);
}

void handleNotifyAfterRotations(EncoderNotifyAfterRotationsMsgReq* request)
{
	logger(Debug, Log_Encoders, "[handleNotifyAfterRotations] received request");

	uint8_t encoderId = request->encoderId;

	if(encoderId >= ENCODERS_NUMBER)
		return;

	int64_t startCounter = getRotationsCounter(g_encoders[encoderId]);

	switch(request->direction)
	{
	case ENCODER_DIR_FWD:
		rotationNotifData[encoderId].threshold = startCounter + request->rotations;
		break;
	case ENCODER_DIR_REV:
		rotationNotifData[encoderId].threshold = startCounter - request->rotations;
		break;
	default:
		return;
	}

	rotationNotifData[encoderId].startCounter = startCounter;
	rotationNotifData[encoderId].notifReceiver = request->sender;
	rotationNotifData[encoderId].expectedDirection = request->direction;

	if(!rotationNotifData[encoderId].isActive)
	{
//		logger(Warning, Log_Encoders, "[handleNotifyAfterRotations] Notification timer is already active");
	    if( xTimerStart( rotationNotifData[encoderId].notifTimer, 0 ) != pdPASS )
	    {
	    	logger(Error, Log_Encoders, "[handleNotifyAfterRotations] Couldn't start timer");
	    }
	}


    rotationNotifData[encoderId].isActive = true;

	vPortFree(request);
}

void notifyAfterRotationsCallback(TimerHandle_t pxTimer)
{
	uint8_t encoderId = (uint8_t) pvTimerGetTimerID( pxTimer );
	int64_t currCounter = getRotationsCounter(g_encoders[encoderId]);

	switch(rotationNotifData[encoderId].expectedDirection)
	{
	case ENCODER_DIR_FWD:
		if(currCounter < rotationNotifData[encoderId].threshold)
			return;
		break;
	case ENCODER_DIR_REV:
		if(currCounter > rotationNotifData[encoderId].threshold)
			return;
		break;
	default:
		return;
	}

	EncoderNotifyAfterRotationsMsgRsp* response = (EncoderNotifyAfterRotationsMsgRsp*) pvPortMalloc(sizeof(EncoderNotifyAfterRotationsMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP;
	response->encoderId = encoderId;
	response->actualRotations = currCounter - rotationNotifData[encoderId].startCounter;

	if(response->actualRotations < 0)
		response->actualRotations *= (-1);

	msgRespond(rotationNotifData[encoderId].notifReceiver, &response, 10);
	rotationNotifData[encoderId].isActive = false;

	logger(Debug, Log_Encoders, "[notifyAfterRotationsCallback] number of rotations reached, send notification");
	xTimerStop( pxTimer, 0 );
}

void measureSpeedCallback(TimerHandle_t pxTimer)
{
	for(int i = 0; i != ENCODERS_NUMBER; ++i)
	{
		measureEncoderSpeed(g_encoders[i], SPEED_TIMER_PERIOD);
		checkSpeedNotif(i);
	}
}

void handleNotifyAfterSpeed(EncoderNotifyAfterSpeedMsgReq* request)
{
	logger(Debug, Log_Encoders, "[handleNotifyAfterSpeed] received request");

	uint8_t encoderId = request->encoderId;

	if(encoderId >= ENCODERS_NUMBER)
		return;

//	if(speedNotifData[encoderId].isActive)
//	{
//		logger(Warning, Log_Encoders, "[handleNotifyAfterSpeed] Notification timer is already active");
//		return;
//	}

	speedNotifData[encoderId].threshold = request->speed;
	speedNotifData[encoderId].greaterOrLess =
			((request->speed > getEncoderSpeed(g_encoders[encoderId])) ?
					ENCODER_SPEED_GRATER : ENCODER_SPEED_LESS);
	speedNotifData[encoderId].notifReceiver = request->sender;
    speedNotifData[encoderId].isActive = true;

	vPortFree(request);
}

void checkSpeedNotif(uint8_t encoderId)
{
	if(!speedNotifData[encoderId].isActive)
		return;

	bool isConditionMet = false;
	uint64_t currSpeed = getEncoderSpeed(g_encoders[encoderId]);


	switch(speedNotifData[encoderId].greaterOrLess)
	{
	case ENCODER_SPEED_LESS:
		isConditionMet = currSpeed <= speedNotifData[encoderId].threshold;
		break;
	case ENCODER_SPEED_GRATER:
		isConditionMet = currSpeed >= speedNotifData[encoderId].threshold;
		break;
	default:
		speedNotifData[encoderId].isActive = false;
	}

	if(!isConditionMet)
		return;

	EncoderNotifyAfterSpeedMsgRsp* response = (EncoderNotifyAfterSpeedMsgRsp*) pvPortMalloc(sizeof(EncoderNotifyAfterSpeedMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP;
	response->encoderId = encoderId;
	response->actualSpeed = currSpeed;

	msgRespond(speedNotifData[encoderId].notifReceiver, &response, 10);
	speedNotifData[encoderId].isActive = false;

}


