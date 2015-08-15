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
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "global_defs.h"
#include "motor.h"
#include "motorsTask.h"
#include "logger.h"

#define MOTORS_TASK_STACK_SIZE		200        // Stack size in words
#define MOTORS_QUEUE_SIZE			 10
#define MOTORS_ITEM_SIZE			  4			// bytes

#define PWM_INITIAL_FREQUENCY	  10000
#define MOTORS_NUMBER				  2

#define MOTOR_L_PWM			    PWM_0
#define MOTOR_L_FW_PORT			GPIOEXP_PORTA
#define MOTOR_L_FW_PIN			GPIOEXP_PIN0
#define MOTOR_L_RV_PORT			GPIOEXP_PORTA
#define MOTOR_L_RV_PIN			GPIOEXP_PIN1

#define MOTOR_R_PWM			    PWM_1
#define MOTOR_R_FW_PORT			GPIOEXP_PORTA
#define MOTOR_R_FW_PIN			GPIOEXP_PIN2
#define MOTOR_R_RV_PORT			GPIOEXP_PORTA
#define MOTOR_R_RV_PIN			GPIOEXP_PIN3


static MsgQueueId g_motorsQueue;
static MotorInstance g_motors[MOTORS_NUMBER];
static GpioExpander g_gpioExpander;

static void initializeMotors();
static void handleMessages(void* msg);
static void handleStart(MotorStartMsgReq* request);
static void handleStop(MotorStopMsgReq* request);
static void handleSetDutyCycle(MotorSetDutyCycleMsgReq* request);
static void handleSetDirection(MotorSetDirectionMsgReq* request);


static void motorsTask()
{
	initializePwm(PWM_INITIAL_FREQUENCY);
	initializeMotors();

	while(true)
	{
		void* msg;
		if(msgReceive(g_motorsQueue, &msg, MSG_WAIT_LONG_TIME))
		{
			handleMessages(msg);
		}

	}

}

bool motorsTaskInit()
{
	g_motorsQueue = registerMainMsgQueue(Msg_MotorsTaskID, MOTORS_QUEUE_SIZE);

    if(xTaskCreate(motorsTask, (signed portCHAR *)"Motors", MOTORS_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_MOTORS_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeMotors()
{
	g_gpioExpander.hwAddress = GPIO_EXPANDER1_ADDRESS;
	g_motors[MOTOR_LEFT].gpioExpander = &g_gpioExpander;
	g_motors[MOTOR_LEFT].pwm = MOTOR_L_PWM;
	g_motors[MOTOR_LEFT].portFwd = MOTOR_L_FW_PORT;
	g_motors[MOTOR_LEFT].pinFwd = MOTOR_L_FW_PIN;
	g_motors[MOTOR_LEFT].portRev = MOTOR_L_RV_PORT;
	g_motors[MOTOR_LEFT].pinRev = MOTOR_L_RV_PIN;

	g_motors[MOTOR_RIGHT].gpioExpander = &g_gpioExpander;
	g_motors[MOTOR_RIGHT].pwm = MOTOR_R_PWM;
	g_motors[MOTOR_RIGHT].portFwd = MOTOR_R_FW_PORT;
	g_motors[MOTOR_RIGHT].pinFwd = MOTOR_R_FW_PIN;
	g_motors[MOTOR_RIGHT].portRev = MOTOR_R_RV_PORT;
	g_motors[MOTOR_RIGHT].pinRev = MOTOR_R_RV_PIN;

	for(size_t i = 0; i != MOTORS_NUMBER; ++i)
		initializeMotor(&g_motors[i]);
}

void handleMessages(void* msg)
{
	switch(*((uint8_t*)msg))
	{
	case MOTOR_START_MSG_REQ:
		handleStart((MotorStartMsgReq*) msg);
		break;
	case MOTOR_STOP_MSG_REQ:
		handleStop((MotorStopMsgReq*) msg);
		break;
	case MOTOR_SET_DUTY_CYCLE_MSG_REQ:
		handleSetDutyCycle((MotorSetDutyCycleMsgReq*) msg);
		break;
	case MOTOR_SET_DIRECTION_MSG_REQ:
		handleSetDirection((MotorSetDirectionMsgReq*) msg);
		break;
	default:
		logger(Warning, Log_Motors, "[handleMessages] Received not recognized message");
		break;
	}
}

void handleStart(MotorStartMsgReq* request)
{
	if(request->motorId >= MOTORS_NUMBER)
		return;

	if(!startMotor(&g_motors[request->motorId]))
		logger(Error, Log_Motors, "[handleStart] Could not start the motor");

	vPortFree(request);
}

void handleStop(MotorStopMsgReq* request)
{
	if(request->motorId >= MOTORS_NUMBER)
		return;

	if(!stopMotor(&g_motors[request->motorId]))
		logger(Error, Log_Motors, "[handleStart] Could not stop the motor");

	vPortFree(request);
}


void handleSetDutyCycle(MotorSetDutyCycleMsgReq* request)
{

	if(request->motorId >= MOTORS_NUMBER)
		return;

	setMotorDutyCycle(&g_motors[request->motorId], request->dutyCycle);

	vPortFree(request);
}

void handleSetDirection(MotorSetDirectionMsgReq* request)
{

	if(request->motorId >= MOTORS_NUMBER)
		return;

	setMotorDirection(&g_motors[request->motorId], request->direction);

	vPortFree(request);
}

