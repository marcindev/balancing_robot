#ifndef WHEEL_H
#define WHEEL_H

#include <stdbool.h>
#include <stdint.h>


#define WHEEL_LEFT          0
#define WHEEL_RIGHT         1

bool wheelSetSpeed(uint8_t wheelId, float speed);
bool wheelSetAcceleration(uint8_t wheelId, float acceleration);
bool wheelRun(uint8_t wheelId, uint8_t direction, float rotations);
bool wheelRunExt(uint8_t wheelId, uint8_t direction, float rotations);
bool wheelStop(uint8_t wheelId);


#endif // WHEEL_H
