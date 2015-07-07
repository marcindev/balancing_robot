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
#include "driverlib/uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "robot.h"

// local globals
static I2CComInstance g_i2cInstance;


const HeapRegion_t xHeapRegions[] =
{
    { ( uint8_t * ) 0x20004000UL, 0x02000 },
    { ( uint8_t * ) 0x20006000UL, 0x02000 },
    { NULL, 0 } /* Terminates the array. */
};

initializeSysClock();
initializeI2c();
initializeGpioExpanders();

void runRobot()
{
	initializeRobot();
}

void initializeRobot()
{
	initializeSysClock();
	initilizeFreeRTOS();
	initializeI2c();
	initializeGpioExpanders();
}


void initializeSysClock()
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);
}

void initializeI2c()
{
	ZeroBuffer(&g_i2cInstance, sizeof(I2CComInstance));

	// setup i2c communication
	g_i2cInstance.gpioPeripheral 	= SYSCTL_PERIPH_GPIOA;
	g_i2cInstance.gpioPortBase 		= GPIO_PORTA_BASE;
	g_i2cInstance.i2cBase 			= I2C1_BASE;
	g_i2cInstance.i2cPeripheral 	= SYSCTL_PERIPH_I2C1;
	g_i2cInstance.sclPin 			= GPIO_PIN_6;
	g_i2cInstance.sclPinConfig 		= GPIO_PA6_I2C1SCL;
	g_i2cInstance.sdaPin 			= GPIO_PIN_7;
	g_i2cInstance.sdaPinConfig		= GPIO_PA7_I2C1SDA;
	g_i2cInstance.speed				= I2C_SPEED_400;

	I2CComInit(&g_i2cInstance);
}

void initializeGpioExpanders()
{
	ZeroBuffer(&g_gpioExpander, sizeof(GpioExpander));

	// setup gpio expander
	g_gpioExpander.hwAddress		= 0x20;
	g_gpioExpander.i2cInstance 		= &g_i2cInstance;

	GpioExpInit(&g_gpioExpander);
}

void initilizeFreeRTOS()
{
	vPortDefineHeapRegions( xHeapRegions );
}
