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
#include "utils.h"
#include "global_defs.h"
#include "motor.h"
#include "motorsTask.h"

#define MOTORS_TASK_STACK_SIZE		300        // Stack size in words
#define MOTORS_QUEUE_SIZE			 50
#define MOTORS_ITEM_SIZE			  4			// bytes

#define PWM_INITIAL_FREQUENCY		10000
#define MOTORS_NUMBER				  1

#define MOTOR_L_EN_PORT			GPIOEXP_PORTA
#define MOTOR_L_EN_PIN			GPIOEXP_PIN1
#define MOTOR_L_FW_PORT			GPIOEXP_PORTA
#define MOTOR_L_FW_PIN			GPIOEXP_PIN2
#define MOTOR_L_RV_PORT			GPIOEXP_PORTA
#define MOTOR_L_RV_PIN			GPIOEXP_PIN3

//#define MOTOR_R_EN_PORT
//#define MOTOR_R_EN_PIN
//#define MOTOR_R_FW_PORT
//#define MOTOR_R_FW_PIN
//#define MOTOR_R_RV_PORT
//#define MOTOR_R_RV_PIN

extern uint32_t g_pwmCounter;

xQueueHandle g_motorsQueue;
SemaphoreHandle_t g_pwmCounterSem = NULL;
static MotorInstance g_motors[MOTORS_NUMBER];
static GpioExpander g_gpioExpander;

static void initializeMotors();
static void motorsJob();
static void handleMessages(void* msg);
static void handleSetDutyCycle(MotorSetDutyCycleMsgReq* request);


static void motorsTask()
{
	g_pwmCounterSem = xSemaphoreCreateBinary();

	initializePwm(PWM_INITIAL_FREQUENCY);
	initializeMotors();

	while(true)
	{
		void* msg;
		if(xQueueReceive(g_motorsQueue, &msg, 0) == pdPASS)
		{
			handleMessages(msg);
		}

		if( xSemaphoreTake( g_pwmCounterSem, portMAX_DELAY ) == pdTRUE )
			motorsJob();	// wait for timer to give semaphore
	}

}

bool motorsTaskInit()
{
	g_motorsQueue = xQueueCreate(MOTORS_QUEUE_SIZE, MOTORS_ITEM_SIZE);

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
	g_motors[MOTOR_LEFT].portEna = MOTOR_L_EN_PORT;
	g_motors[MOTOR_LEFT].pinEna = MOTOR_L_EN_PIN;
	g_motors[MOTOR_LEFT].portFwd = MOTOR_L_FW_PORT;
	g_motors[MOTOR_LEFT].pinFwd = MOTOR_L_FW_PIN;
	g_motors[MOTOR_LEFT].portRev = MOTOR_L_RV_PORT;
	g_motors[MOTOR_LEFT].pinRev = MOTOR_L_RV_PIN;
	//g_motors[MOTOR_RIGHT].gpioExpander = &g_gpioExpander;

	for(size_t i = 0; i != MOTORS_NUMBER; ++i)
		initializeMotor(&g_motors[i]);
}

void motorsJob()
{
	for(size_t i = 0; i != MOTORS_NUMBER; ++i)
	{
		uint8_t dutyCycle = getMotorDutyCycle(&g_motors[i]);

		if(g_pwmCounter == 0)
		{
			if(dutyCycle == 0 && isMotorRunning(&g_motors[i]))
			{
				stopMotor(&g_motors[i]);
			}
			else if (!isMotorRunning(&g_motors[i]))
			{
				startMotor(&g_motors[i]);
			}
		}
		else if(g_pwmCounter == dutyCycle)
		{
			//if(dutyCycle != 100)
			stopMotor(&g_motors[i]);
		}

	}
}

void handleMessages(void* msg)
{
	switch(*((uint8_t*)msg))
	{
	case MOTOR_SET_DUTY_CYCLE_MSG_REQ:
		handleSetDutyCycle((MotorSetDutyCycleMsgReq*) msg);
		break;

	default:
		// Received not-recognized message
		break;
	}
}


void handleSetDutyCycle(MotorSetDutyCycleMsgReq* request)
{

	if(request->motorId >= MOTORS_NUMBER)
		return;

	setMotorDutyCycle(&g_motors[request->motorId], request->dutyCycle);

	vPortFree(request);
}

