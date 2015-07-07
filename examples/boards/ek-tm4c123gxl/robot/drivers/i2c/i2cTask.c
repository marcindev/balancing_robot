//*****************************************************************************
//
//
//*****************************************************************************
#include <stdbool.h>
#include <stdint.h>
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "I2CWrapper.h"
#include "messages.h"

#define I2C_TASK_STACK_SIZE		128         // Stack size in words
#define I2C_QUEUE_SIZE			100
#define I2C_ITEM_SIZE			  4			// bytes

extern xQueueHandle g_i2cRxQueues[];
xQueueHandle g_i2cTxQueue;
static I2CComInstance i2cInstance;

// TODO consider using dynamically allocated messages instead of automatic in case those didn't work
// ... and other variables
static void i2cTask(void *pvParameters)
{
	I2CComInit(&i2cInstance);

	while(true)
	{
		void* msg;
        if(xQueueReceive(g_i2cTxQueue, msg, 0) == pdPASS)
        {
        	switch(*((uint8_t*)msg))
        	{
        	case I2C_SEND_MSG_REQ:
        		i2cHandleSend((I2cSendMsgReq*) msg);
        		break;
        	case I2C_RECEIVE_MSG_REQ:
        		i2cHandleReceive((I2cReceiveMsgReq*) msg);
        		break;

        	case I2C_SEND_N_RECEIVE_MSG_REQ:
        		i2cHandleSendAndReceive((I2cSendAndReceiveMsgReq*) msg);
        		break;

        	default:
        		// Received not-recognized message
        		break;
        	}

        }
	}
}

bool i2cTaskInit()
{
	g_i2cTxQueue = xQueueCreate(I2C_QUEUE_SIZE, I2C_ITEM_SIZE);

    if(xTaskCreate(i2cTask, (signed portCHAR *)"I2C", I2C_TASK_STACK_SIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_I2C_TASK, NULL) != pdTRUE)
    {
        return false;
    }

    return true;
}

void i2cHandleSend(I2cSendMsgReq* request)
{
	bool result;
	result = I2CComSend(&i2cInstance, request->slaveAddress, request->data, request->length);

	I2cSendMsgRsp response = INIT_SEND_I2C_MSG_RSP;
	response.status = result;

	xQueueSend(g_i2cRxQueues[request->sender], &response, portMAX_DELAY);
}

void i2cHandleReceive(I2cReceiveMsgReq* request)
{
	bool result;
	uint8_t data;

	result = I2CComReceive(&i2cInstance, request->slaveAddress, &data, request->length);

	I2cReceiveMsgRsp response = INIT_I2C_RECEIVE_MSG_RSP;
	response.status = result;
	response.data = data;
	response.length = request->length;

	xQueueSend(g_i2cRxQueues[request->sender], &response, portMAX_DELAY);
}

void i2cHandleSendAndReceive(I2cSendAndReceiveMsgReq* request)
{
	bool result;
	uint8_t data;
	I2cSendAndReceiveMsgRsp response = INIT_I2C_SEND_N_RECEIVE_MSG_RSP;

	result = I2CComSend(&i2cInstance, request->slaveAddress, request->data, request->sentLength);

	if(!result)
	{
		response.status = 0;
		xQueueSend(g_i2cRxQueues[request->sender], &response, portMAX_DELAY);

		return;
	}

	result = I2CComReceive(&i2cInstance, request->slaveAddress, &data, request->rcvLength);

	response.status = result;
	response.data = data;
	response.length = request->rcvLength;

	xQueueSend(g_i2cRxQueues[request->sender], &response, portMAX_DELAY);
}
