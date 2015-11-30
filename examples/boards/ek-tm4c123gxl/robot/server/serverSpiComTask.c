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
#include "serverSpiComTask.h"
#include "logger.h"



#define SERVER_SPI_TASK_STACK_SIZE		50        // Stack size in words
#define SERVER_SPI_QUEUE_SIZE			10
#define SERVER_SPI_ITEM_SIZE			4			// bytes

#define SPI_SEM_WAIT_TIME				10

extern SemaphoreHandle_t g_ssiRxIntSem;

static MsgQueueId g_serverSpiComQueue;

static void handleMessages(void* msg);
static void handleGetLogs(void* msg);


static void serverSpiComTask()
{
	initInterrupts();
	initializeSpi();

	while(true)
	{
		void* msg = NULL;

		if(xSemaphoreTake(g_ssiRxIntSem, SPI_SEM_WAIT_TIME) == pdTRUE)
		{
			if(receiveSpiMsg(&msg))
				handleMessages(msg);
		}

		if(msgReceive(g_serverSpiComQueue, &msg, 0))
		{
			handleMessages(msg);
		}

	}

}

bool serverSpiComTaskInit()
{
	g_serverSpiComQueue = registerMainMsgQueue(Msg_ServerSpiComTaskID, SERVER_SPI_QUEUE_SIZE);

	if(g_serverSpiComQueue < 0)
	{
		logger(Error, Log_ServerSpiCom, "[serverSpiComTaskInit] Couldn't register main msg queue");
	}


    if(xTaskCreate(serverSpiComTask, (signed portCHAR *)"ServerSpiCom", SERVER_SPI_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_SERVER_SPI_TASK, NULL) != pdTRUE)
    {
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

	case GET_LOGS_MSG_REQ:
		handleGetLogs(msg);
		break;

	default:
		logger(Warning, Log_ServerSpiCom, "[handleMessages] Received not recognized message");
		break;
	}
}

void handleStartTask(StartTaskMsgReq* request)
{
	bool result = serverSpiComTaskInit();

	StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = result;
	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
}

void handleGetLogs(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	uint32_t timestamp;
	LogLevel logLevel;
	LogComponent logComponent;
	const char* strPtr;
	uint8_t argsNum;
	uint8_t* argsBuffer;

	uint16_t totalLinesNum = getLinesNumber();

	GetLogsMsgRsp response = INIT_GET_LOGS_MSG_RSP;

	uint16_t lineNum = 1;

	while(getNextLogLine(&timestamp, &logLevel, &logComponent,
			(void*)&strPtr, &argsNum, (void*)&argsBuffer))
	{
		response.lineNum = lineNum++;
		response.totalLineNum = totalLinesNum;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		response.component = (uint8_t) logComponent;
		strcpy(&response.strBuffer[0], strPtr);
		response.argsNum = argsNum;
		memcpy(&response.argsBuffer[0], argsBuffer, argsNum * sizeof(uint64_t));

		sendSpiMsg(&response);
	}
#else
	sendSpiMsg(msg);
#endif
}


