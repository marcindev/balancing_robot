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
#include "timers.h"
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "global_defs.h"
#include "tcpServerHandlerTask.h"
#include "logger.h"

#define TCP_SERVER_HANDLER_TASK_STACK_SIZE		300        // Stack size in words
#define TCP_SERVER_HANDLER_QUEUE_SIZE			10
#define TCP_SERVER_HANDLER_SUB_QUEUES			5
#define TCP_SERVER_HANDLER_ITEM_SIZE			4			// bytes
#define MAX_SLOTS_NUM							3
#define CONN_TIMER_PERIOD						1000
#define CONNECTION_TIMOUT						5          // seconds

#define TCP_BUFFER_SIZE		256


static TimerHandle_t g_connTimer;
static MsgQueueId g_serverHandlerQueues[MAX_SLOTS_NUM];
static uint16_t g_sockets[MAX_SLOTS_NUM];
static uint8_t g_slots[MAX_SLOTS_NUM];
static uint8_t g_connected[MAX_SLOTS_NUM];
static uint8_t g_counter[MAX_SLOTS_NUM];
static uint8_t g_buffer[TCP_BUFFER_SIZE]; // TODO: semaphore for the buffer

static void initConnectionTimer();
static void connTimerCallback(TimerHandle_t pxTimer);
static void checkConnection(uint16_t slot);
static void connectionLost(uint16_t slot);
static void sendConnStatusNotif(uint16_t slot, uint8_t state);
static uint16_t reserveSlot(uint16_t socketId);
static void freeSlot(uint16_t slot);
static bool receiveTcpMsg(uint16_t slot);
static void sendTcpMsg(uint16_t slot, void* msg);
static void handleMessages(uint16_t slot);
static void handleSpiMessages(void* msg);
static void handleGetLogs(uint16_t slot);
static void handleGetLogsRsp(GetLogsMsgRsp* msg);
static void handleGetFreeHeapSize(uint16_t slot);
static void handleGetTaskList(uint16_t slot);
static void handleSetTaskPriority(uint16_t slot);
static void forwardMsgToSpi(uint16_t slot);
static void forwardMsgToTcp(void* msg);
static void handleHandShake(uint16_t slot);
static void handleGetPostmortem(uint16_t slot);
static void handleUpdaterCmd(uint16_t slot);



static void tcpServerHandlerTask(void *pvParameters)
{
	uint16_t slot = *((uint16_t*)pvParameters);
	vPortFree(pvParameters);

	sendConnStatusNotif(slot, true);
	initConnectionTimer();

	while(true)
	{
		//void* msg = NULL;
		if(receiveTcpMsg(slot))
		{
			handleMessages(slot);
		}

		void* msg = NULL;

		if(msgReceive(g_serverHandlerQueues[slot], &msg, 0))
		{
			handleSpiMessages(msg);
		}

		checkConnection(slot);
	}

}

