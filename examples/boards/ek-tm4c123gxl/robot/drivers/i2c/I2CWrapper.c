/*  Wrapper for I2C driver
 *  autor: Marcin Gozdziewski
 */
#include "I2CWrapper.h"


bool I2CComIsOk(I2CComInstance* i2cComInst);

void I2CComInit(I2CComInstance* i2cComInst)
{
    if(i2cComInst->initiated)
        return;

    SysCtlPeripheralEnable(i2cComInst->i2cPeripheral);
    SysCtlPeripheralEnable(i2cComInst->gpioPeripheral);
    GPIOPinConfigure(i2cComInst->sclPinConfig);
    GPIOPinConfigure(i2cComInst->sdaPinConfig);
    GPIOPinTypeI2CSCL(i2cComInst->gpioPortBase, i2cComInst->sclPin);
    GPIOPinTypeI2C(i2cComInst->gpioPortBase, i2cComInst->sdaPin);

    i2cComInst->isMaster = true; // until there's no slave implementation

    if(i2cComInst->isMaster)
        I2CMasterInitExpClk(i2cComInst->i2cBase, SysCtlClockGet(), i2cComInst->speed);
    else
    {
        I2CSlaveEnable(i2cComInst->i2cBase);
        I2CSlaveInit(i2cComInst->i2cBase, i2cComInst->slaveAddress);
    }

    i2cComInst->initiated = true;
}

bool I2CComInitiated(I2CComInstance* i2cComInst)
{
    return i2cComInst->initiated;
}

bool I2CComSend(I2CComInstance* i2cComInst, uint8_t slaveAddress, const uint8_t* data, uint32_t length)
{
    if(!length)
        return false;

    uint32_t dataIndex = 0;
// TODO: interrupt-semaphore solution instead of polling
    if(i2cComInst->isMaster)
    {

        I2CMasterSlaveAddrSet(i2cComInst->i2cBase, slaveAddress, false);

        if(length == 1)
        {
            I2CMasterDataPut(i2cComInst->i2cBase, data[dataIndex]);
            I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_SINGLE_SEND);
            while(I2CMasterBusy(i2cComInst->i2cBase));

            return I2CComIsOk(i2cComInst);
        }

        I2CMasterDataPut(i2cComInst->i2cBase, data[dataIndex++]);
        I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(i2cComInst->i2cBase));

        while(dataIndex < length - 1)
        {
            I2CMasterDataPut(i2cComInst->i2cBase, data[dataIndex++]);
            I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_BURST_SEND_CONT);
            while(I2CMasterBusy(i2cComInst->i2cBase));
        }

        I2CMasterDataPut(i2cComInst->i2cBase, data[dataIndex++]);
        I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_BURST_SEND_FINISH);
        while(I2CMasterBusy(i2cComInst->i2cBase));

    }
    else
    {
        I2CSlaveDataPut(i2cComInst->i2cBase, data[dataIndex]);
    }

    return I2CComIsOk(i2cComInst);
}

bool I2CComReceive(I2CComInstance* i2cComInst, uint8_t slaveAddress, uint8_t* data, uint32_t length)
{
    if(!length)
        return false;

    uint32_t dataIndex = 0;

    I2CMasterSlaveAddrSet(i2cComInst->i2cBase, slaveAddress, true);

    if(length == 1)
    {
        I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_SINGLE_RECEIVE);
        while(I2CMasterBusy(i2cComInst->i2cBase));
        data[dataIndex] = I2CMasterDataGet(i2cComInst->i2cBase);

        return I2CComIsOk(i2cComInst);
    }

    I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(i2cComInst->i2cBase));
    data[dataIndex++] = I2CMasterDataGet(i2cComInst->i2cBase);

    while(dataIndex < length - 1)
    {
        I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        while(I2CMasterBusy(i2cComInst->i2cBase));
        data[dataIndex++] = I2CMasterDataGet(i2cComInst->i2cBase);
    }

    I2CMasterControl(i2cComInst->i2cBase, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(i2cComInst->i2cBase));
    data[dataIndex++] = I2CMasterDataGet(i2cComInst->i2cBase);

    return I2CComIsOk(i2cComInst);
}

bool I2CComIsOk(I2CComInstance* i2cComInst)
{
    if(!i2cComInst->isMaster)
        return true;

    i2cComInst->errState = I2CMasterErr(i2cComInst->i2cBase);

    if(i2cComInst->errState == I2C_MASTER_ERR_NONE)
        return true;

    return false;
}
