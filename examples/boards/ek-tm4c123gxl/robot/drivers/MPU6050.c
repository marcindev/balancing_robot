#include "MPU6050.h"
#include "MPU6050RegMap.h"
#include "logger.h"
#include <math.h>


#define MPU_I2C_ADDRESS         0x68
#define WRITE_MSG_SIZE          2
#define GYRO_CALIB_ITERS        1000

typedef struct
{
    int16_t x_offset,
            y_offset,
            z_offset;
} GyroCalib;


/* Sensitivity for gyro
FS_SEL  Full Scale Range    LSB Sensitivity
0       ± 250 °/s         131 LSB/°/s
1       ± 500 °/s         65.5 LSB/°/s
2       ± 1000 °/s            32.8 LSB/°/s
3       ± 2000 °/s            16.4 LSB/°/s
*/

static GyroCalib g_gyroCalib;
static float g_gyroSensitivity;
static AccelAngles g_accelAngles;
static GyroRates g_gyroRates;

static bool MpuTest(Mpu6050* mpu);
static bool MpuConfigure(Mpu6050* mpu);
static bool MpuCalibrateGyros(Mpu6050* mpu);
static bool MpuReadRawData(Mpu6050* mpu, MpuRawData* rawData);
static AccelAngles calcAccelAngles(const MpuRawData* rawData);
static GyroRates calcGyroRates(const MpuRawData* rawData);
static float getGyroSensitivity(Mpu6050* mpu);

void MpuInit(Mpu6050* mpu)
{
    if(mpu->initiated)
        return;

    if(!initI2cManager(mpu->i2cManager))
    {
        mpu->initiated = false;
        return;
    }

    mpu->hwAddress = MPU_I2C_ADDRESS;

    mpu->initiated = true;

    bool testResult = MpuTest(mpu);

    if(!testResult)
    {
        logger(Error, Log_Mpu, "[MpuInit] Mpu test failed");
        mpu->initiated = false;
        return;
    }

    uint8_t regAddr = PWR_MGMT_1,
            regVal = 0x0; // wake up the device

    if(!MpuRegWrite(mpu, regAddr, regVal))
    {
        logger(Error, Log_Mpu, "[MpuInit] Couldn't write to PWR_MGMT_1 register");
        mpu->initiated = false;
        return;
    }

    if(!MpuConfigure(mpu))
    {
        logger(Error, Log_Mpu, "[MpuInit] Couldn't configure mpu");
        mpu->initiated = false;
        return;
    }

    if(!MpuCalibrateGyros(mpu))
    {
        logger(Error, Log_Mpu, "[MpuInit] Couldn't calibrate gyros");
        mpu->initiated = false;
        return;
    }

    logger(Info, Log_Mpu, "[MpuInit] Mpu initialized successfully");


}



bool MpuConfigure(Mpu6050* mpu)
{
    bool result = true;

    //Sets sample rate to 8000/1+7 = 1000Hz
    result &= MpuRegWrite(mpu, SMPLRT_DIV, 0x7);
    //Disable FSync, 256Hz DLPF
    result &= MpuRegWrite(mpu, CONFIG, 0x0);
    //Disable gyro self tests, scale of 500 degrees/s
    result &= MpuRegWrite(mpu, GYRO_CONFIG, 0b00001000);
    //Disable accel self tests, scale of +-2g, no DHPF
    result &= MpuRegWrite(mpu, ACCEL_CONFIG, 0x0);
    //Motion threshold of 0mg
    result &= MpuRegWrite(mpu, MOT_THR, 0x00);
    //Disable sensor output to FIFO buffer
    result &= MpuRegWrite(mpu, FIFO_EN, 0x00);

    //Setup INT pin and AUX I2C pass through
    result &= MpuRegWrite(mpu, INT_PIN_CFG, 0x00);
    //Enable data ready interrupt
    result &= MpuRegWrite(mpu, INT_ENABLE, 0x00);
    //Reset sensor signal paths
    result &= MpuRegWrite(mpu, SIGNAL_PATH_RESET, 0x00);
    //Motion detection control
    result &= MpuRegWrite(mpu, MOT_DETECT_CTRL, 0x00);
    //Disables FIFO, AUX I2C, FIFO and I2C reset bits to 0
    result &= MpuRegWrite(mpu, USER_CTRL, 0x00);
    //Sets clock source to gyro reference w/ PLL
    result &= MpuRegWrite(mpu, PWR_MGMT_1, 0b00000010);
    //Controls frequency of wakeups in accel low power mode plus the sensor standby modes
    result &= MpuRegWrite(mpu, PWR_MGMT_2, 0x00);

    //Data transfer to and from the FIFO buffer
    result &= MpuRegWrite(mpu, FIFO_R_W, 0x00);


    // set gyro sensitivity based on scale range
    g_gyroSensitivity = getGyroSensitivity(mpu);
    logger(Debug, Log_Mpu, "[MpuConfigure] gyro sensitivity: %f", g_gyroSensitivity);

    return result;
}