bool tcpServerHandlerTaskInit(uint16_t socketID)
{

	uint16_t* slot = (uint16_t*) pvPortMalloc(sizeof(uint16_t));
	*slot = reserveSlot(socketID);

	if(*slot < 0)
	{
		logger(Error, Log_TcpServerHandler, "[tcpServerHandlerTaskInit] Can't create another task, number of available slots exceeded");
		return false;
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

void initConnectionTimer()
{
	if(g_connTimer)
		return;

	g_connTimer = xTimerCreate(
			"ConnTimer",
			pdMS_TO_TICKS(CONN_TIMER_PERIOD),
			pdTRUE,
			( void * ) 0,
			connTimerCallback
	);

	if(g_connTimer == NULL)
	{
		logger(Error, Log_TcpServerHandler, "[initConnectionTimer] Couldn't create timer");
		return;
	}

	logger(Info, Log_TcpServerHandler, "[initConnectionTimer] timer created");

	if( xTimerStart( g_connTimer, 0 ) != pdPASS )
	{
		logger(Error, Log_TcpServerHandler, "[initConnectionTimer] Couldn't start timer");
	}

	logger(Debug, Log_TcpServerHandler, "[initConnectionTimer] timer started");
}

void connTimerCallback(TimerHandle_t pxTimer)
{
	for(uint8_t slot = 0; slot != MAX_SLOTS_NUM; slot++)
	{
		if(!g_slots[slot] || !g_connected[slot])
			continue;

//		UARTprintf("++g_counter[slot]: %d", g_counter[slot]);
		if(++g_counter[slot] > CONNECTION_TIMOUT)
			g_connected[slot] = 0;
	}
}

void checkConnection(uint16_t slot)
{
	if(!g_connected[slot])
		connectionLost(slot);
}

void connectionLost(uint16_t slot)
{
	sendConnStatusNotif(slot, false);

	logger(Info, Log_TcpServerHandler, "[connectionLost] freeing the slot and deleting the task");

	freeSlot(slot);
	vTaskDelete(NULL);
}

void sendConnStatusNotif(uint16_t slot, uint8_t state)
{
	logger(Info, Log_TcpServerHandler, "[sendConnStatusNotif] slot: %d, connection state: %d", slot, state);

	ConnectionStatusMsgReq* connStatus = (ConnectionStatusMsgReq*) pvPortMalloc(sizeof(ConnectionStatusMsgReq));
	*connStatus = INIT_CONNECTION_STATUS_MSG_REQ;
	connStatus->slot = slot;
	connStatus->state = state;
	msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &connStatus, MSG_WAIT_LONG_TIME);
}

void handleMessages(uint16_t slot)
{

	switch(g_buffer[0])
	{
	case HANDSHAKE_MSG_REQ:
		handleHandShake(slot);
		break;
	case GET_LOGS_MSG_REQ:
		handleGetLogs(slot);
		break;
	case GET_POSTMORTEM_MSG_REQ:
		handleGetPostmortem(slot);
		break;
	case GET_FREE_HEAP_SIZE_MSG_REQ:
		handleGetFreeHeapSize(slot);
		break;
	case GET_TASK_LIST_MSG_REQ:
		handleGetTaskList(slot);
		break;
	case SET_TASK_PRIORITY_MSG_REQ:
		handleSetTaskPriority(slot);
		break;
	case UPDATER_CMD_MSG_REQ:
		handleUpdaterCmd(slot);
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

void handleHandShake(uint16_t slot)
{
	UARTprintf("handleHandShake: slot: %d", slot);
	g_counter[slot] = 0;

	HandshakeMsgRsp response = INIT_HANDSHAKE_MSG_RSP;
	response.slot = slot;

	sendTcpMsg(slot, (void*) &response);
}

void handleGetLogs(uint16_t slot)
{
	GetLogsMsgReq* msg = &g_buffer[0];

	logger(Info, Log_TcpServerHandler, "[handleGetLogs] slot: %d, isMaster: %d", slot, msg->isMaster);


	if(msg->isMaster)
	{
		UARTprintf("handleGetLogs(uint16_t slot): slot: %d", slot);
		GetLogsMsgReq* getLogsReq = (GetLogsMsgReq*) pvPortMalloc(sizeof(GetLogsMsgReq));
		*getLogsReq = INIT_GET_LOGS_MSG_REQ;
		getLogsReq->slot = slot;
		msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &getLogsReq, MSG_WAIT_LONG_TIME);
		return;
	}


	uint32_t timestamp;
	LogLevel logLevel;
	LogComponent logComponent;
	const char* strPtr;
	uint8_t argsNum;
	uint8_t* argTypes;
	uint8_t* argsBuffer;
	uint8_t argsBuffSize;

	uint16_t totalLinesNum = getLinesNumber();

	GetLogsMsgRsp response = INIT_GET_LOGS_MSG_RSP;

	uint16_t lineNum = 1;

	while(getNextLogLine(&timestamp, &logLevel, &logComponent,
			(void*)&strPtr, &argsNum, (void*)&argTypes, (void*)&argsBuffer, &argsBuffSize))
	{
		response.isMaster = msg->isMaster;
		response.lineNum = lineNum++;
		response.totalLineNum = totalLinesNum;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		response.component = (uint8_t) logComponent;
		strcpy(&response.strBuffer[0], strPtr);
		response.argsNum = argsNum;
		memcpy(&response.argTypes[0], argTypes, argsNum);
		memcpy(&response.argsBuffer[0], argsBuffer, argsBuffSize);
		sendTcpMsg(slot, (void*) &response);

	}
}

void handleGetPostmortem(uint16_t slot)
{
	GetPostmortemMsgReq* msg = &g_buffer[0];

	logger(Info, Log_TcpServerHandler, "[handleGetPostmortem] slot: %d, isMaster: %d", slot, msg->isMaster);


	if(msg->isMaster)
	{
		UARTprintf("handleGetPostmortem(uint16_t slot): slot: %d", slot);
		GetPostmortemMsgReq* request = (GetPostmortemMsgReq*) pvPortMalloc(sizeof(GetPostmortemMsgReq));
		*request = INIT_GET_POSTMORTEM_MSG_REQ;
		request->slot = slot;
		msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &request, MSG_WAIT_LONG_TIME);
		return;
	}

	const uint8_t NORMAL = 0x01,
			      LAST   = 0x02,
				  EMPTY  = 0x04;

	uint32_t timestamp;
	LogLevel logLevel;
	LogComponent logComponent;
	const char* strPtr;
	uint8_t argsNum;
	uint8_t* argTypes;
	uint8_t* argsBuffer;
	uint8_t argsBuffSize;

	GetPostmortemMsgRsp response = INIT_GET_POSTMORTEM_MSG_RSP;
	response.isMaster = msg->isMaster;

	argTypes = response.argTypes;
	argsBuffer = response.argsBuffer;

	uint16_t lineNum = 1;


	while(getNextPMLine(&timestamp, &logLevel, &logComponent,
			(void*)&strPtr, &argsNum, argTypes, argsBuffer, &argsBuffSize))
	{
		response.ctrlByte = NORMAL;
		response.lineNum = lineNum++;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		response.component = (uint8_t) logComponent;
		strcpy(response.strBuffer, strPtr);
		response.argsNum = argsNum;

		sendTcpMsg(slot, (void*) &response);
	}

	if(lineNum == 1)
		response.ctrlByte = EMPTY;
	else
		response.ctrlByte = LAST;

	sendTcpMsg(slot, (void*) &response);
}

void handleGetLogsRsp(GetLogsMsgRsp* msg)
{
	sendTcpMsg(msg->slot, (void*) msg);

	vPortFree(msg);
	msg = NULL;
}

void handleGetFreeHeapSize(uint16_t slot)
{
	GetFreeHeapSizeReq* msg = &g_buffer[0];
	logger(Info, Log_TcpServerHandler, "[handleGetFreeHeapSize] slot: %d, isMaster: %d", slot, msg->isMaster);

	if(msg->isMaster)
	{
		GetFreeHeapSizeReq* request = (GetFreeHeapSizeReq*) pvPortMalloc(sizeof(GetFreeHeapSizeReq));
		*request = INIT_GET_FREE_HEAP_SIZE_MSG_REQ;
		request->slot = slot;
		msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &request, MSG_WAIT_LONG_TIME);
		return;
	}

	GetFreeHeapSizeRsp response = INIT_GET_FREE_HEAP_SIZE_MSG_RSP;
	response.isMaster = msg->isMaster;
	response.heapSize = xPortGetFreeHeapSize();

	sendTcpMsg(slot, (void*) &response);
}

