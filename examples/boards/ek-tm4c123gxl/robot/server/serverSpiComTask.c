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
#include "wdg.h"
#include "serverSpiComTask.h"
#include "serverSpiCom.h"
#include "logger.h"
#include "led.h"

#include "MCP23017.h"
#include "spiWrapper.h"

#ifdef _ROBOT_MASTER_BOARD
#define SERVER_SPI_TASK_STACK_SIZE		300        // Stack size in words
#else
#define SERVER_SPI_TASK_STACK_SIZE		200        // Stack size in words
#endif
#define SERVER_SPI_QUEUE_SIZE			5
#define SERVER_SPI_ITEM_SIZE			4			// bytes

#define SPI_SEM_WAIT_TIME				10

extern SemaphoreHandle_t g_ssiRxIntSem;

static MsgQueueId g_serverSpiComQueue;
static bool isSpiComInitialized = false;

//static MsgHeader g_updaterCmdSender;

static void handleStartTask(StartTaskMsgReq* request);
static void handleMessages(void* msg);
static void handleSpiMessages(void* msg);
static void handleGetLogs(void* msg);
static void handleGetPostmortem(void* msg);
static void handleGetFreeHeapSize(void* msg);
static void handleGetTaskList(void* msg);
static void handleWheelSetSpeed(void* msg);
static void handleWheelRun(void* msg);
static void handleSetTaskPriority(void* msg);
static void handleUpdaterMsgs(void* msg);
static void handleServerStartedNotif(void* msg);
static void handleMpuMsgs(void* msg);
static void handleConnStatusNotif(void* msg);



extern SpiComInstance* g_spiComInstServer;

static void serverSpiComTask()
{
	uint8_t wdgTaskID = registerToWatchDog();

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = 10;
	xLastWakeTime = xTaskGetTickCount();

	while(true)
	{
		feedWatchDog(wdgTaskID, WDG_ALIVE);

		void* msg = NULL;

		if(isSpiComInitialized) //&& (xSemaphoreTake(g_ssiRxIntSem, SPI_SEM_WAIT_TIME) == pdTRUE))
		{

			if(receiveSpiMsg(&msg)){

				handleSpiMessages(msg);
			}
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
	uint8_t msgId = *((uint8_t*)msg);

	switch(msgId)
	{
	case START_TASK_MSG_REQ:
		handleStartTask((StartTaskMsgReq*) msg);
		break;
	default:
		sendSpiMsg(msg);
		vPortFree(msg);
		msg = NULL;
		break;
	}
}

void handleSpiMessages(void* msg)
{
	uint8_t msgId = *((uint8_t*)msg);

	switch(msgId)
	{
#ifdef _ROBOT_MASTER_BOARD
	case GET_LOGS_MSG_REQ:
		handleGetLogs(msg);
		break;
	case GET_POSTMORTEM_MSG_REQ:
		handleGetPostmortem(msg);
		break;
	case GET_FREE_HEAP_SIZE_MSG_REQ:
		handleGetFreeHeapSize(msg);
		break;
	case GET_TASK_LIST_MSG_REQ:
		handleGetTaskList(msg);
		break;
	case SET_TASK_PRIORITY_MSG_REQ:
		handleSetTaskPriority(msg);
		break;
	case UPDATER_CMD_MSG_REQ:
	case UPDATER_SEND_DATA_MSG_REQ:
		handleUpdaterMsgs(msg);
		break;
	case MPU_REG_READ_MSG_REQ:
	case MPU_REG_WRITE_MSG_REQ:
		handleMpuMsgs(msg);
		break;
	case SERVER_STARTED_NOTIF_MSG_REQ:
		handleServerStartedNotif(msg);
		break;
	case CONNECTION_STATUS_MSG_REQ:
		handleConnStatusNotif(msg);
		break;
	case WHEEL_SET_SPEED_TCP_MSG_REQ:
		handleWheelSetSpeed(msg);
		break;
	case WHEEL_RUN_TCP_MSG_REQ:
		handleWheelRun(msg);
		break;
	default:
		logger(Warning, Log_ServerSpiCom, "[handleSpiMessages] Received not recognized message %d", msgId);
		break;
#else
	default:
		msgRespond(msgGetAddress(msg), &msg, MSG_WAIT_LONG_TIME);
		break;

#endif
	}

}

void handleStartTask(StartTaskMsgReq* request)
{

	initInterrupts();
	initializeSpi();
	bool result = true;

#ifdef _ROBOT_MASTER_BOARD
    LedTurnOff(LED1);
#endif

	StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = result;
	msgRespond(msgGetAddress(request), &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
	request = NULL;

	isSpiComInitialized = true;
	logger(Info, Log_ServerSpiCom, "[handleStartTask] task initiated");

}

#ifdef _ROBOT_MASTER_BOARD

void handleGetLogs(void* msg)
{
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
		msgSetAddress(&response, msgGetAddress(msg));
		response.isMaster = 1;
		response.lineNum = lineNum++;
		response.totalLineNum = totalLinesNum;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		response.component = (uint8_t) logComponent;
		strcpy(&response.strBuffer[0], strPtr);
		response.argsNum = argsNum;
		memcpy(&response.argTypes[0], argTypes, argsNum);
		memcpy(&response.argsBuffer[0], argsBuffer, argsBuffSize);

		portTickType ui32WakeTime;
		ui32WakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2));

		sendSpiMsg(&response);
	}

	vPortFree(msg);
	msg = NULL;
}

