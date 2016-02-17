//*****************************************************************************
//
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "global_defs.h"
#include "logger.h"
#include "updater.h"

#define UPDATER_TASK_STACK_SIZE			200        // Stack size in words
#define UPDATER_QUEUE_SIZE				5
#define UPDATER_MSG_WAIT_TIME	 		0xFFFF

static MsgQueueId g_updaterMainQueue;

static void handleStartTask(StartTaskMsgReq* request);
static void handleUpdaterCmd(UpdaterCmdMsgReq* request);
static void handleMessages(void* msg);

static void updaterTask()
{
	while(true)
	{
		void* msg;
		if(msgReceive(g_updaterMainQueue, &msg, UPDATER_MSG_WAIT_TIME))
		{
			handleMessages(msg);
		}

	}

}

bool updaterTaskInit()
{
	g_updaterMainQueue = registerMainMsgQueue(Msg_UpdaterTaskID, UPDATER_QUEUE_SIZE);

	if(g_updaterMainQueue < 0)
	{
		logger(Error, Log_Updater, "[updaterTaskInit] Couldn't register main msg queue");
		return false;
	}

    if(xTaskCreate(updaterTask, (signed portCHAR *)"Updater", UPDATER_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_UPDATER_TASK, NULL) != pdTRUE)
    {
    	logger(Error, Log_Updater, "[updaterTaskInit] Couldn't create updater task");
        return false;
    }

    return true;
}

void handleMessages(void* msg)
{
	uint8_t msgId = *((uint8_t*)msg);

	switch(msgId)
	{
	case START_TASK_MSG_REQ:
		handleStartTask((StartTaskMsgReq*) msg);
		break;
	case UPDATER_CMD_MSG_REQ:
		handleUpdaterCmd((UpdaterCmdMsgReq*) msg);
		break;
	default:
		logger(Warning, Log_Updater, "[handleMessages] Received not recognized message %d", msgId);
		break;
	}
}


void handleStartTask(StartTaskMsgReq* request)
{
	initUpdater();

	StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = true;
	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
}

void handleUpdaterCmd(UpdaterCmdMsgReq* request)
{
	uint8_t status = handleUpdateCommand(request->command,
										 request->data1,
										 request->data2,
										 request->data3);

	UpdaterCmdMsgRsp* response = (UpdaterCmdMsgRsp*) pvPortMalloc(sizeof(UpdaterCmdMsgRsp));
	*response = INIT_UPDATER_CMD_MSG_RSP;
	response->slot = request->slot;
	response->status = status;
	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
}