void handleGetTaskList(uint16_t slot)
{
	GetTaskListReq* msg = &g_buffer[0];
	logger(Info, Log_TcpServerHandler, "[handleGetFreeHeapSize] slot: %d, isMaster: %d", slot, msg->isMaster);

	if(msg->isMaster)
	{
		GetTaskListReq* request = (GetTaskListReq*) pvPortMalloc(sizeof(GetTaskListReq));
		*request = INIT_GET_TASK_LIST_MSG_REQ;
		request->slot = slot;
		msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &request, MSG_WAIT_LONG_TIME);
		return;
	}

	const uint32_t REPORT_BUFFER_SIZE = 1000;
	const uint32_t MSG_BUFFER_SIZE = 200;

	char* reportBuffer = (char*) pvPortMalloc(REPORT_BUFFER_SIZE);

	vTaskList(reportBuffer);

	char* reportPtr = reportBuffer;
	int32_t leftToSend = strlen(reportBuffer);

	GetTaskListRsp response = INIT_GET_TASK_LIST_MSG_RSP;
	response.isMaster = msg->isMaster;
	response.totalParts = leftToSend / (MSG_BUFFER_SIZE - 1)
			+ ((leftToSend % (MSG_BUFFER_SIZE - 1)) ? 1 : 0);

	response.partId = 0;

	while(leftToSend > 0)
	{
		strncpy(&response.strBuffer[0], reportPtr, MSG_BUFFER_SIZE - 1);
		response.strBuffer[MSG_BUFFER_SIZE - 1] = '\0';
		reportPtr += MSG_BUFFER_SIZE - 1;
		leftToSend -= MSG_BUFFER_SIZE - 1;

		sendTcpMsg(slot, (void*) &response);

		++response.partId;
	}


	vPortFree(reportBuffer);

}

