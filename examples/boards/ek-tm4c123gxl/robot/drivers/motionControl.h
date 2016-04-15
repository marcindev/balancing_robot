#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

void MCtrlInitialize();
void MCtrlDoJob();
void MCtrlSetPeriod(uint32_t period);


#endif // MOTION_CONTROL_H
