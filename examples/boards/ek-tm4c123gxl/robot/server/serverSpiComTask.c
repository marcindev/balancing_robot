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
#include "serverSpiCom.h"
#include "logger.h"
#include "led.h"

#include "MCP23017.h"
#include "spiWrapper.h"

#define SERVER_SPI_TASK_STACK_SIZE		200        // Stack size in words
#define SERVER_SPI_QUEUE_SIZE			5
#define SERVER_SPI_ITEM_SIZE			4			// bytes

#define SPI_SEM_WAIT_TIME				10

extern SemaphoreHandle_t g_ssiRxIntSem;

static MsgQueueId g_serverSpiComQueue;
static bool isSpiComInitialized = false;

static uint8_t g_updaterCmdSender;

static void handleStartTask(StartTaskMsgReq* request);
static void handleMessages(void* msg);
static void handleGetLogs(void* msg);
static void handleGetLogsRsp(void* msg);
static void handleGetPostmortem(void* msg);
static void handleGetPostmortemRsp(void* msg);
static void handleGetFreeHeapSize(void* msg);
static void handleGetFreeHeapSizeRsp(void* msg);
static void handleGetTaskList(void* msg);
static void handleGetTaskListRsp(void* msg);
static void handleWheelSetSpeed(void* msg);
static void handleWheelRun(void* msg);
static void handleSetTaskPriority(void* msg);
static void handleUpdaterCmd(void* msg);
static void handleUpdaterCmdRsp(void* msg);
static void handleUpdaterSendData(void* msg);
static void handleUpdaterSendDataRsp(void* msg);
static void handleServerStartedNotif(void* msg);
static void sendConnStatusNotif(void* msg);


extern SpiComInstance* g_spiComInstServer;

static void serverSpiComTask()
{

	 TickType_t xLastWakeTime;
	 const TickType_t xFrequency = 10;
	 xLastWakeTime = xTaskGetTickCount();
	 uint32_t counter = 0;
	while(true)
	{
		void* msg = NULL;

		if(isSpiComInitialized) //&& (xSemaphoreTake(g_ssiRxIntSem, SPI_SEM_WAIT_TIME) == pdTRUE))
		{
#ifdef _ROBOT_MASTER_BOARD
//			size_t heapSize = xPortGetFreeHeapSize();
			if(!(++counter % 100000UL))
				UARTprintf("Ping\n");

#endif
			if(receiveSpiMsg(&msg)){
#ifdef _ROBOT_MASTER_BOARD
//				I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
//				GpioExpander* gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));
//				//
//				ZeroBuffer(gpioExpander, sizeof(GpioExpander));
//				gpioExpander->i2cManager = i2cManager;
//				gpioExpander->hwAddress		= 0x21;
//				GpioExpInit(gpioExpander);
//				GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN3);
//				GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN3);
//				GetLogsMsgReq getLogsReq = INIT_GET_LOGS_MSG_REQ;
//				getLogsReq.slot = 1;
//				msg = &getLogsReq;  // TODO delete ??
#endif

				handleMessages(msg);

			}
		}
#ifndef _ROBOT_MASTER_BOARD
//		vTaskDelayUntil( &xLastWakeTime, 1000 / portTICK_RATE_MS );
		GetLogsMsgReq getLogsReq = INIT_GET_LOGS_MSG_REQ;
		if(!(++counter % 100000UL))
		{	UARTprintf("Ping\n");
//			sendSpiMsg(&getLogsReq);
		}
#endif
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
		handleUpdaterCmd(msg);
		break;
	case UPDATER_CMD_MSG_RSP:
		handleUpdaterCmdRsp(msg);
		break;
	case UPDATER_SEND_DATA_MSG_REQ:
		handleUpdaterSendData(msg);
		break;
	case UPDATER_SEND_DATA_MSG_RSP:
		handleUpdaterSendDataRsp(msg);
		break;

#ifdef _ROBOT_MASTER_BOARD
	case SERVER_STARTED_NOTIF_MSG_REQ:
		handleServerStartedNotif(msg);
		break;
	case CONNECTION_STATUS_MSG_REQ:
		sendConnStatusNotif(msg);
		break;
	case WHEEL_SET_SPEED_TCP_MSG_REQ:
		handleWheelSetSpeed(msg);
		break;
	case WHEEL_RUN_TCP_MSG_REQ:
		handleWheelRun(msg);
		break;
	default:
		logger(Warning, Log_ServerSpiCom, "[handleMessages] Received not recognized message %d", msgId);
		break;
#else
	case GET_LOGS_MSG_RSP:
		handleGetLogsRsp(msg);
		break;
	case GET_POSTMORTEM_MSG_RSP:
		handleGetPostmortemRsp(msg);
		break;
	case GET_FREE_HEAP_SIZE_MSG_RSP:
		handleGetFreeHeapSizeRsp(msg);
		break;
	case GET_TASK_LIST_MSG_RSP:
		handleGetTaskListRsp(msg);
		break;
	default:
		sendSpiMsg(msg);
		vPortFree(msg);
		msg = NULL;
		break;
#endif
	}
}

