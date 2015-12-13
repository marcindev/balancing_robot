/*	Wrapper for SPI driver
 * 	autor: Marcin Gozdziewski
 */
#ifndef SPIWRAPPER_H
#define SPIWRAPPER_H


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "utils/circularBuffer.h"
#include "driverlib/udma.h"

#define SPI_MASTER 1
#define SPI_SLAVE  0

typedef struct
{
	uint32_t spiPeripheral,
			 gpioPeripheral,
			 sigTxGpioPeripheral,
			 clkPinConfig,
			 fssPinConfig,
			 rxPinConfig,
			 txPinConfig,
			 gpioPortBase,
			 sigTxPortBase,
			 ssiBase,
			 masterSlave,
			 clkPin,
			 fssPin,
			 rxPin,
			 txPin,
			 sigTxPin,
			 uDmaChannelRx,
			 uDmaChannelTx,
			 uDmaRxReadSize,
			 uDmaLastTxLen,
			 uDmaLastRxLen;

	CircularBuffer* circBufferRx, *circBufferTx;
	bool enableInt;
	bool initiated;
	bool isBuffRollover;
	uint32_t errState;
} SpiComInstance;

void SpiComInit(SpiComInstance* i2cComInst);
bool SpiComInitiated(SpiComInstance* i2cComInst);
bool SpiComSend(SpiComInstance* i2cComInst, const uint8_t* data, uint32_t length);
bool SpiComReceive(SpiComInstance* i2cComInst, void** data, uint32_t* length);
bool startDmaTransactionRx(SpiComInstance* spiComInst);
bool startDmaTransactionTx(SpiComInstance* spiComInst);
void onDmaTransactionRxEnd(SpiComInstance* spiComInst);
void onDmaTransactionTxEnd(SpiComInstance* spiComInst);



#endif // SPIWRAPPER_H