void handleSetTaskPriority(uint16_t slot)
{
	SetTaskPriorityMsgReq* msg = &g_buffer[0];
	logger(Info, Log_TcpServerHandler, "[handleSetTaskPriority] slot: %d, isMaster: %d", slot, msg->isMaster);

	if(msg->isMaster)
	{
		SetTaskPriorityMsgReq* request = (SetTaskPriorityMsgReq*) pvPortMalloc(sizeof(SetTaskPriorityMsgReq));
		*request = INIT_SET_TASK_PRIORITY_MSG_REQ;
		request->slot = slot;
		msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &request, MSG_WAIT_LONG_TIME);
		return;
	}

	TaskHandle_t taskHandle = getTaskHandleByNum(msg->taskId);
	if(taskHandle)
	{
		vTaskPrioritySet(taskHandle, tskIDLE_PRIORITY + msg->priority);
		logger(Info, Log_TcpServerHandler, "[handleSetTaskPriority] priority of task %d has been changed to %d"
				, msg->taskId, msg->priority);
	}
}

void handleUpdaterCmd(uint16_t slot)
{
	UpdaterCmdMsgReq* msg = &g_buffer[0];
//	logger(Info, Log_TcpServerHandler, "[handleUpdaterCmd] slot: %d, isMaster: %d", slot, msg->isMaster);

	UpdaterCmdMsgReq* request = (UpdaterCmdMsgReq*) pvPortMalloc(sizeof(UpdaterCmdMsgReq));
	*request = *msg;
	request->slot = slot;

	if(request->isMaster)
	{
		msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &request, MSG_WAIT_LONG_TIME);
		return;
	}

	msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_UpdaterTaskID), &request, MSG_WAIT_LONG_TIME);
}

void forwardMsgToSpi(uint16_t slot)
{
	TcpMsgHeader* tempMsgHdr = &g_buffer[0];
	uint16_t msgLen = getMsgSize(tempMsgHdr);
	TcpMsgHeader* msgHdr = (TcpMsgHeader*) pvPortMalloc(sizeof(msgLen));
	memcpy(msgHdr, tempMsgHdr, msgLen);
	msgHdr->slot = slot;
	msgSend(g_serverHandlerQueues[slot], getQueueIdFromTaskId(Msg_ServerSpiComTaskID), &msgHdr, MSG_WAIT_LONG_TIME);
}

void forwardMsgToTcp(void* msg)
{
	TcpMsgHeader* msgHdr = (TcpMsgHeader*) msg;
	sendTcpMsg(msgHdr->slot, msg);

	vPortFree(msg);
}

bool receiveTcpMsg(uint16_t slot)
{
	const int16_t MSG_LEN_OFFSET = 2;

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

	leftToRcv = MSG_LEN_OFFSET;

	while(leftToRcv > 0)
	{
		status = sl_Recv(socketFd, &(g_buffer[bufferIndex]), leftToRcv, 0);

		if(status == 0)
			return false;

		if(status < 0)
		{
			logger(Error, Log_TcpServerHandler, "[receiveTcpMsg] Couldn't receive TCP message");
			return false;
		}

		leftToRcv -= status;
		bufferIndex += status;

		if(bufferIndex == MSG_LEN_OFFSET)
			leftToRcv = getMsgSize(&g_buffer[0]) - MSG_LEN_OFFSET;
	}

	return true;
}

void sendTcpMsg(uint16_t slot, void* msg)
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
	int16_t leftToSend = 0;

	uint8_t* msgPtr = (uint8_t*) msg;

	msgLen = getMsgSize(msg);
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
			g_connected[i] = 1;
			g_counter[i] = 0;
			g_sockets[i] = socketId;

			g_serverHandlerQueues[i] = registerMainMsgQueue(MSG_TcpServerHandlerID, TCP_SERVER_HANDLER_QUEUE_SIZE);

			if(g_serverHandlerQueues[i] < 0)
			{
				g_slots[i] = 0;
				g_connected[i] = 0;
				sl_Close(g_sockets[i]);
				g_sockets[i] = 0;
				logger(Error, Log_TcpServerHandler, "[reserveSlot] Couldn't register main msg queue");
			}
			logger(Debug, Log_TcpServerHandler, "[reserveSlot] Slot reserved successfully");
			return i;
		}
	}

	sl_Close(socketId);
	logger(Error, Log_TcpServerHandler, "[reserveSlot] No more slots available");
	return -1;
}

void freeSlot(uint16_t slot)
{
	g_slots[slot] = 0;
	g_connected[slot] = 0;
	sl_Close(g_sockets[slot]);
	g_sockets[slot] = 0;
	deregisterMainMsgQueue(MSG_TcpServerHandlerID);
	logger(Debug, Log_TcpServerHandler, "[reserveSlot] Slot freed successfully");
}


