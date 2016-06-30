#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

typedef enum
{
    WDG_ALIVE,
    WDG_ASLEEP,
    WDG_UNKNOWN
}WdgTaskState;

void initWatchDog();
int16_t registerToWatchDog();
void feedWatchDog(uint8_t taskId, WdgTaskState state);
void onWatchDogTimeout();
void setIsDumpPM(bool isOn);
