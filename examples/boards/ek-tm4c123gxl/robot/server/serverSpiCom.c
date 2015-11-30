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

static SpiComInstance g_spiComInst;

static void handleGetLogs(uint16_t slot);

void initializeSpi()
{
	ZeroBuffer(&g_spiComInst, sizeof(SpiComInstance));

	g_spiComInst.spiPeripheral = SYSCTL_PERIPH_SSI0;
	g_spiComInst.gpioPeripheral = SYSCTL_PERIPH_GPIOA;
	g_spiComInst.clkPinConfig = GPIO_PA2_SSI0CLK;
	g_spiComInst.fssPinConfig = GPIO_PA3_SSI0FSS;
	g_spiComInst.rxPinConfig = GPIO_PA4_SSI0RX;
	g_spiComInst.txPinConfig = GPIO_PA5_SSI0TX;
	g_spiComInst.gpioPortBase = GPIO_PORTA_BASE;
	g_spiComInst.ssiBase = SSI0_BASE;
#ifdef _ROBOT_MASTER_BOARD
	g_spiComInst.masterSlave = SPI_MASTER;
#else
	g_spiComInst.masterSlave = SPI_SLAVE;
#endif
	g_spiComInst.clkPin = GPIO_PIN_2;
	g_spiComInst.fssPin = GPIO_PIN_3;
	g_spiComInst.rxPin = GPIO_PIN_4;
	g_spiComInst.txPin = GPIO_PIN_5;

	SpiComInit(g_spiComInst);
}

bool receiveSpiMsg(void** msg)
{
	if(!SpiComReceive(g_spiComInst, *msg, 1))
		return false;

	uint32_t msgLen = getMsgSize(*((uint8_t*) *msg));
	SpiComReceive(g_spiComInst, (*msg) + 1, msgLen);

	return true;
}

bool sendSpiMsg(void* msg)
{
	uint32_t msgLen = getMsgSize(*((uint8_t*) msg));

	if(!SpiComSend(g_spiComInst, msg, msgLen))
		return false;

	return true;
}


void handleGetLogs()
{
	uint32_t timestamp;
	LogLevel logLevel;
	LogComponent logComponent;
	const char* strPtr;
	uint8_t argsNum;
	uint8_t* argsBuffer;

	uint16_t totalLinesNum = getLinesNumber();

	GetLogsMsgRsp response = INIT_GET_LOGS_MSG_RSP;

	uint16_t lineNum = 1;

	while(getNextLogLine(&timestamp, &logLevel, &logComponent,
			(void*)&strPtr, &argsNum, (void*)&argsBuffer))
	{
		response.lineNum = lineNum++;
		response.totalLineNum = totalLinesNum;
		response.timestamp = timestamp;
		response.logLevel = (uint8_t) logLevel;
		response.component = (uint8_t) logComponent;
		strcpy(&response.strBuffer[0], strPtr);
		response.argsNum = argsNum;
		memcpy(&response.argsBuffer[0], argsBuffer, argsNum * sizeof(uint64_t));

		SpiComSend(&g_spiComInst, &response, sizeof(GetLogsMsgRsp));
	}
}



