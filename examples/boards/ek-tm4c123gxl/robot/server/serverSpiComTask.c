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

#include "MCP23017.h"
#include "spiWrapper.h"

#define SERVER_SPI_TASK_STACK_SIZE		200        // Stack size in words
#define SERVER_SPI_QUEUE_SIZE			5
#define SERVER_SPI_ITEM_SIZE			4			// bytes

#define SPI_SEM_WAIT_TIME				10

extern SemaphoreHandle_t g_ssiRxIntSem;

static MsgQueueId g_serverSpiComQueue;
static bool isSpiComInitialized = false;

static void handleMessages(void* msg);
static void handleGetLogs(void* msg);
static void handleGetLogsRsp(void* msg);


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
	switch(*((uint8_t*)msg))
	{
	case START_TASK_MSG_REQ:
		handleStartTask((StartTaskMsgReq*) msg);
		break;

	case GET_LOGS_MSG_REQ:
		handleGetLogs(msg);
		break;
#ifdef _ROBOT_MASTER_BOARD


	default:
		logger(Warning, Log_ServerSpiCom, "[handleMessages] Received not recognized message");
		break;
#else
	case GET_LOGS_MSG_RSP:
		handleGetLogsRsp(msg);
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
//
//	I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
//	GpioExpander* gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));
//	//
//	ZeroBuffer(gpioExpander, sizeof(GpioExpander));
//	gpioExpander->i2cManager = i2cManager;
//	gpioExpander->hwAddress		= 0x21;
//	GpioExpInit(gpioExpander);
//	GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN4);
//	GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN4);



	uint32_t timestamp;
	LogLevel logLevel;
	LogComponent logComponent;
	const char* strPtr;
	uint8_t argsNum;
	uint8_t* argTypes;
	uint8_t* argsBuffer;
	uint8_t* argsBuffSize;

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
	logger(Info, Log_ServerSpiCom, "[handleGetLogs] forwarding msg to master; msgId: %d,%f,%d,%f,%f",2, 3.0, 4, 5.0, 6.0);// *((uint8_t*)msg));
	sendSpiMsg(msg);
#endif

	vPortFree(msg);
	msg = NULL;
}

#ifdef _ROBOT_MASTER_BOARD

#else

void handleGetLogsRsp(void* msg)
{
	UARTprintf("handleGetLogsRsp: response received\n");

	msgRespond(((GetLogsMsgRsp*)msg)->sender, &msg, MSG_WAIT_LONG_TIME);
	logger(Info, Log_ServerSpiCom, "[handleGetLogsRsp] response received");

}

#endif

