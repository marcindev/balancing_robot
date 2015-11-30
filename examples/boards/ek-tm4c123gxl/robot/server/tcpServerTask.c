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
#include "tcpServerTask.h"
#include "tcpServer.h"
#include "logger.h"

#define TCP_SERVER_TASK_STACK_SIZE		250        // Stack size in words
#define TCP_SERVER_QUEUE_SIZE			5

static MsgQueueId g_tcpServerMainQueue;

static void handleStartTask(StartTaskMsgReq* request);

static void tcpServerTask()
{
	while(true)
	{
		void* msg;
		if(msgReceive(g_tcpServerMainQueue, &msg, 0))
		{
			handleMessages(msg);
		}

		runTcpServer();
	}

}

bool tcpServerTaskInit()
{
	g_tcpServerMainQueue = registerMainMsgQueue(Msg_TcpServerTaskID, TCP_SERVER_QUEUE_SIZE);

	if(g_tcpServerMainQueue < 0)
	{
		logger(Error, Log_Wheels, "[tcpServerTaskInit] Couldn't register main msg queue");
		return false;
	}

    if(!startSimpleLinkTask(PRIORITY_SIMPLE_LINK_TASK))
    {
    	while(1){}
    }

    if(xTaskCreate(tcpServerTask, (signed portCHAR *)"TcpServer", TCP_SERVER_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_TCP_SERVER_TASK, NULL) != pdTRUE)
    {
    	logger(Error, Log_TcpServer, "[tcpServerTaskInit] Couldn't create server task");
        return false;
    }

    return true;
}

void handleMessages(void* msg)
{
	switch(*((uint8_t*)msg))
	{
	case START_TASK_MSG_REQ:
		handleStartTask((StartTaskMsgReq*) msg);
		break;
	default:
		logger(Warning, Log_Wheels, "[handleMessages] Received not recognized message");
		break;
	}
}


void handleStartTask(StartTaskMsgReq* request)
{
	bool result = initTcpServer();

	StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = result;
	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
}


