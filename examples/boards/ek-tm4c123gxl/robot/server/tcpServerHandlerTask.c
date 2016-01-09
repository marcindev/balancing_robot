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
#include "tcpServerHandlerTask.h"
#include "logger.h"

#define TCP_SERVER_HANDLER_TASK_STACK_SIZE		200        // Stack size in words
#define TCP_SERVER_HANDLER_QUEUE_SIZE			10
#define TCP_SERVER_HANDLER_SUB_QUEUES			5
#define TCP_SERVER_HANDLER_ITEM_SIZE			4			// bytes
#define MAX_SLOTS_NUM							3

#define TCP_BUFFER_SIZE		256

static MsgQueueId g_serverHandlerQueue;

static uint16_t g_sockets[MAX_SLOTS_NUM];
static uint8_t g_slots[MAX_SLOTS_NUM];
static uint8_t g_buffer[TCP_BUFFER_SIZE]; // TODO: semaphore for the buffer

static uint16_t reserveSlot(uint16_t socketId);
static void freeSlot(uint16_t slot);
static bool receiveTcpMsg(uint16_t slot);
static void sendTcpMsg(uint16_t slot, uint8_t msgId, void* msg);
static void handleMessages(uint16_t slot);
static void handleSpiMessages(void* msg);
static void handleGetLogs(uint16_t slot);
static void handleGetLogsRsp(GetLogsMsgRsp* msg);
static void forwardMsgToSpi(uint16_t slot);
static void forwardMsgToTcp(void* msg);


static void tcpServerHandlerTask(void *pvParameters)
{
	uint16_t slot = *((uint16_t*)pvParameters);
	vPortFree(pvParameters);

	while(true)
	{
		//void* msg = NULL;
		if(receiveTcpMsg(slot))
		{
			handleMessages(slot);
		}

		void* msg = NULL;

		if(msgReceive(g_serverHandlerQueue, &msg, 0))
		{
			handleSpiMessages(msg);
		}

	}

}

bool tcpServerHandlerTaskInit(uint16_t socketID)
{

	uint16_t* slot = (uint16_t*) pvPortMalloc(sizeof(uint16_t));
	*slot = reserveSlot(socketID);

	if(slot < 0)
	{
		logger(Error, Log_TcpServerHandler, "[tcpServerHandlerTaskInit] Can't create another task, number of available slots exceeded");
		return false;
	}

	g_serverHandlerQueue = registerMainMsgQueue(MSG_TcpServerHandlerID, TCP_SERVER_HANDLER_QUEUE_SIZE);

	if(g_serverHandlerQueue < 0)
	{
		logger(Error, Log_TcpServerHandler, "[tcpServerHandlerTaskInit] Couldn't register main msg queue");
	}



    if(xTaskCreate(tcpServerHandlerTask, (signed portCHAR *)"TcpServHandler", TCP_SERVER_HANDLER_TASK_STACK_SIZE, (void*) slot,
                   tskIDLE_PRIORITY + PRIORITY_TCP_SERVER_HANDLER_TASK, NULL) != pdTRUE)
    {
    	freeSlot(*slot);
    	vPortFree(slot);
        return false;
    }

    return true;
}


void handleMessages(uint16_t slot)
{

	switch(g_buffer[0])
	{
	case GET_LOGS_MSG_REQ:
		handleGetLogs(slot);
		break;

	default:
		forwardMsgToSpi(slot);
		break;
	}
}

void handleSpiMessages(void* msg)
{

	switch(*((uint8_t*)msg))
	{
	case GET_LOGS_MSG_RSP:
		handleGetLogsRsp((GetLogsMsgRsp*)msg);
		break;

	default:
		forwardMsgToTcp(msg);
		break;
	}
}

