//*****************************************************************************
//
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "utils.h"
#include "wdg.h"
#include "encoder.h"
#include "encoderSamplerTask.h"

#define ENCODER_SAMPLER_TASK_STACK_SIZE		100        // Stack size in words

#define LONG_TIME 0xffff

extern SemaphoreHandle_t g_gpioExp1PortBIntSem;

static void encoderSamplerTask()
{
	uint8_t wdgTaskID = registerToWatchDog();
	initInterrupts();

	while(true)
	{
		feedWatchDog(wdgTaskID, WDG_ASLEEP);
		BaseType_t result = xSemaphoreTake(g_gpioExp1PortBIntSem, LONG_TIME);
		feedWatchDog(wdgTaskID, WDG_ALIVE);

		if(result == pdTRUE)
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
