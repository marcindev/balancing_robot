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
#define SPI_BUFFER_SIZE		1000
#define UDMA_RX_READ_SIZE	16


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
	if(!SpiComReceive(g_spiComInstServer, &msgId, 1))
		return false;

#ifdef _ROBOT_MASTER_BOARD
	I2cManager* i2cManager = (I2cManager*) pvPortMalloc(sizeof(I2cManager));
	GpioExpander* gpioExpander = (GpioExpander*) pvPortMalloc(sizeof(GpioExpander));
	//
	ZeroBuffer(gpioExpander, sizeof(GpioExpander));
	gpioExpander->i2cManager = i2cManager;
	gpioExpander->hwAddress		= 0x21;
	GpioExpInit(gpioExpander);
	GpioExpSetPinDirOut(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN5);
	GpioExpSetPin(gpioExpander, GPIOEXP_PORTB, GPIOEXP_PIN5);
#endif
	UARTprintf("receiveSpiMsg msgId: %d\n", msgId);
	uint32_t msgLen = getMsgSize(msgId);
	*msg = pvPortMalloc(msgLen);
#ifdef _ROBOT_MASTER_BOARD
	UARTprintf("receiveSpiMsg msgLen: %d\n", msgLen);
#endif
	if(*msg == NULL)
		return false;

	*((uint8_t*) *msg) = msgId;

	if(!SpiComReceive(g_spiComInstServer, ((uint8_t*)*msg) + 1, msgLen - 1))
		return false;
#ifdef _ROBOT_MASTER_BOARD
	uint8_t* m = (uint8_t*)*msg;
	UARTprintf("receiveSpiMsg msg: %d %d %d\n", m[0], m[1], m[2]);
	UARTprintf("receiveSpiMsg success ");
#endif
	return true;
}

bool sendSpiMsg(void* msg)
{
	uint32_t msgLen = getMsgSize(*((uint8_t*) msg));

	if(!SpiComSend(g_spiComInstServer, msg, msgLen))
		return false;
	logger(Info, Log_ServerSpiCom, "[sendSpiMsg] Message sent");
	return true;
}



