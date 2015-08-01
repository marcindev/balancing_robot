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
#include "priorities.h"
#include "drivers/i2c/i2cTask.h"
#include "drivers/motorsTask.h"
#include "server/tcpServerTask.h"
#include "interrupts/interrupts.h"
#include "drivers/encoder.h"


// local globals

#define ROBOT_TASK_STACK_SIZE		500         // Stack size in words

//static I2cManager g_i2cManager;
//static GpioExpander g_gpioExpander;

extern xQueueHandle g_motorsQueue;

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

static void initializeSysClock();
static void initilizeFreeRTOS();
static void initializeGpioExpanders();
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

	if(!motorsTaskInit())
	{
		while(1){ }
	}

	if(!initRobotTask())
	{
		while(1){ }
	}

	if(!encoderSamplerTaskInit())
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

	ZeroBuffer(gpioExpander, sizeof(GpioExpander));
	gpioExpander->i2cManager = i2cManager;
	gpioExpander->hwAddress		= 0x20;


	portTickType ui32WakeTime;
//	initializeGpioExpanders();

	ui32WakeTime = xTaskGetTickCount();

	EncoderInstance encoder;
	encoder.gpioExpander = gpioExpander;
	encoder.ch1_pin = GPIOEXP_PIN1;
	encoder.ch2_pin = GPIOEXP_PIN2;

	initEncoder(&encoder);

//	GpioExpSetPinDirOut(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN2);
//	GpioExpSetPinDirOut(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN1);

	uint8_t direction = 0;

	while(true)
	{
/*		MotorSetDirectionMsgReq* directionReq = (MotorSetDirectionMsgReq*) pvPortMalloc(sizeof(MotorSetDirectionMsgReq));

		*directionReq = INIT_MOTOR_SET_DIRECTION_MSG_REQ;
		directionReq->motorId = 0;
		directionReq->direction = direction;

		if(xQueueSend(g_motorsQueue, (void*) &directionReq, portMAX_DELAY) != pdPASS)
		{
			while(1) {}
		}

		for(int8_t dutyCycle = 2; dutyCycle <= 100; dutyCycle++)
		{
			MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));

			*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
			request->motorId = 0;
			request->dutyCycle = dutyCycle;

			if(xQueueSend(g_motorsQueue, (void*) &request, portMAX_DELAY) != pdPASS)
			{
				while(1) {}
			}

			vTaskDelayUntil(&ui32WakeTime, 100 / portTICK_RATE_MS);
		}

		for(int8_t dutyCycle = 100; dutyCycle > 1; dutyCycle--)
		{
			MotorSetDutyCycleMsgReq* request = (MotorSetDutyCycleMsgReq*) pvPortMalloc(sizeof(MotorSetDutyCycleMsgReq));

			*request = INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;
			request->motorId = 0;
			request->dutyCycle = dutyCycle;

			if(xQueueSend(g_motorsQueue, (void*) &request, portMAX_DELAY) != pdPASS)
			{
				while(1) {}
			}

			vTaskDelayUntil(&ui32WakeTime, 100 / portTICK_RATE_MS);
		}
////		GpioExpSetPin(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN1);
////		GpioExpClearPin(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN2);
////		vTaskDelayUntil(&ui32WakeTime, 50 / portTICK_RATE_MS);
////		GpioExpClearPin(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN1);
////		GpioExpSetPin(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN2);
////		vTaskDelayUntil(&ui32WakeTime, 50 / portTICK_RATE_MS);
//
		vTaskDelayUntil(&ui32WakeTime, 2000 / portTICK_RATE_MS);
		direction ^= 1;
			*/
	}

}

bool initRobotTask()
{
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
