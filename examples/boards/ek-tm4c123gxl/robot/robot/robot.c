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
#include "FreeRTOS.h"
#include "portable.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "robot.h"
#include "priorities.h"
#include "drivers/i2c/i2cTask.h"


// local globals

#define ROBOT_TASK_STACK_SIZE		300         // Stack size in words

I2cManager g_i2cManager;
GpioExpander g_gpioExpander;

const HeapRegion_t xHeapRegions[] =
{
    { ( uint8_t * ) 0x20006000UL, 0x02000 },
    { NULL, 0 } /* Terminates the array. */
};

void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{

    while(1)
    {
    }
}

void initializeSysClock();
void initilizeFreeRTOS();
void initializeGpioExpanders();
bool initRobotTask();

void runRobot()
{
	initializeRobot();
	if(!i2cTaskInit())
	{
		while(1){ }
	}
	if(!initRobotTask())
	{
		while(1){ }
	}

	vTaskStartScheduler();
}

void initializeRobot()
{
	initializeSysClock();
	enableInterrupts();
	initilizeFreeRTOS();

}

static void robotTask(void *pvParameters)
{
	portTickType ui32WakeTime;
	initializeGpioExpanders();

	ui32WakeTime = xTaskGetTickCount();

	GpioExpSetPinDirOut(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN1);

	while(true)
	{
		GpioExpSetPin(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN1);
		vTaskDelayUntil(&ui32WakeTime, 10 / portTICK_RATE_MS);
		GpioExpClearPin(&g_gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN1);
		vTaskDelayUntil(&ui32WakeTime, 10 / portTICK_RATE_MS);
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


void initializeGpioExpanders()
{
	ZeroBuffer(&g_gpioExpander, sizeof(GpioExpander));

	// setup gpio expander
	g_gpioExpander.hwAddress		= 0x20;
	g_gpioExpander.i2cManager 		= &g_i2cManager;
	GpioExpInit(&g_gpioExpander);
}

void initilizeFreeRTOS()
{
	vPortDefineHeapRegions( xHeapRegions );
}
