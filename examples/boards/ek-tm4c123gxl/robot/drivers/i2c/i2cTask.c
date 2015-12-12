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
#include "utils.h"
#include "i2cTask.h"
#include "logger.h"

#define I2C_TASK_STACK_SIZE		100        // Stack size in words
#define I2C_QUEUE_SIZE			 10
#define I2C_ITEM_SIZE			  4			// bytes
#define I2C_MSG_WAIT_TIME	 0xFFFF

extern xQueueHandle g_i2cRxQueues[];
xQueueHandle g_i2cTxQueue;
static I2CComInstance g_i2cInstance;

// TODO consider using dynamically allocated messages instead of automatic in case those didn't work
// ... and other variables

static void initializeI2c();
static void i2cHandleSend(I2cSendMsgReq* request);
static void i2cHandleReceive(I2cReceiveMsgReq* request);
static void i2cHandleSendAndReceive(I2cSendAndReceiveMsgReq* request);

static void i2cTask(void *pvParameters)
{
	initializeI2c();

	while(true)
	{
		void* msg;
        if(xQueueReceive(g_i2cTxQueue, &msg, I2C_MSG_WAIT_TIME) == pdPASS)
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
        		logger(Error, Log_I2CTask, "[i2cTask] Received not-recognized message");
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
	result = I2CComSend(&g_i2cInstance, request->slaveAddress, request->data, request->length);
#ifndef _DISABLE_I2C_ACK
	I2cSendMsgRsp* response = (I2cSendMsgRsp*) pvPortMalloc(sizeof(I2cSendMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_SEND_I2C_MSG_RSP;
	response->status = result;

	xQueueSend(g_i2cRxQueues[request->sender], (void*) &response, portMAX_DELAY);
#endif
	//vPortFree(request);
}

void i2cHandleReceive(I2cReceiveMsgReq* request)
{
	bool result;
	uint8_t* data = (uint8_t*) pvPortMalloc(request->length);

	if(!data)
		return;

	result = I2CComReceive(&g_i2cInstance, request->slaveAddress, data, request->length);

	I2cReceiveMsgRsp* response = (I2cReceiveMsgRsp*) pvPortMalloc(sizeof(I2cReceiveMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_I2C_RECEIVE_MSG_RSP;
	response->status = result;
	response->data = data;
	response->length = request->length;

	xQueueSend(g_i2cRxQueues[request->sender], (void*) &response, portMAX_DELAY);

	//vPortFree(request);
}

void i2cHandleSendAndReceive(I2cSendAndReceiveMsgReq* request)
{
	bool result;
	uint8_t* data = (uint8_t*) pvPortMalloc(request->rcvLength);

	if(!data)
		return;

	I2cSendAndReceiveMsgRsp* response = (I2cSendAndReceiveMsgRsp*) pvPortMalloc(sizeof(I2cSendAndReceiveMsgRsp));
	if(!response)
		return; // out of memory

	*response = INIT_I2C_SEND_N_RECEIVE_MSG_RSP;

	result = I2CComSend(&g_i2cInstance, request->slaveAddress, request->data, request->sentLength);

	if(!result)
	{
		response->status = 0;
		xQueueSend(g_i2cRxQueues[request->sender], (void*) &response, portMAX_DELAY);

		return;
	}

	result = I2CComReceive(&g_i2cInstance, request->slaveAddress, data, request->rcvLength);

	response->status = result;
	response->data = data;
	response->length = request->rcvLength;

	xQueueSend(g_i2cRxQueues[request->sender], (void*) &response, portMAX_DELAY);

	//vPortFree(request);
}

void initializeI2c()
{
	ZeroBuffer(&g_i2cInstance, sizeof(I2CComInstance));

	// setup i2c communication
	g_i2cInstance.gpioPeripheral 	= SYSCTL_PERIPH_GPIOA;
	g_i2cInstance.gpioPortBase 		= GPIO_PORTA_BASE;
	g_i2cInstance.i2cBase 			= I2C1_BASE;
	g_i2cInstance.i2cPeripheral 	= SYSCTL_PERIPH_I2C1;
	g_i2cInstance.sclPin 			= GPIO_PIN_6;
	g_i2cInstance.sclPinConfig 		= GPIO_PA6_I2C1SCL;
	g_i2cInstance.sdaPin 			= GPIO_PIN_7;
	g_i2cInstance.sdaPinConfig		= GPIO_PA7_I2C1SDA;
	g_i2cInstance.speed				= I2C_SPEED_400;

	I2CComInit(&g_i2cInstance);

	logger(Info, Log_I2CTask, "[initializeI2c] I2C initialized");
}

