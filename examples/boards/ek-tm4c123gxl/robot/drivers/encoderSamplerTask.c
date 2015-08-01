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
#include "encoder.h"
#include "encoderSamplerTask.h"

#define ENCODER_SAMPLER_TASK_STACK_SIZE		200        // Stack size in words

#define LONG_TIME 0xffff

extern SemaphoreHandle_t g_gpioExp1PortBIntSem;

static void encoderSamplerTask()
{
	initInterrupts();
	while(true)
	{
		if(xSemaphoreTake(g_gpioExp1PortBIntSem, LONG_TIME) == pdTRUE)
			doEncoderJob();
	}
}

bool encoderSamplerTaskInit()
{

    if(xTaskCreate(encoderSamplerTask, (signed portCHAR *)"EncoderSampler", ENCODER_SAMPLER_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_ENCODER_SAMPLER_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}
