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
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "wdg.h"
#include "motionControlComTask.h"
#include "motionControl.h"
#include "logger.h"

#define MOTION_CONTROL_COM_TASK_STACK_SIZE      100        // Stack size in words
#define MOTION_CONTROL_COM_QUEUE_SIZE            5



static MsgQueueId g_motionCtrlComQueue;

static void initializeMotionControl();
static void handleMessages(void* msg);
static void handleStartTask(StartTaskMsgReq* request);
static void handleSetPidParam(MctrlSetPidParamMsgReq* request);
static void handleSetPeriod(MctrlSetPeriodMsgReq* request);
static void handleSetPidDir(MctrlSetPidDirMsgReq* request);
static void handleGetData(MctrlGetDataMsgReq* request);

static void motionControlComTask()
{
    uint8_t wdgTaskID = registerToWatchDog();

    while(true)
    {
        void* msg;
        feedWatchDog(wdgTaskID, WDG_ASLEEP);

        if(msgReceive(g_motionCtrlComQueue, &msg, MSG_WAIT_LONG_TIME))
        {
            feedWatchDog(wdgTaskID, WDG_ALIVE);
            handleMessages(msg);
        }

    }

}

bool motionControlComTaskInit()
{
    g_motionCtrlComQueue = registerMainMsgQueue(Msg_MotionControlTaskID, MOTION_CONTROL_COM_QUEUE_SIZE);

    if(g_motionCtrlComQueue < 0)
    {
        logger(Error, Log_MotionCtrl, "[motionControlComTaskInit] Couldn't register main msg queue");
    }

    if(xTaskCreate(motionControlComTask, (signed portCHAR *)"MotionControlCom",
                   MOTION_CONTROL_COM_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_MOTION_CONTROL_COM_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void initializeMotionControl()
{
    motionControlTaskInit();

    logger(Info, Log_MotionCtrl, "[initializeMotionControl] motion control initialized");
}


void handleMessages(void* msg)
{
    switch(*((uint8_t*)msg))
    {
    case START_TASK_MSG_REQ:
        handleStartTask((StartTaskMsgReq*) msg);
        break;
    case MCTRL_SET_PID_PARAM_MSG_REQ:
        handleSetPidParam((MctrlSetPidParamMsgReq*) msg);
        break;
    case MCTRL_SET_PERIOD_MSG_REQ:
        handleSetPeriod((MctrlSetPeriodMsgReq*) msg);
        break;
    case MCTRL_SET_PID_DIR_MSG_REQ:
        handleSetPidDir((MctrlSetPidDirMsgReq*) msg);
        break;
    case MCTRL_GET_DATA_MSG_REQ:
        handleGetData((MctrlGetDataMsgReq*) msg);
        break;
    default:
        logger(Warning, Log_MotionCtrl, "[handleMessages] Received not-recognized message");
        break;
    }
}

void handleStartTask(StartTaskMsgReq* request)
{
    initializeMotionControl();

    StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
    *response = INIT_START_TASK_MSG_RSP;
    response->status = true;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleSetPidParam(MctrlSetPidParamMsgReq* request)
{
    logger(Debug, Log_MotionCtrl, "[handleSetPidParam] setting PID param");

    MctrlSetPidParamMsgRsp* response = (MctrlSetPidParamMsgRsp*) pvPortMalloc(sizeof(MctrlSetPidParamMsgRsp));
    *response = INIT_MCTRL_SET_PID_PARAM_MSG_RSP;

    MCtrlSetPidParam(request->param, request->value);

    response->status = true;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleSetPeriod(MctrlSetPeriodMsgReq* request)
{
    logger(Debug, Log_MotionCtrl, "[handleSetPeriod] setting PID period");

    MctrlSetPeriodMsgRsp* response = (MctrlSetPeriodMsgRsp*) pvPortMalloc(sizeof(MctrlSetPeriodMsgRsp));
    *response = INIT_MCTRL_SET_PERIOD_MSG_RSP;

    MCtrlSetPeriod(request->periodMs);

    response->status = true;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleSetPidDir(MctrlSetPidDirMsgReq* request)
{
    logger(Debug, Log_MotionCtrl, "[handleSetPidDir] setting PID direction");

    MctrlSetPidDirMsgRsp* response = (MctrlSetPidDirMsgRsp*) pvPortMalloc(sizeof(MctrlSetPidDirMsgRsp));
    *response = INIT_MCTRL_SET_PID_DIR_MSG_RSP;

    MCtrlSetPidDir(request->direction);

    response->status = true;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}

void handleGetData(MctrlGetDataMsgReq* request)
{

    MctrlGetDataMsgRsp* response = (MctrlGetDataMsgRsp*) pvPortMalloc(sizeof(MctrlGetDataMsgRsp));
    *response = INIT_MCTRL_GET_DATA_MSG_RSP;

    response->data = MCtrlGetData(request->param);

    response->status = true;
    msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
    vPortFree(request);
}






