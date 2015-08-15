//***********************************************
// Heart of robot functionality
// autor: Marcin Gozdziewski
//
//***********************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/fpu.h"
#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "robot.h"
#include "messages.h"
#include "msgSystem.h"
#include "priorities.h"
#include "drivers/i2c/i2cTask.h"
#include "server/tcpServerTask.h"
#include "interrupts/interrupts.h"
#include "drivers/encoder.h"
#include "drivers/wheelsTask.h"


// local globals

#define ROBOT_TASK_STACK_SIZE		200         // Stack size in words
#define ROBOT_TASK_QUEUE_SIZE		 20

//static I2cManager g_i2cManager;
//static GpioExpander g_gpioExpander;

const HeapRegion_t xHeapRegions[] =
{
    { ( uint8_t * ) 0x20005000UL, 0x03000 },
    { NULL, 0 } /* Terminates the array. */
};

void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{

    while(1)
    {
    }
}

static MsgQueueId g_robotQueue;

static void initializeSysClock();
static void initilizeFreeRTOS();
static bool initRobotTask();
static void enableInterrupts();
static void initializeFPU();


void runRobot()
{
	initializeRobot();
	if(!i2cTaskInit())
	{
		while(1){ }
	}

	if(!wheelsTaskInit())
	{
		while(1){ }
	}

	if(!initRobotTask())
	{
		while(1){ }
	}

//	if(!tcpServerTaskInit())
//	{
//		while(1){ }
//	}


	vTaskStartScheduler();
}

void initializeRobot()
{
	initializeFPU();
	initializeSysClock();
	enableInterrupts();
	initilizeFreeRTOS();


}

static void robotTask(void *pvParameters)
{


	I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
	GpioExpander* gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));
//
	ZeroBuffer(gpioExpander, sizeof(GpioExpander));
	gpioExpander->i2cManager = i2cManager;
	gpioExpander->hwAddress		= 0x21;
	GpioExpInit(gpioExpander);
//
	portTickType ui32WakeTime;
////	initializeGpioExpanders();
//
	ui32WakeTime = xTaskGetTickCount();
//
////	EncoderInstance encoder;
////	encoder.gpioExpander = gpioExpander;
////	encoder.ch1_pin = GPIOEXP_PIN1;
////	encoder.ch2_pin = GPIOEXP_PIN2;
////
////	initEncoder(&encoder);
//
	GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN7);
	GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN6);
//
//	uint8_t direction = 0;

	WheelSetSpeedMsgReq* wheelLeftSetSpeedReq = (WheelSetSpeedMsgReq*) pvPortMalloc(sizeof(WheelSetSpeedMsgReq));
	*wheelLeftSetSpeedReq = INIT_WHEEL_SET_SPEED_MSG_REQ;
	wheelLeftSetSpeedReq->wheelId = 0;
	wheelLeftSetSpeedReq->speed = 2.0f;

	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelLeftSetSpeedReq, portMAX_DELAY))
	{
		while(1) {}
	}

	WheelSetSpeedMsgReq* wheelRightSetSpeedReq = (WheelSetSpeedMsgReq*) pvPortMalloc(sizeof(WheelSetSpeedMsgReq));
	*wheelRightSetSpeedReq = INIT_WHEEL_SET_SPEED_MSG_REQ;
	wheelRightSetSpeedReq->wheelId = 1;
	wheelRightSetSpeedReq->speed = 1.0f;

	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelRightSetSpeedReq, portMAX_DELAY))
	{
		while(1) {}
	}


	WheelRunMsgReq* wheelLeftRunReq = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
	*wheelLeftRunReq = INIT_WHEEL_RUN_MSG_REQ;
	wheelLeftRunReq->wheelId = 0;
	wheelLeftRunReq->direction = 0;
	wheelLeftRunReq->rotations = 10.0f;

	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelLeftRunReq, portMAX_DELAY))
	{
		while(1) {}
	}

	WheelRunMsgReq* wheelRightRunReq = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
	*wheelRightRunReq = INIT_WHEEL_RUN_MSG_REQ;
	wheelRightRunReq->wheelId = 1;
	wheelRightRunReq->direction = 1;
	wheelRightRunReq->rotations = 10.0f;

	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), (void**) &wheelRightRunReq, portMAX_DELAY))
	{
		while(1) {}
	}

