
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "interrupts.h"


#define GPIOEXP1_PORTB_INT_PIN			GPIO_PIN_1
#define MPU_INT_PIN						GPIO_PIN_2
#define GPIOEXP1_PORTB_INT_ACT_PIN		GPIO_INT_PIN_1
#define MPU_INT_ACT_PIN					GPIO_INT_PIN_2

#define INTERRUPT_PINS			GPIOEXP1_PORTB_INT_PIN // | MPU_INT_PIN
#define INT_ACT_PINS			GPIOEXP1_PORTB_INT_ACT_PIN // | MPU_INT_ACT_PIN

SemaphoreHandle_t g_gpioExp1PortBIntSem  = NULL;

void initInterrupts()
{
	static bool initialized = false;

	if(initialized)
		return;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, INTERRUPT_PINS);
	GPIOIntTypeSet(GPIO_PORTE_BASE, INTERRUPT_PINS, GPIO_BOTH_EDGES);

	GPIOIntEnable(GPIO_PORTE_BASE, INT_ACT_PINS);
	// must be greater or equal to configMAX_SYSCALL_INTERRUPT_PRIORITY
	// for the FREERTOS to work properly
	IntPrioritySet(INT_GPIOE, 5 << 5);	// priority 5 (3 top bits)
	IntEnable(INT_GPIOE);

	g_gpioExp1PortBIntSem = xSemaphoreCreateBinary();

	initialized = true;
}

void GPIOE_intHandler(void)
{
	uint32_t intStatus = GPIOIntStatus(GPIO_PORTE_BASE, false);
	GPIOIntClear(GPIO_PORTE_BASE, INT_ACT_PINS);

	if(intStatus & GPIOEXP1_PORTB_INT_ACT_PIN)
	{
		static BaseType_t xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR( g_gpioExp1PortBIntSem, &xHigherPriorityTaskWoken );
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

		return;
	}

//	if(intStatus & MPU_INT_ACT_PIN)
//	{
//
//	}
}
