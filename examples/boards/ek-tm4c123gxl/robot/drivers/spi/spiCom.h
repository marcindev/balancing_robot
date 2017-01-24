/*  Wrapper for SPI driver
 *  autor: Marcin Gozdziewski
 */
#ifndef SPICOM_H
#define SPICOM_H


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "utils/circularBuffer.h"
#include "driverlib/udma.h"

typedef enum
{
    SPI_MASTER = 1,
    SPI_SLAVE = 0
} SpiMode;

typedef enum
{
    SPI_SSI0 = 0,
    SPI_SSI1,
    SPI_SSI2,
    SPI_SSI3
} SpiModule;

typedef enum
{
    SPI_CS_AUTO,
    SPI_CS_PERMANENT,
    SPI_CS_MANUAL
} SpiCsMode;

typedef struct _SpiCom* SpiCom;

SpiCom SpiComCreate(SpiModule module, SpiMode spiMode, uint32_t buffersSize, SpiCsMode csMode);
SpiCom SpiComGetInstance(SpiModule module);
void SpiComSetIdleFrame(SpiCom spiComInst, void* frame, uint32_t size);
void SpiComInit(SpiCom spiComInst);
bool SpiComInitiated(SpiCom spiComInst);
bool SpiComGetCsState(SpiCom spiComInst);
void SpiComSetCsState(SpiCom spiComInst, bool state);
bool SpiComSend(SpiCom spiComInst, const uint8_t* data, uint32_t length);
bool SpiComReceive(SpiCom spiComInst, void* data, uint32_t length);
bool startDmaTransactionRx(SpiCom spiComInst);
bool startDmaTransactionTx(SpiCom spiComInst);
void onDmaTransactionRxEnd(SpiCom spiComInst);
void onDmaTransactionTxEnd(SpiCom spiComInst);



#endif // SPICOM_H