void handleStartTask(StartTaskMsgReq* request)
{

	initInterrupts();
	initializeSpi();
	bool result = true;

	StartTaskMsgRsp* response = (StartTaskMsgRsp*) pvPortMalloc(sizeof(StartTaskMsgRsp));
	*response = INIT_START_TASK_MSG_RSP;
	response->status = result;
	msgRespond(request->sender, &response, MSG_WAIT_LONG_TIME);
	vPortFree(request);
	request = NULL;

	isSpiComInitialized = true;
	logger(Info, Log_ServerSpiCom, "[handleStartTask] task initiated");

}


void handleGetLogs(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD

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
		response.sender = ((GetLogsMsgReq*)msg)->sender;
		response.slot = ((GetLogsMsgReq*)msg)->slot;
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

		sendSpiMsg(&response);
	}
#else
	logger(Info, Log_ServerSpiCom, "[handleGetLogs] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
#endif

	vPortFree(msg);
	msg = NULL;
}

void handleGetPostmortem(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD

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
	response.sender = ((GetLogsMsgReq*)msg)->sender;
	response.slot = ((GetLogsMsgReq*)msg)->slot;
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
		sendSpiMsg(&response);
	}

	if(lineNum == 1)
		response.ctrlByte = EMPTY;
	else
		response.ctrlByte = LAST;

	sendSpiMsg(&response);
#else
	logger(Info, Log_ServerSpiCom, "[handleGetPostmortem] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
#endif

	vPortFree(msg);
	msg = NULL;
}


