#ifndef MPU6050_H
#define MPU6050_H

#include <stdbool.h>
#include <stdint.h>
#include "i2c/i2cManager.h"



typedef struct
{
    I2cManager* i2cManager;
    uint8_t hwAddress;
    bool initiated;
} Mpu6050;


typedef struct
{
    int16_t acX,
            acY,
            acZ,
            tmp,
            gyX,
            gyY,
            gyZ;

} MpuRawData;

typedef struct
{
    float x_axis;
    float y_axis;
    bool isDataValid;
} AccelAngles;

typedef struct
{
    float x;
    float y;
    float z;
} GyroRates;

void MpuInit(Mpu6050* mpu);
bool MpuUpdateData(Mpu6050* mpu);
AccelAngles MpuGetAccelAngles(Mpu6050* mpu);
GyroRates MpuGetGyroRates(Mpu6050* mpu);
bool MpuSetDLPF(Mpu6050* mpu, uint8_t val);
bool MpuRegWrite(Mpu6050* mpu, uint8_t reg, uint8_t value);
bool MpuRegRead(Mpu6050* mpu, uint8_t reg, uint8_t* value);
bool MpuMRegRead(Mpu6050* mpu, uint8_t startReg, uint8_t* values, uint8_t num);

#endif // MPU6050_H
