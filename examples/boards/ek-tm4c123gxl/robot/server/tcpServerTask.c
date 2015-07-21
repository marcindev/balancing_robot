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
#include "tcpServerTask.h"
#include "tcpServer.h"
#include "logger.h"

#define TCP_SERVER_TASK_STACK_SIZE		700        // Stack size in words
#define TCP_SERVER_QUEUE_SIZE			50
#define TCP_SERVER_ITEM_SIZE			4			// bytes



static void tcpServerTask()
{
	initTcpServer();

	while(true)
	{
		runTcpServer();
	}

}

bool tcpServerTaskInit()
{
	//g_motorsQueue = xQueueCreate(MOTORS_QUEUE_SIZE, MOTORS_ITEM_SIZE);


    if(xTaskCreate(tcpServerTask, (signed portCHAR *)"TcpServer", TCP_SERVER_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_TCP_SERVER_TASK, NULL) != pdTRUE)
    {
    	logger(Error, Log_TcpServer, "[tcpServerTaskInit] Couldn't create server task");
        return false;
    }

    return true;
}