//	int motorsNum = 2;
//
//	for(int i = 0; i != motorsNum; ++i)
//	{
//	MotorStartMsgReq* motorStartReq = (MotorStartMsgReq*) pvPortMalloc(sizeof(MotorStartMsgReq));
//
//	*motorStartReq = INIT_MOTOR_START_MSG_REQ;
//	motorStartReq->motorId = i;
//
//	if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &motorStartReq, portMAX_DELAY))
//	{
//		while(1) {}
//	}
//	}


	while(true)
	{
//		for(int i = 0; i != motorsNum; ++i)
//		{
//		MotorSetDirectionMsgReq* directionReq = (MotorSetDirectionMsgReq*) pvPortMalloc(sizeof(MotorSetDirectionMsgReq));
//
//		*directionReq = INIT_MOTOR_SET_DIRECTION_MSG_REQ;
//		directionReq->motorId = i;
//		directionReq->direction = direction;
//
//		if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &directionReq, portMAX_DELAY))
//		{
//			while(1) {}
//		}
//		}
//
//		for(int8_t dutyCycle = 2; dutyCycle <= 100; dutyCycle++)
//		{
//			for(int i = 0; i != motorsNum; ++i)
//			{
//			MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));
//
//			*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
//			request->motorId = i;
//			request->dutyCycle = dutyCycle;
//
//			if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
//			{
//				while(1) {}
//			}
//			}
//
//			vTaskDelayUntil(&ui32WakeTime, 100 / portTICK_RATE_MS);
//		}
//
//		for(int8_t dutyCycle = 100; dutyCycle > 1; dutyCycle--)
//		{
//			for(int i = 0; i != motorsNum; ++i)
//			{
//			MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));
//
//			*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
//			request->motorId = i;
//			request->dutyCycle = dutyCycle;
//
//			if(!msgSend(g_robotQueue, getQueueIdFromTaskId(Msg_MotorsTaskID), (void**) &request, portMAX_DELAY))
//			{
//				while(1) {}
//			}
//			}
//
//			vTaskDelayUntil(&ui32WakeTime, 100 / portTICK_RATE_MS);
//		}
		GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN6);
		GpioExpClearPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN7);
		vTaskDelayUntil(&ui32WakeTime, 50 / portTICK_RATE_MS);
		GpioExpClearPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN6);
		GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN7);
		vTaskDelayUntil(&ui32WakeTime, 50 / portTICK_RATE_MS);
//
//		vTaskDelayUntil(&ui32WakeTime, 2000 / portTICK_RATE_MS);
//		direction ^= 1;

	}

}

bool initRobotTask()
{
	g_robotQueue = registerMainMsgQueue(Msg_RobotTaskID, ROBOT_TASK_QUEUE_SIZE);

    if(xTaskCreate(robotTask, (signed portCHAR *)"ROBOT", ROBOT_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_ROBOT_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeFPU()
{
    FPUEnable();
    FPULazyStackingEnable();
}

void initializeSysClock()
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);
}

void enableInterrupts()
{
	// Enable processor interrupts.
	IntMasterEnable();
}


//void initializeGpioExpanders()
//{
//	ZeroBuffer(&g_gpioExpander, sizeof(GpioExpander));
//
//	// setup gpio expander
//	g_gpioExpander.hwAddress		= 0x20;
//	g_gpioExpander.i2cManager 		= &g_i2cManager;
//	GpioExpInit(&g_gpioExpander);
//}

void initilizeFreeRTOS()
{
	vPortDefineHeapRegions( xHeapRegions );
}
