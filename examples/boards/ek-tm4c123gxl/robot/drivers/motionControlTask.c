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
#include "motionControlTask.h"

#define MOTION_CONTROL_TASK_STACK_SIZE		200        // Stack size in words

#define LONG_TIME 0xffff

extern SemaphoreHandle_t g_timerB0TimoutSem;

static void motionControlTask()
{
	uint8_t wdgTaskID = registerToWatchDog();
	initInterrupts();

	while(true)
	{
		feedWatchDog(wdgTaskID, WDG_ASLEEP);
		BaseType_t result = xSemaphoreTake(g_timerB0TimoutSem, LONG_TIME);
		feedWatchDog(wdgTaskID, WDG_ALIVE);

		if(result == pdTRUE)
			MCtrlDoJob();
	}
}

bool motionControlTaskInit()
{

    if(xTaskCreate(motionControlTask, (signed portCHAR *)"MotionControl", MOTION_CONTROL_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_MOTION_CONTROL_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}
