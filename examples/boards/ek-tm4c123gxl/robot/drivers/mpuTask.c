//*****************************************************************************
//
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "wdg.h"
#include "MPU6050.h"
#include "mpuTask.h"
#include "logger.h"

#define MPU_TASK_STACK_SIZE     200        // Stack size in words
#define MPU_QUEUE_SIZE           5



static MsgQueueId g_MpuQueue;
static Mpu6050* g_mpu;

static void initializeMpu();
static void handleMessages(void* msg);
static void handleStartTask(StartTaskMsgReq* request);
static void handleMpuRegRead(MpuRegReadMsgReq* request);
static void handleMpuRegWrite(MpuRegWriteMsgReq* request);
static void handleMpuGetData(MpuGetDataMsgReq* request);
static void handleMpuGetDataTcp(MpuGetDataTcpMsgReq* request);


static void mpuTask()
{
    uint8_t wdgTaskID = registerToWatchDog();

    while(true)
    {
        void* msg;
        feedWatchDog(wdgTaskID, WDG_ASLEEP);

        if(msgReceive(g_MpuQueue, &msg, MSG_WAIT_LONG_TIME))
        {
            feedWatchDog(wdgTaskID, WDG_ALIVE);
            handleMessages(msg);
        }

    }

}

bool mpuTaskInit()
{
    g_MpuQueue = registerMainMsgQueue(Msg_MpuTaskID, MPU_QUEUE_SIZE);

    if(g_MpuQueue < 0)
    {
        logger(Error, Log_Mpu, "[mpuTaskInit] Couldn't register main msg queue");
    }

    if(xTaskCreate(mpuTask, (signed portCHAR *)"Mpu", MPU_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_MPU_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeMpu()
{

    I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
    g_mpu = (Mpu6050*) pvPortMalloc(sizeof(Mpu6050));

    if(!i2cManager || !g_mpu)
    {
        logger(Error, Log_Mpu, "[initializeMpu] Out of memory");
        return;
    }

    ZeroBuffer(g_mpu, sizeof(Mpu6050));
    g_mpu->i2cManager = i2cManager;

    MpuInit(g_mpu);

    logger(Info, Log_Mpu, "[initializeMpu] Mpu initialized");
}


void handleMessages(void* msg)
{
    switch(*((uint8_t*)msg))
    {
    case START_TASK_MSG_REQ:
        handleStartTask((StartTaskMsgReq*) msg);
        break;
    case MPU_REG_READ_MSG_REQ:
        handleMpuRegRead((MpuRegReadMsgReq*) msg);
        break;
    case MPU_REG_WRITE_MSG_REQ:
        handleMpuRegWrite((MpuRegWriteMsgReq*) msg);
        break;
    case MPU_GET_DATA_MSG_REQ:
        handleMpuGetData((MpuGetDataMsgReq*) msg);
        break;
    case MPU_GET_DATA_TCP_MSG_REQ:
        handleMpuGetDataTcp((MpuGetDataTcpMsgReq*) msg);
        break;
    default:
        logger(Warning, Log_Mpu, "[handleMessages] Received not-recognized message");
        break;
    }
}

void handleStartTask(StartTaskMsgReq* request)
{
    initializeMpu();

    StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
    *response = INIT_START_TASK_MSG_RSP;
    response->status = true;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleMpuGetData(MpuGetDataMsgReq* request)
{
    MpuGetDataMsgRsp* response = (MpuGetDataMsgRsp*) pvPortMalloc(sizeof(MpuGetDataMsgRsp));
    *response = INIT_MPU_GET_DATA_MSG_RSP;

    bool status = MpuUpdateData(g_mpu);

    AccelAngles accAngles = MpuGetAccelAngles(g_mpu);
    GyroRates gyrRates = MpuGetGyroRates(g_mpu);

    response->accelX = accAngles.x_axis;
    response->accelY = accAngles.y_axis;
    response->isAccValid = accAngles.isDataValid;
    response->gyroX = gyrRates.x;
    response->gyroY = gyrRates.y;
    response->gyroZ = gyrRates.z;


    response->status = status;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleMpuGetDataTcp(MpuGetDataTcpMsgReq* request)
{
    MpuGetDataTcpMsgRsp* response = (MpuGetDataTcpMsgRsp*) pvPortMalloc(sizeof(MpuGetDataTcpMsgRsp));
    *response = INIT_MPU_GET_DATA_TCP_MSG_RSP;

    bool status = MpuUpdateData(g_mpu);

    AccelAngles accAngles = MpuGetAccelAngles(g_mpu);
    GyroRates gyrRates = MpuGetGyroRates(g_mpu);

    response->accelX = accAngles.x_axis;
    response->accelY = accAngles.y_axis;
    response->gyroX = gyrRates.x;
    response->gyroY = gyrRates.y;
    response->gyroZ = gyrRates.z;

    response->status = status;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleMpuRegRead(MpuRegReadMsgReq* request)
{
    logger(Debug, Log_Mpu, "[handleMpuRegRead] read MPU reg");

    MpuRegReadMsgRsp* response = (MpuRegReadMsgRsp*) pvPortMalloc(sizeof(MpuRegReadMsgRsp));
    *response = INIT_MPU_REG_READ_MSG_RSP;

    bool status = MpuRegRead(g_mpu, request->regAddr, &response->regVal);

    response->status = status;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleMpuRegWrite(MpuRegWriteMsgReq* request)
{
    logger(Debug, Log_Mpu, "[handleMpuRegWrite] write MPU reg");

    MpuRegWriteMsgRsp* response = (MpuRegWriteMsgRsp*) pvPortMalloc(sizeof(MpuRegWriteMsgRsp));
    *response = INIT_MPU_REG_WRITE_MSG_RSP;

    bool status = MpuRegWrite(g_mpu, request->regAddr, request->regVal);

    response->status = status;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}