void handleGetLogs(uint16_t slot)
{
	GetLogsMsgReq* msg = &g_buffer[0];

	if(msg->isMaster)
	{
		UARTprintf("handleGetLogs(uint16_t slot): slot: %d", slot);
		GetLogsMsgReq* getLogsReq = (GetLogsMsgReq*) pvPortMalloc(sizeof(GetLogsMsgReq));
		*getLogsReq = INIT_GET_LOGS_MSG_REQ;
		getLogsReq->slot = slot;
		msgSend(g_serverHandlerQueue, getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &getLogsReq, MSG_WAIT_LONG_TIME);
		return;
	}


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
		response.isMaster = msg->isMaster;
		response.lineNum = lineNum++;
		response.totalLineNum = totalLinesNum;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		response.component = (uint8_t) logComponent;
		strcpy(&response.strBuffer[0], strPtr);
		response.argsNum = argsNum;
		memcpy(&response.argsBuffer[0], argsBuffer, argsNum * sizeof(uint64_t));

		sendTcpMsg(slot, response.msgId, (void*) &response);
	}
}

void handleGetLogsRsp(GetLogsMsgRsp* msg)
{
	sendTcpMsg(msg->slot, msg->msgId, (void*) msg);

	vPortFree(msg);
	msg = NULL;
}

void forwardMsgToSpi(uint16_t slot)
{
	TcpMsgHeader* tempMsgHdr = &g_buffer[0];
	uint16_t msgLen = getMsgSize(tempMsgHdr->msgId);
	TcpMsgHeader* msgHdr = (TcpMsgHeader*) pvPortMalloc(sizeof(msgLen));
	memcpy(tempMsgHdr, msgHdr, msgLen);
	msgHdr->slot = slot;
	msgSend(g_serverHandlerQueue, getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &msgHdr, MSG_WAIT_LONG_TIME);
}

void forwardMsgToTcp(void* msg)
{
	TcpMsgHeader* msgHdr = (TcpMsgHeader*) msg;
	sendTcpMsg(msgHdr->slot, msgHdr->msgId, msg);

	vPortFree(msg);
}

bool receiveTcpMsg(uint16_t slot)
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
	int16_t leftToRcv = 0;

	uint16_t socketFd = g_sockets[slot];

	fd_set         input;
	FD_ZERO(&input);
	FD_SET(socketFd, &input);
	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 5000;
	status = select(socketFd + 1, &input, NULL, NULL, &timeout);

	if(status <= 0)
	{
		return false;
	}

	if (!FD_ISSET(socketFd, &input))
	   return false;

	// receive 1 byte to check msgId
	status = sl_Recv(socketFd, &(g_buffer[bufferIndex++]), 1, 0);

	if(status == 0)
		return false;

	if(status < 0)
	{
		logger(Error, Log_TcpServerHandler, "[receiveTcpMsg] Couldn't receive TCP message");
		return false;
	}

	msgLen = getMsgSize((uint8_t) g_buffer[0]);

	if(msgLen == 0)
		return false;

	leftToRcv = msgLen - 1;



	while(leftToRcv > 0)
	{
		status = sl_Recv(socketFd, &(g_buffer[bufferIndex]), leftToRcv, 0);

		if(status == 0)
			return true;

		if(status < 0)
		{
			logger(Error, Log_TcpServerHandler, "[receiveTcpMsg] Couldn't receive TCP message");
			return false;
		}

		leftToRcv -= status;
		bufferIndex += status;
	}

	return true;
}

void sendTcpMsg(uint16_t slot, uint8_t msgId, void* msg)
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
	int16_t leftToSend = 0;

	uint8_t* msgPtr = (uint8_t*) msg;

	msgLen = getMsgSize(msgId);
	leftToSend = msgLen;

	while(leftToSend > 0)
	{
		status = sl_Send(g_sockets[slot], &(msgPtr[bufferIndex]), leftToSend, 0);

		if(status == 0)
			return;

		if(status < 0)
		{
			logger(Error, Log_TcpServerHandler, "[receiveTcpMsg] Couldn't send TCP message");
			return;
		}

		leftToSend -= status;
		bufferIndex += status;
	}


}

uint16_t reserveSlot(uint16_t socketId)
{
	for(uint16_t i = 0; i != MAX_SLOTS_NUM; ++i)
	{
		if(!g_slots[i])
		{
			g_slots[i] = 1;
			g_sockets[i] = socketId;
			return i;
		}
	}

	return -1;
}

void freeSlot(uint16_t slot)
{
	g_slots[slot] = 1;
	sl_Close(g_sockets[slot]);
	g_sockets[slot] = 0;

}


