//*****************************************************************************
//
//
//*****************************************************************************
#include "memory.h"
#include <string.h>
#include "FreeRTOS.h"
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "global_defs.h"
#include "serverSpiCom.h"
#include "logger.h"
#include "spiCom.h"
#include "circularBuffer.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.h"


#include "MCP23017.h"

#define SPI_BUFFER_SIZE     4096
#define UDMA_RX_READ_SIZE   16

#define ROUTE_TABLE_SIZE                10

#define SPI_START_FRAME             0xF1
#define SPI_DUMMY_FRAME_ID          0xE1
#define SPI_DATA_FRAME_ID           0xE2
#define SPI_ACK_FRAME_ID            0xE3

#define SPI_DUMMY_FRAME_LEN         0x08
#define SPI_HEADER_LEN              0x03
 
static uint8_t dummyFrame[SPI_DUMMY_FRAME_LEN] = 
{  
    SPI_START_FRAME,
    SPI_DUMMY_FRAME_ID,
    SPI_DUMMY_FRAME_LEN - 3
};


typedef struct
{
    MsgAddress adresses[ROUTE_TABLE_SIZE];
    uint8_t index;
} RoutingTable;

static SpiCom spiComInst;
static RoutingTable routingTable;
static bool isFirstUpdate = true;

static bool routeMsg(void* msg);
//static bool updateRoutingTable(void* msg);


void initializeSpi()
{
#ifdef _ROBOT_MASTER_BOARD
    SpiMode spiMode = SPI_MASTER;
#else
    SpiMode spiMode = SPI_SLAVE;
#endif
 
    spiComInst = SpiComCreate(SPI_SSI0, spiMode, SPI_BUFFER_SIZE, SPI_CS_AUTO);

    if(!spiComInst)
	return;

    SpiComSetIdleFrame(spiComInst, dummyFrame, SPI_DUMMY_FRAME_LEN);
    SpiComInit(spiComInst);
}

bool receiveSpiMsg(void** msg)
{
    uint8_t msgId = 0;
    uint32_t len = 0;
    uint8_t byte = 0;
    bool dataFramFound = false;

    while(!dataFramFound && SpiComReceive(spiComInst, &byte, 1))
    {
        if(byte != SPI_START_FRAME)
            continue;

        byte = 0;

        if(!SpiComReceive(spiComInst, &byte, 1))
	    return false;

        switch(byte)
        {
        case SPI_DUMMY_FRAME_ID:
        {
            uint8_t len = 0;

            if(!SpiComReceive(spiComInst, &len, 1))
		return false;

            if(!len)
                return false;

            uint8_t dummyData[len];
	    
	    if(!SpiComReceive(spiComInst, dummyData, len))
		return false;

            break;
        }
        case SPI_DATA_FRAME_ID:
            dataFramFound = true;
            break;

        default:
            break;
        }
    }

    if(!dataFramFound)
        return false;

    len = 0;

    if(!SpiComReceive(spiComInst, &len, 1))
	return false;

    *msg = malloc(len);

    if(!(*msg))
    {
        return false;
    }

    if(!SpiComReceive(spiComInst, *msg, len))
	return false;

#ifdef _ROBOT_MASTER_BOARD
    if(isFirstUpdate)
    {
        if(!updateRoutingTable(*msg))
        {
            logger(Error, Log_ServerSpiCom, "[receiveSpiMsg] Couldn't update routing table");
            return false;
        }
    }
#endif

    return true;
}

bool sendSpiMsg(void* msg)
{
    uint32_t msgLen = getMsgSize(msg);

#ifdef _ROBOT_MASTER_BOARD
    if(!routeMsg(msg))
        return false;
#endif
    uint8_t* message = (uint8_t*) malloc(SPI_HEADER_LEN + msgLen);

    if(!message)
    {
        logger(Error, Log_ServerSpiCom, "[sendSpiMsg] Out of memory");
        return false;
    }

    message[0] = SPI_START_FRAME;
    message[1] = SPI_DATA_FRAME_ID;
    message[2] = msgLen;

    memcpy(message + SPI_HEADER_LEN, msg, msgLen);

    if(!SpiComSend(spiComInst, message, SPI_HEADER_LEN + msgLen))
    {
        logger(Error, Log_ServerSpiCom, "[sendSpiMsg] Couldn't send msg");
        return false;
    }

    portTickType ui32WakeTime;
    ui32WakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2));

    return true;
}

bool routeMsg(void* msg)
{
    MsgHeader* msgHeader = (MsgHeader*) msg;

    for(int i = 0; i != routingTable.index; ++i)
    {
        if(routingTable.adresses[i].slot == msgHeader->slot)
        {
            msgHeader->queueId = routingTable.adresses[i].queueId;
            return true;
        }
    }

    logger(Error, Log_ServerSpiCom, "[routeMsg] Couldn't find slot");
    return false;
}

bool updateRoutingTable(void* msg)
{
    isFirstUpdate = false;

    MsgHeader* msgHeader = (MsgHeader*) msg;

    for(int i = 0; i != routingTable.index; ++i)
    {
        if(routingTable.adresses[i].slot == msgHeader->slot)
        {
            routingTable.adresses[i].queueId = msgHeader->queueId;
            return true;
        }
    }

    if(routingTable.index < ROUTE_TABLE_SIZE - 1)
    {
        uint8_t index = routingTable.index++;
        routingTable.adresses[index].slot = msgHeader->slot;
        routingTable.adresses[index].queueId = msgHeader->queueId;
        return true;
    }

    return false;
}