void handleGetPostmortem(void* msg)
{
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
	msgSetAddress(&response, msgGetAddress(msg));
	response.isMaster = 1;

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

		portTickType ui32WakeTime;
		ui32WakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2));

		sendSpiMsg(&response);
	}

	if(lineNum == 1)
		response.ctrlByte = EMPTY;
	else
		response.ctrlByte = LAST;

	sendSpiMsg(&response);

	vPortFree(msg);
	msg = NULL;
}


void handleGetFreeHeapSize(void* msg)
{
	GetFreeHeapSizeRsp response = INIT_GET_FREE_HEAP_SIZE_MSG_RSP;
	msgSetAddress(&response, msgGetAddress(msg));
	response.isMaster = 1;
	response.heapSize = xPortGetFreeHeapSize();
	sendSpiMsg(&response);

	vPortFree(msg);
	msg = NULL;
}

void handleUpdaterMsgs(void* msg)
{
	msgSend(g_serverSpiComQueue, getAddressFromTaskId(Msg_UpdaterTaskID), &msg, MSG_WAIT_LONG_TIME);
}

void handleMpuMsgs(void* msg)
{
	msgSend(g_serverSpiComQueue, getAddressFromTaskId(Msg_MpuTaskID), &msg, MSG_WAIT_LONG_TIME);
}

void handleGetTaskList(void* msg)
{
	const uint32_t REPORT_BUFFER_SIZE = 1000;
	const uint32_t MSG_BUFFER_SIZE = 200;

	char* reportBuffer = (char*) pvPortMalloc(REPORT_BUFFER_SIZE);

	vTaskList(reportBuffer);

	char* reportPtr = reportBuffer;
	int32_t leftToSend = strlen(reportBuffer);

	GetTaskListRsp response = INIT_GET_TASK_LIST_MSG_RSP;
	response.isMaster = 1;
	msgSetAddress(&response, msgGetAddress(msg));
	response.totalParts = leftToSend / (MSG_BUFFER_SIZE - 1)
			+ ((leftToSend % (MSG_BUFFER_SIZE - 1)) ? 1 : 0);

	response.partId = 0;

	while(leftToSend > 0)
	{
		strncpy(&response.strBuffer[0], reportPtr, MSG_BUFFER_SIZE - 1);
		response.strBuffer[MSG_BUFFER_SIZE - 1] = '\0';
		reportPtr += MSG_BUFFER_SIZE - 1;
		leftToSend -= MSG_BUFFER_SIZE - 1;

		sendSpiMsg(&response);

		++response.partId;
	}

	vPortFree(reportBuffer);

	vPortFree(msg);
	msg = NULL;
}

void handleSetTaskPriority(void* msg)
{
	SetTaskPriorityMsgReq* spiMsg = (SetTaskPriorityMsgReq*) msg;
	TaskHandle_t taskHandle = getTaskHandleByNum(spiMsg->taskId);
	if(taskHandle)
	{
		vTaskPrioritySet(taskHandle, tskIDLE_PRIORITY + spiMsg->priority);
		logger(Info, Log_TcpServerHandler, "[handleSetTaskPriority] priority of task %d has been changed to %d"
				, spiMsg->taskId, spiMsg->priority);
	}

	vPortFree(msg);
	msg = NULL;
}


void handleServerStartedNotif(void* msg)
{
	LedBlinkStart(LED1, 1);
	vPortFree(msg);
	msg = NULL;
}

void handleConnStatusNotif(void* msg)
{
	bool state = ((ConnectionStatusMsgReq*)msg)->state;

	if(state)
	{
		if(!updateRoutingTable(msg))
			logger(Error, Log_ServerSpiCom, "[handleConnStatusNotif] couldn't update routing table");

		LedBlinkStop(LED1);
		LedTurnOn(LED1);
	}
	else
	{
		LedBlinkStart(LED1, 1);
	}

	vPortFree(msg);
	msg = NULL;
}

void handleWheelSetSpeed(void* msg)
{
	WheelSetSpeedTcpMsgReq* spiMsg = (WheelSetSpeedTcpMsgReq*) msg;

	WheelSetSpeedMsgReq* request = (WheelSetSpeedMsgReq*) pvPortMalloc(sizeof(WheelSetSpeedMsgReq));
	*request = INIT_WHEEL_SET_SPEED_MSG_REQ;
	request->wheelId = spiMsg->wheelId;
	request->speed = spiMsg->speed;

	msgSend(g_serverSpiComQueue, getAddressFromTaskId(Msg_WheelsTaskID), &request, portMAX_DELAY);

	vPortFree(msg);
	msg = NULL;
}

void handleWheelRun(void* msg)
{
	WheelRunTcpMsgReq* spiMsg = (WheelRunTcpMsgReq*) msg;

	WheelRunMsgReq* request = (WheelRunMsgReq*) pvPortMalloc(sizeof(WheelRunMsgReq));
	*request = INIT_WHEEL_RUN_MSG_REQ;
	request->wheelId = spiMsg->wheelId;
	request->direction = spiMsg->direction;
	request->rotations = spiMsg->rotations;

	msgSend(g_serverSpiComQueue, getAddressFromTaskId(Msg_WheelsTaskID), &request, portMAX_DELAY);

	vPortFree(msg);
	msg = NULL;
}

#endif

