#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"


void initWatchDog();
int16_t registerToWatchDog();
void feedWatchDog(uint8_t taskId);
void onWatchDogTimeout();
void setIsDumpPM(bool isOn);
