//****************************
// Utility functions
// autor: Marcin Gozdziewski
// ***************************
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"


void ZeroBuffer(void* buffer, int len)
{
    unsigned char* uchPtr = (unsigned char*) buffer;

    for(int i = 0; i != len; i++)
    {
        *uchPtr++ = 0;
    }
}

void printBuffer(void* buffer, int len)
{
    UARTprintf("Buffer:");
    unsigned char* uchPtr = (unsigned char*) buffer;

    for(int i = 0; i < len; ++i)
    {
        UARTprintf(" 0x%02X", *uchPtr++);
    }
    UARTprintf("\n");
}

void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}


void* getTaskHandleByNum(int taskNumber)
{
    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize;
    unsigned long ulTotalRunTime;
    TaskHandle_t taskHandle = NULL;

    uxArraySize = uxTaskGetNumberOfTasks();

    pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

    if(!pxTaskStatusArray)
        return NULL;

    uxArraySize = uxTaskGetSystemState( pxTaskStatusArray,
                                        uxArraySize,
                                        &ulTotalRunTime );

    for(int i = 0; i != uxArraySize; ++i)
    {
        if(pxTaskStatusArray[i].xTaskNumber == taskNumber)
        {
            taskHandle = pxTaskStatusArray[i].xHandle;
            break;
        }
    }

    vPortFree( pxTaskStatusArray );

    return taskHandle;
}
