//*****************************************************************************
//
//
//*****************************************************************************
#include <string.h>
#include "FreeRTOS.h"
#include "messages.h"
#include "msgSystem.h"
#include "utils.h"
#include "global_defs.h"
#include "serverSpiCom.h"
#include "logger.h"
#include "spiWrapper.h"
#include "circularBuffer.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.h"


#include "MCP23017.h"

SpiComInstance* g_spiComInstServer = NULL;
#define SPI_BUFFER_SIZE     4096
#define UDMA_RX_READ_SIZE   16

#define ROUTE_TABLE_SIZE                10

typedef struct
{
    MsgAddress adresses[ROUTE_TABLE_SIZE];
    uint8_t index;
} RoutingTable;

static RoutingTable g_routingTable;
static bool g_isFirstUpdate = true;

static bool routeMsg(void* msg);
//static bool updateRoutingTable(void* msg);


void initializeSpi()
{
    void* bufferRx = pvPortMalloc(SPI_BUFFER_SIZE);
    void* bufferTx = pvPortMalloc(SPI_BUFFER_SIZE);
    CircularBuffer* circBufferRx = (CircularBuffer*) pvPortMalloc(sizeof(CircularBuffer));
    CircularBuffer* circBufferTx = (CircularBuffer*) pvPortMalloc(sizeof(CircularBuffer));
    g_spiComInstServer = (SpiComInstance*) pvPortMalloc(sizeof(SpiComInstance));

    ZeroBuffer(g_spiComInstServer, sizeof(SpiComInstance));
    ZeroBuffer(circBufferRx, sizeof(SpiComInstance));
    ZeroBuffer(circBufferTx, sizeof(SpiComInstance));
    ZeroBuffer(bufferRx, sizeof(SPI_BUFFER_SIZE));
    ZeroBuffer(bufferTx, sizeof(SPI_BUFFER_SIZE));

    CB_setBuffer(circBufferRx, bufferRx, SPI_BUFFER_SIZE);
    CB_setBuffer(circBufferTx, bufferTx, SPI_BUFFER_SIZE);
    g_spiComInstServer->circBufferRx = circBufferRx;
    g_spiComInstServer->circBufferTx = circBufferTx;

    g_spiComInstServer->spiPeripheral = SYSCTL_PERIPH_SSI0;
    g_spiComInstServer->gpioPeripheral = SYSCTL_PERIPH_GPIOA;
    g_spiComInstServer->sigTxGpioPeripheral = SYSCTL_PERIPH_GPIOC;
    g_spiComInstServer->clkPinConfig = GPIO_PA2_SSI0CLK;
    g_spiComInstServer->fssPinConfig = GPIO_PA3_SSI0FSS;
    g_spiComInstServer->rxPinConfig = GPIO_PA4_SSI0RX;
    g_spiComInstServer->txPinConfig = GPIO_PA5_SSI0TX;
    g_spiComInstServer->gpioPortBase = GPIO_PORTA_BASE;
    g_spiComInstServer->sigTxPortBase = GPIO_PORTC_BASE;
    g_spiComInstServer->ssiBase = SSI0_BASE;
#ifdef _ROBOT_MASTER_BOARD
    g_spiComInstServer->masterSlave = SPI_MASTER;
#else
    g_spiComInstServer->masterSlave = SPI_SLAVE;
#endif
    g_spiComInstServer->clkPin = GPIO_PIN_2;
    g_spiComInstServer->fssPin = GPIO_PIN_3;
    g_spiComInstServer->rxPin = GPIO_PIN_4;
    g_spiComInstServer->txPin = GPIO_PIN_5;
    g_spiComInstServer->sigTxPin = GPIO_PIN_6;
    g_spiComInstServer->enableInt = true;
    g_spiComInstServer->uDmaRxReadSize = UDMA_RX_READ_SIZE;
    g_spiComInstServer->uDmaChannelRx = UDMA_CHANNEL_SSI0RX;
    g_spiComInstServer->uDmaChannelTx = UDMA_CHANNEL_SSI0TX;


    SpiComInit(g_spiComInstServer);
}

bool receiveSpiMsg(void** msg)
{
    uint8_t msgId = 0;
    uint32_t len = 0;
    if(!SpiComReceive(g_spiComInstServer, msg, &len))
        return false;

    if(*msg == NULL)
        return false;

#ifdef _ROBOT_MASTER_BOARD
    if(g_isFirstUpdate)
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
//  printBuffer(msg, msgLen);
    if(!SpiComSend(g_spiComInstServer, msg, msgLen))
    {
        logger(Error, Log_ServerSpiCom, "[sendSpiMsg] Couldn't send msg");
        return false;
    }
    portTickType ui32WakeTime;
    ui32WakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&ui32WakeTime, pdMS_TO_TICKS(2));
//  UARTprintf("sendSpiMsg\n");
//  logger(Info, Log_ServerSpiCom, "[sendSpiMsg] Message sent");
    return true;
}

bool routeMsg(void* msg)
{
    MsgHeader* msgHeader = (MsgHeader*) msg;

    for(int i = 0; i != g_routingTable.index; ++i)
    {
        if(g_routingTable.adresses[i].slot == msgHeader->slot)
        {
            msgHeader->queueId = g_routingTable.adresses[i].queueId;
            return true;
        }
    }

    logger(Error, Log_ServerSpiCom, "[routeMsg] Couldn't find slot");
    return false;
}

bool updateRoutingTable(void* msg)
{
    g_isFirstUpdate = false;

    MsgHeader* msgHeader = (MsgHeader*) msg;

    for(int i = 0; i != g_routingTable.index; ++i)
    {
        if(g_routingTable.adresses[i].slot == msgHeader->slot)
        {
            g_routingTable.adresses[i].queueId = msgHeader->queueId;
            return true;
        }
    }

    if(g_routingTable.index < ROUTE_TABLE_SIZE - 1)
    {
        uint8_t index = g_routingTable.index++;
        g_routingTable.adresses[index].slot = msgHeader->slot;
        g_routingTable.adresses[index].queueId = msgHeader->queueId;
        return true;
    }

    return false;
}

