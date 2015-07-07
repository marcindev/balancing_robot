//********************************************
// Interface for communication with i2c driver
//
//********************************************

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "messages.h"
#include "i2cManager.h"

#define RX_QUEUES_MAX_SIZE			20
#define RX_WAIT_TIME_TICKS			50
#define I2C_RX_QUEUE_SIZE       	5
#define I2C_RX_QUEUE_ITEM_SIZE     	5

extern xQueueHandle g_i2cTxQueue;

xQueueHandle g_i2cRxQueues[RX_QUEUES_MAX_SIZE] = {0};
xSemaphoreHandle g_i2cNextSenderIdSem;
uint8_t g_i2cNextSenderId = 0;
bool g_i2cSemaphoresCreated = false;

bool initI2cManager(I2cManager* i2cMng)
{
	if(!g_i2cSemaphoresCreated)
	{
		g_i2cNextSenderIdSem = xSemaphoreCreateMutex();
		g_i2cSemaphoresCreated = true;
	}

	xSemaphoreTake(g_i2cNextSenderIdSem, portMAX_DELAY);

	if(g_i2cNextSenderIdSem == RX_QUEUES_MAX_SIZE)
		return false;

	i2cMng->taskId = g_i2cNextSenderIdSem++;

	xSemaphoreGive(g_i2cNextSenderIdSem);

	size_t index = i2cMng->taskId;

	g_i2cRxQueues[index] = xQueueCreate(I2C_RX_QUEUE_SIZE, I2C_RX_QUEUE_ITEM_SIZE);

	return true;
}

bool send(I2cManager* i2cMng, uint8_t slaveAddress, const uint8_t* data, uint32_t length)
{
	size_t queueInd = i2cMng->taskId;

	I2cSendMsgReq request = INIT_I2C_SEND_MSG_REQ;
	request.slaveAddress = slaveAddress;
	request.data = data;
	request.length = length;
	request.sender = queueInd;

	if(xQueueSend(g_i2cTxQueue, &request, portMAX_DELAY) != pdPASS)
	{
		return false;
	}

	I2cSendMsgRsp response = INIT_SEND_I2C_MSG_RSP;

	if(xQueueReceive(g_i2cRxQueues[queueInd], &response, ( portTickType ) RX_WAIT_TIME_TICKS) != pdPASS)
		return false;

	if(response.msgId != I2C_SEND_MSG_RSP || !response.status)
		return false;

	return true;
}

bool receive(I2cManager* i2cMng, uint8_t slaveAddress, uint8_t* data, uint32_t length)
{
	size_t queueInd = i2cMng->taskId;

	I2cReceiveMsgReq request = INIT_I2C_RECEIVE_MSG_REQ;
	request.slaveAddress = slaveAddress;
	request.length = length;
	request.sender = queueInd;

	if(xQueueSend(g_i2cTxQueue, &request, portMAX_DELAY) != pdPASS)
	{
		return false;
	}

	I2cReceiveMsgRsp response = INIT_I2C_RECEIVE_MSG_RSP;

	if(xQueueReceive(g_i2cRxQueues[queueInd], &response, ( portTickType ) RX_WAIT_TIME_TICKS) != pdPASS)
		return false;

	if(response.msgId != I2C_RECEIVE_MSG_RSP || !response.status)
		return false;

	data = response.data;

	return true;
}

bool sendAndReceive(I2cManager* i2cMng, uint8_t slaveAddress, uint8_t* sentData,
					uint32_t sentLength, uint8_t* recvData, uint32_t recvLength)
{
	size_t queueInd = i2cMng->taskId;

	I2cSendAndReceiveMsgReq request = INIT_I2C_SEND_N_RECEIVE_MSG_REQ;
	request.slaveAddress = slaveAddress;
	request.data = sentData;
	request.sentLength = sentLength;
	request.rcvLength = recvLength;
	request.sender = queueInd;

	if(xQueueSend(g_i2cTxQueue, &request, portMAX_DELAY) != pdPASS)
	{
		return false;
	}

	I2cSendAndReceiveMsgRsp response = INIT_I2C_SEND_N_RECEIVE_MSG_RSP;

	if(xQueueReceive(g_i2cRxQueues[queueInd], &response, ( portTickType ) RX_WAIT_TIME_TICKS) != pdPASS)
		return false;

	if(response.msgId != I2C_SEND_N_RECEIVE_MSG_RSP || !response.status)
		return false;

	recvData = response.data;

	return true;
}

