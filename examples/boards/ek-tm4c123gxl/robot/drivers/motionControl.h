#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

void MCtrlInitialize();
void MCtrlDoJob();
void MCtrlSetPeriod(uint32_t period);
void MCtrlSetPidParam(uint8_t param, float value);
void MCtrlSetPidDir(uint8_t direction);
float MCtrlGetData(uint8_t param);


#endif // MOTION_CONTROL_H