bool MpuSetDLPF(Mpu6050* mpu, uint8_t val)
{
    uint8_t regVal = 0;

    if(!MpuRegRead(mpu, CONFIG, &regVal))
        return false;

    regVal |= CONFIG__DLPF_CFG(val);

    return MpuRegWrite(mpu, CONFIG, regVal);
}

float getGyroSensitivity(Mpu6050* mpu)
{
    uint8_t regVal = 0;

    if(!MpuRegRead(mpu, GYRO_CONFIG, regVal))
    {
        logger(Error, Log_Mpu, "[getGyroSensitivity] couldn't read register");
        return 1.0f;
    }

    const uint8_t FS_SEL_SHIFT = 3,
                  FS_SEL  = 0x3;

    uint8_t fsSelVal = (regVal >> FS_SEL_SHIFT) & FS_SEL;
    logger(Debug, Log_Mpu, "[getGyroSensitivity] fsSelVal: %d", (int)regVal);

    switch(fsSelVal)
    {
    case 0:
        return 131.0f;
    case 1:
        return 65.5f;
    case 2:
        return 32.8f;
    case 3:
        return 16.4f;
    default:
        break;
    }

    return 1.0f; // won't get here
}


bool MpuReadRawData(Mpu6050* mpu, MpuRawData* rawData)
{
    if(!mpu->initiated)
        return false;

    // reading 14 registers starting from ACCEL_XOUT_H

    uint8_t regAddr = ACCEL_XOUT_H;
    const uint8_t REG_NUM = 14;

    uint8_t tempData[REG_NUM];

    bool result = MpuMRegRead(mpu, regAddr, tempData, REG_NUM);

    if(!result)
        return false;

    rawData->acX = tempData[0] << 8 | tempData[1]; // in reverse as MPU data is big-endian
    rawData->acY = tempData[2] << 8 | tempData[3];
    rawData->acZ = tempData[4] << 8 | tempData[5];
    rawData->tmp = tempData[6] << 8 | tempData[7];
    rawData->gyX = tempData[8] << 8 | tempData[9];
    rawData->gyY = tempData[10] << 8 | tempData[11];
    rawData->gyZ = tempData[12] << 8 | tempData[13];


    return true;
}

