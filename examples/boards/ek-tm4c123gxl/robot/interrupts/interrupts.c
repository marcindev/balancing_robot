
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
#include "driverlib/ssi.h"
#include "interrupts.h"
#include "spiWrapper.h"
#include "utils.h"


#define GPIOEXP1_PORTB_INT_PIN			GPIO_PIN_1
#define MPU_INT_PIN						GPIO_PIN_2
#define SSI_RX_INT_PIN					GPIO_PIN_3
#define GPIOEXP1_PORTB_INT_ACT_PIN		GPIO_INT_PIN_1
#define MPU_INT_ACT_PIN					GPIO_INT_PIN_2
#define SSI_RX_INT_ACT_PIN				GPIO_INT_PIN_3

#define INTERRUPT_PINS			GPIOEXP1_PORTB_INT_PIN | SSI_RX_INT_PIN // | MPU_INT_PIN
#define INT_ACT_PINS			GPIOEXP1_PORTB_INT_ACT_PIN | SSI_RX_INT_ACT_PIN // | MPU_INT_ACT_PIN

SemaphoreHandle_t g_gpioExp1PortBIntSem  = NULL;
SemaphoreHandle_t g_ssiRxIntSem  = NULL;
extern SpiComInstance* g_spiComInstServer;

void initInterrupts()
{
	static bool initialized = false;

	if(initialized)
		return;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, INTERRUPT_PINS);
	GPIOIntTypeSet(GPIO_PORTE_BASE, INTERRUPT_PINS, GPIO_RISING_EDGE);//GPIO_BOTH_EDGES);

	GPIOIntEnable(GPIO_PORTE_BASE, INT_ACT_PINS);
	// must be greater or equal to configMAX_SYSCALL_INTERRUPT_PRIORITY
	// for the FREERTOS to work properly
	IntPrioritySet(INT_GPIOE, 5 << 5);	// priority 5 (3 top bits)
	IntEnable(INT_GPIOE);

	g_gpioExp1PortBIntSem = xSemaphoreCreateBinary();
	g_ssiRxIntSem = xSemaphoreCreateBinary();

	initialized = true;
}

void GPIOE_intHandler(void)
{
	uint32_t intStatus = GPIOIntStatus(GPIO_PORTE_BASE, false);
	GPIOIntClear(GPIO_PORTE_BASE, INT_ACT_PINS);
	static BaseType_t xHigherPriorityTaskWoken;
	if(intStatus & GPIOEXP1_PORTB_INT_ACT_PIN)
	{
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR( g_gpioExp1PortBIntSem, &xHigherPriorityTaskWoken );
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

		return;
	}

//	if(intStatus & MPU_INT_ACT_PIN)
//	{
//
//	}

	if(intStatus & SSI_RX_INT_ACT_PIN)
	{
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR( g_ssiRxIntSem, &xHigherPriorityTaskWoken );
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

		return;
	}
}

//Interrupt for SSI0
void SSI0_intHandler(void)
{
  uint32_t intStatus;
  //Get the interrrupt status.
  intStatus = SSIIntStatus(SSI0_BASE, true);
  // Clear the asserted interrupts.
  SSIIntClear(SSI0_BASE, intStatus);

  if(g_spiComInstServer)
  {
	  onDmaTransactionRxEnd(g_spiComInstServer);
	  onDmaTransactionTxEnd(g_spiComInstServer);
  }

//  if(intStatus==SSI_RXFF)
//  {
//		static BaseType_t xHigherPriorityTaskWoken;
//		xHigherPriorityTaskWoken = pdFALSE;
//		xSemaphoreGiveFromISR( g_ssiRxIntSem, &xHigherPriorityTaskWoken );
//		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
//
//		return;
//  }
}

void uDMAErrorHandler(void)
{
	uint32_t status;

    //
    // Check for uDMA error bit
    //
	status = uDMAErrorStatusGet();

    //
    // If there is a uDMA error, then clear the error and continue.  If we're
    // still debugging our project, we want to infinte loop here so we can
    // investiage the failure cause.
    //
    if(status)
    {
        uDMAErrorStatusClear();
        //while(1);
    }
}