void handleGetFreeHeapSize(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	GetFreeHeapSizeRsp response = INIT_GET_FREE_HEAP_SIZE_MSG_RSP;
	response.sender = ((GetFreeHeapSizeReq*)msg)->sender;
	response.slot = ((GetFreeHeapSizeReq*)msg)->slot;
	response.isMaster = 1;
	response.heapSize = xPortGetFreeHeapSize();
	sendSpiMsg(&response);

#else
	logger(Info, Log_ServerSpiCom, "[handleGetFreeHeapSize] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
#endif

	vPortFree(msg);
	msg = NULL;
}

void handleUpdaterCmd(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	g_updaterCmdSender = ((UpdaterCmdMsgReq*) msg)->sender; // remember the sender for later response routing

	msgSend(g_serverSpiComQueue, getQueueIdFromTaskId(Msg_UpdaterTaskID), &msg, MSG_WAIT_LONG_TIME);

#else
//	logger(Info, Log_ServerSpiCom, "[handleUpdaterCmd] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
	vPortFree(msg);
	msg = NULL;
#endif

}


void handleUpdaterCmdRsp(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	((UpdaterCmdMsgRsp*)msg)->sender = g_updaterCmdSender;
	sendSpiMsg(msg);
	vPortFree(msg);
	msg = NULL;
#else
	msgRespond(((UpdaterCmdMsgRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
#endif
}

void handleUpdaterSendData(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	g_updaterCmdSender = ((UpdaterSendDataMsgReq*) msg)->sender; // remember the sender for later response routing

	msgSend(g_serverSpiComQueue, getQueueIdFromTaskId(Msg_UpdaterTaskID), &msg, MSG_WAIT_LONG_TIME);

#else
//	logger(Info, Log_ServerSpiCom, "[handleUpdaterSendData] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
	vPortFree(msg);
	msg = NULL;
#endif

}

void handleUpdaterSendDataRsp(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	((UpdaterSendDataMsgRsp*)msg)->sender = g_updaterCmdSender;
	sendSpiMsg(msg);
	vPortFree(msg);
	msg = NULL;
#else
	msgRespond(((UpdaterSendDataMsgRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
#endif
}

void handleGetTaskList(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	const uint32_t REPORT_BUFFER_SIZE = 1000;
	const uint32_t MSG_BUFFER_SIZE = 200;

	char* reportBuffer = (char*) pvPortMalloc(REPORT_BUFFER_SIZE);

	vTaskList(reportBuffer);

	char* reportPtr = reportBuffer;
	int32_t leftToSend = strlen(reportBuffer);

	GetTaskListRsp response = INIT_GET_TASK_LIST_MSG_RSP;
	response.isMaster = 1;
	response.sender = ((GetTaskListReq*)msg)->sender;
	response.slot = ((GetTaskListReq*)msg)->slot;
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

#else
	logger(Info, Log_ServerSpiCom, "[handleGetTaskList] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
#endif

	vPortFree(msg);
	msg = NULL;
}

void handleSetTaskPriority(void* msg)
{
#ifdef _ROBOT_MASTER_BOARD
	SetTaskPriorityMsgReq* spiMsg = (SetTaskPriorityMsgReq*) msg;
	TaskHandle_t taskHandle = getTaskHandleByNum(spiMsg->taskId);
	if(taskHandle)
	{
		vTaskPrioritySet(taskHandle, tskIDLE_PRIORITY + spiMsg->priority);
		logger(Info, Log_TcpServerHandler, "[handleSetTaskPriority] priority of task %d has been changed to %d"
				, spiMsg->taskId, spiMsg->priority);
	}

#else
	logger(Info, Log_ServerSpiCom, "[handleSetTaskPriority] forwarding msg to master; msgId: %d",*((uint8_t*)msg));
	sendSpiMsg(msg);
#endif

	vPortFree(msg);
	msg = NULL;
}


#ifdef _ROBOT_MASTER_BOARD
void handleServerStartedNotif(void* msg)
{
	LedBlinkStart(LED1, 1);
	vPortFree(msg);
	msg = NULL;
}

void sendConnStatusNotif(void* msg)
{
	bool state = ((ConnectionStatusMsgReq*)msg)->state;

	if(state)
	{
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

	msgSend(g_serverSpiComQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), &request, portMAX_DELAY);

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

	msgSend(g_serverSpiComQueue, getQueueIdFromTaskId(Msg_WheelsTaskID), &request, portMAX_DELAY);

	vPortFree(msg);
	msg = NULL;
}


#else

void handleGetLogsRsp(void* msg)
{
//	UARTprintf("handleGetLogsRsp: response received\n");

	msgRespond(((GetLogsMsgRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
	logger(Debug, Log_ServerSpiCom, "[handleGetLogsRsp] response received");

}

void handleGetPostmortemRsp(void* msg)
{
	msgRespond(((GetPostmortemMsgRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
	logger(Debug, Log_ServerSpiCom, "[handleGetPostmortemRsp] response received");
}

void handleGetFreeHeapSizeRsp(void* msg)
{
	msgRespond(((GetFreeHeapSizeRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
	logger(Debug, Log_ServerSpiCom, "[handleGetFreeHeapSizeRsp] response received");
}

void handleGetTaskListRsp(void* msg)
{
	msgRespond(((GetTaskListRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
	logger(Debug, Log_ServerSpiCom, "[handleGetTaskListRsp] response received");
}

#endif