bool MpuCalibrateGyros(Mpu6050* mpu)
{
    logger(Info, Log_Mpu, "[MpuCalibrateGyros] Calibrating gyroscope");

    MpuRawData rawData;

    const int MAX_FAILS = 5;
    int failsCnt = 0;

    int xOffsetSum = 0,
        yOffsetSum = 0,
        zOffsetSum = 0;


    for(int i = 0; i != GYRO_CALIB_ITERS; ++i)
    {
        if(!MpuReadRawData(mpu, &rawData))
                ++failsCnt;

        if(failsCnt > MAX_FAILS)
        {
            logger(Error, Log_Mpu, "[MpuCalibrateGyros] Calibration failed");
            return false;
        }

        xOffsetSum += rawData.gyX;
        yOffsetSum += rawData.gyY;
        zOffsetSum += rawData.gyZ;
    }

    g_gyroCalib.x_offset = xOffsetSum / (GYRO_CALIB_ITERS - failsCnt);
    g_gyroCalib.y_offset = yOffsetSum / (GYRO_CALIB_ITERS - failsCnt);
    g_gyroCalib.z_offset = zOffsetSum / (GYRO_CALIB_ITERS - failsCnt);

    logger(Debug, Log_Mpu, "[MpuCalibrateGyros] offsets: %d, %d, %d",
            g_gyroCalib.x_offset, g_gyroCalib.y_offset, g_gyroCalib.z_offset);

    return true;
}

bool MpuUpdateData(Mpu6050* mpu)
{
    MpuRawData rawData;

    if(!MpuReadRawData(mpu, &rawData))
        return false;

    g_accelAngles = calcAccelAngles(&rawData);
    g_gyroRates = calcGyroRates(&rawData);

    return true;
}


AccelAngles MpuGetAccelAngles(Mpu6050* mpu)
{
    return g_accelAngles;
}

GyroRates MpuGetGyroRates(Mpu6050* mpu)
{
    return g_gyroRates;
}

AccelAngles calcAccelAngles(const MpuRawData* rawData)
{
    AccelAngles angles;

    int forceMagnApprox = abs(rawData->acX) + abs(rawData->acY) + abs(rawData->acZ);

    angles.isDataValid = (forceMagnApprox > 8192 && forceMagnApprox < 32768);

    angles.x_axis = 57.295*atan((float)rawData->acY /
            sqrt(pow((float)rawData->acZ,2) + pow((float)rawData->acX,2)));

    angles.y_axis = 57.295*atan((float)-rawData->acX /
            sqrt(pow((float)rawData->acZ,2)+pow((float)rawData->acY,2)));

    return angles;
}

GyroRates calcGyroRates(const MpuRawData* rawData)
{
    GyroRates rates;

    rates.x = (float)(rawData->gyX - g_gyroCalib.x_offset) / g_gyroSensitivity;
    rates.y = (float)(rawData->gyY - g_gyroCalib.y_offset) / g_gyroSensitivity;
    rates.z = (float)(rawData->gyZ - g_gyroCalib.z_offset) / g_gyroSensitivity;

    return rates;
}

bool MpuRegWrite(Mpu6050* mpu, uint8_t reg, uint8_t value)
{
    if(!mpu->initiated)
        return false;

    uint8_t msg[WRITE_MSG_SIZE];
    msg[0] = reg;
    msg[1] = value;

    if(!i2cSend(mpu->i2cManager, mpu->hwAddress, msg, WRITE_MSG_SIZE))
    {
        logger(Error, Log_Mpu, "[MpuRegWrite] Couldn't write val: %#x to reg: %#x", value, reg);
        return false;
    }

    return true;
}

bool MpuRegRead(Mpu6050* mpu, uint8_t reg, uint8_t* value)
{

    bool result = MpuMRegRead(mpu, reg, value, 1);

    return result;
}

bool MpuMRegRead(Mpu6050* mpu, uint8_t startReg, uint8_t* values, uint8_t num)
{
    if(!mpu->initiated)
        return false;

    bool result = i2cSendAndReceive(mpu->i2cManager,
                                    mpu->hwAddress,
                                    &startReg,
                                    1,
                                    values,
                                    num);
    if(!result)
    {
        logger(Error, Log_Mpu, "[MpuMRegRead] Couldn't read reg: %#x", startReg);
        return false;
    }

    return true;
}

bool MpuTest(Mpu6050* mpu)
{
    uint8_t regAddr = WHO_AM_I;
    uint8_t regVal = 0;

    bool result = MpuRegRead(mpu, regAddr, &regVal);

    return result && (regVal == MPU_I2C_ADDRESS);
}
