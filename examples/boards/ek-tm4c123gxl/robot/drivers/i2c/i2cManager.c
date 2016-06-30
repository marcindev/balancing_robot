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
#include "portable.h"
#include <string.h>

#define RX_QUEUES_MAX_SIZE          15
#define RX_WAIT_TIME_TICKS          50
#define I2C_RX_QUEUE_SIZE           5
#define I2C_RX_QUEUE_ITEM_SIZE      5

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

    if(g_i2cNextSenderId == RX_QUEUES_MAX_SIZE)
        return false;

    i2cMng->taskId = g_i2cNextSenderId++;

    xSemaphoreGive(g_i2cNextSenderIdSem);

    size_t index = i2cMng->taskId;

    g_i2cRxQueues[index] = xQueueCreate(I2C_RX_QUEUE_SIZE, I2C_RX_QUEUE_ITEM_SIZE);

    return true;
}

bool i2cSend(I2cManager* i2cMng, uint8_t slaveAddress, const uint8_t* data, uint32_t length)
{
    size_t queueInd = i2cMng->taskId;
    I2cSendMsgReq req;
//  I2cSendMsgReq* request = (I2cSendMsgReq*) pvPortMalloc(sizeof(I2cSendMsgReq));
    I2cSendMsgReq* request = &req;
    if(!request)
        return false; // out of memory

    *request = INIT_I2C_SEND_MSG_REQ;
    request->slaveAddress = slaveAddress;
    request->data = (uint8_t*)data;
    request->length = length;
    request->header.queueId = queueInd;

    if(xQueueSend(g_i2cTxQueue, (void*) &request, portMAX_DELAY) != pdPASS)
    {
        return false;
    }
#ifndef _DISABLE_I2C_ACK
    I2cSendMsgRsp* response;

    if(xQueueReceive(g_i2cRxQueues[queueInd], &response, ( portTickType ) RX_WAIT_TIME_TICKS) != pdPASS)
        return false;

    if(response->header.msgId != I2C_SEND_MSG_RSP || !response->status)
    {
        vPortFree(response);
        return false;
    }

    vPortFree(response);
#endif
    return true;
}

bool i2cReceive(I2cManager* i2cMng, uint8_t slaveAddress, uint8_t* data, uint32_t length)
{
    size_t queueInd = i2cMng->taskId;

    I2cReceiveMsgReq req;
//  I2cReceiveMsgReq* request = (I2cReceiveMsgReq*) pvPortMalloc(sizeof(I2cReceiveMsgReq));
    I2cReceiveMsgReq* request = &req; // TODO: dynamic allocation overrides stack memory
    if(!request)
        return false; // out of memory

    *request = INIT_I2C_RECEIVE_MSG_REQ;
    request->slaveAddress = slaveAddress;
    request->length = length;
    request->header.queueId = queueInd;

    if(xQueueSend(g_i2cTxQueue, (void*) &request, portMAX_DELAY) != pdPASS)
    {
        return false;
    }

    I2cReceiveMsgRsp* response;

    if(xQueueReceive(g_i2cRxQueues[queueInd], &response, ( portTickType ) RX_WAIT_TIME_TICKS) != pdPASS)
        return false;

    if(response->header.msgId != I2C_RECEIVE_MSG_RSP || !response->status)
    {
        vPortFree(response);
        return false;
    }

    data = response->data;

    vPortFree(response);
    return true;
}

bool i2cSendAndReceive(I2cManager* i2cMng, uint8_t slaveAddress, uint8_t* sentData,
                    uint32_t sentLength, uint8_t* recvData, uint32_t recvLength)
{
    size_t queueInd = i2cMng->taskId;
    I2cSendAndReceiveMsgReq req;
//  I2cSendAndReceiveMsgReq* request = (I2cSendAndReceiveMsgReq*) pvPortMalloc(sizeof(I2cSendAndReceiveMsgReq));
    I2cSendAndReceiveMsgReq* request = &req;
    if(!request)
        return false; // out of memory

    *request = INIT_I2C_SEND_N_RECEIVE_MSG_REQ;
    request->slaveAddress = slaveAddress;
    request->data = sentData;
    request->sentLength = sentLength;
    request->rcvLength = recvLength;
    request->header.queueId = queueInd;

    if(xQueueSend(g_i2cTxQueue, (void*) &request, portMAX_DELAY) != pdPASS)
    {
        return false;
    }

    I2cSendAndReceiveMsgRsp* response;

    if(xQueueReceive(g_i2cRxQueues[queueInd], &response, ( portTickType ) RX_WAIT_TIME_TICKS) != pdPASS)
        return false;

    if(response->header.msgId != I2C_SEND_N_RECEIVE_MSG_RSP || !response->status)
    {
        vPortFree(response);
        return false;
    }

    memcpy(recvData, response->data, response->length);

    vPortFree(response->data);
    vPortFree(response);
    return true;
}

