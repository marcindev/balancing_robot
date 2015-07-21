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
#include "utils.h"
#include "global_defs.h"
#include "tcpServerHandlerTask.h"
#include "logger.h"

#define TCP_SERVER_HANDLER_TASK_STACK_SIZE		400        // Stack size in words
#define TCP_SERVER_HANDLER_QUEUE_SIZE			50
#define TCP_SERVER_HANDLER_ITEM_SIZE			4			// bytes
#define MAX_SLOTS_NUM							3

#define TCP_BUFFER_SIZE		500

static uint16_t g_sockets[MAX_SLOTS_NUM];
static uint8_t g_slots[MAX_SLOTS_NUM];
static uint8_t g_buffer[TCP_BUFFER_SIZE]; // TODO: semaphore for the buffer

static uint16_t reserveSlot(uint16_t socketId);
static void freeSlot(uint16_t slot);
static bool receiveTcpMsg(uint16_t slot);
static void sendTcpMsg(uint16_t slot, uint8_t msgId, void* msg);
static void handleMessages(uint16_t slot);
static void handleGetLogs(uint16_t slot);


static void tcpServerHandlerTask(void *pvParameters)
{
	uint16_t slot = *((uint16_t*)pvParameters);

	while(true)
	{
		//void* msg = NULL;
		if(receiveTcpMsg(slot))
		{
			handleMessages(slot);
		}

	}

}

bool tcpServerHandlerTaskInit(uint16_t socketID)
{
	//g_motorsQueue = xQueueCreate(MOTORS_QUEUE_SIZE, MOTORS_ITEM_SIZE);

	uint16_t slot = reserveSlot(socketID);

	if(slot < 0)
	{
		logger(Error, Log_TcpServerHandler, "[tcpServerHandlerTaskInit] Can't create another task, number of available slots exceeded");
		return false;
	}

    if(xTaskCreate(tcpServerHandlerTask, (signed portCHAR *)"TcpServHandler", TCP_SERVER_HANDLER_TASK_STACK_SIZE, (void*) &slot,
                   tskIDLE_PRIORITY + PRIORITY_TCP_SERVER_HANDLER_TASK, NULL) != pdTRUE)
    {
    	freeSlot(slot);
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
		// Received not-recognized message
		break;
	}
}

void handleGetLogs(uint16_t slot)
{
	uint32_t timestamp;
	LogLevel logLevel;
	const char* strPtr;

	uint16_t totalLinesNum = getLinesNumber();

	GetLogsMsgRsp response = INIT_GET_LOGS_MSG_RSP;

	uint16_t lineNum = 1;

	while(getNextLogLine(&timestamp, &logLevel, (void*)&strPtr))
	{
		response.lineNum = lineNum++;
		response.totalLineNum = totalLinesNum;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		strcpy(&response.buffer[0], strPtr);

		sendTcpMsg(slot, response.msgId, (void*) &response);
	}
}

bool receiveTcpMsg(uint16_t slot)
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
	int16_t leftToRcv = 0;

	// receive 1 byte to check msgId
	status = sl_Recv(g_sockets[slot], &(g_buffer[bufferIndex++]), 1, 0);

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
		status = sl_Recv(g_sockets[slot], &(g_buffer[bufferIndex]), leftToRcv, 0);

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


