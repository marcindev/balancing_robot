/*  Wrapper for Spi driver
 *  autor: Marcin Gozdziewski
 */
#include "memory.h"
#include "spiCom.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"
#include "udmaWrapper.h"
#include "utils.h"

#define MODULES_NUM  4
#define UDMA_RX_READ_SIZE	     16
#define DEFAULT_IDLE_FRAME_SIZE      8

static void SpiEmptyRxFifo(SpiCom spiComInst);
static bool isDMAtransactionOngoingRx(SpiCom spiComInst);
static bool isDMAtransactionOngoingTx(SpiCom spiComInst);

typedef enum
{
    SPI_UDMA_RX,
    SPI_UDMA_TX
} UdmaDirection;

struct _SpiCom
{
    SpiModule module; 
    uint32_t spiPeripheral,
	     gpioPeripheral,
	     clkPinConfig,
	     fssPinConfig,
	     rxPinConfig,
	     txPinConfig,
	     gpioPortBase,
	     ssiBase,
	     clkPin,
	     fssPin,
	     rxPin,
	     txPin,
	     interrupt;

    CircularBuffer* circBufferRx, *circBufferTx;
    UdmaChannel udmaChannelRx, udmaChannelTx;
    uint32_t uDmaLastRxLen,
	     uDmaLastTxLen,
	     uDmaRxReadSize;
    uint32_t buffersSize;
    void* idleFrame;
    uint32_t idleFrameSize;
    SpiMode spiMode;
    SpiCsMode csMode;
    bool csState;
    bool initiated;
    bool isBuffRollover;
    uint32_t errState;
};

static uint8_t defaultIdleFrame[DEFAULT_IDLE_FRAME_SIZE] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static SpiCom spiComInstances[MODULES_NUM];

static uint32_t spiModuleToUdmaChannelNum(SpiModule module, UdmaDirection dir)
{
    switch(dir)
    {
    case SPI_UDMA_RX:
	switch(module)
	{
	    case SPI_SSI0:
		return UDMA_CHANNEL_SSI0RX;
	    case SPI_SSI1:
		return UDMA_CHANNEL_SSI1RX;
	    case SPI_SSI2:
		uDMAChannelAssign(UDMA_CH12_SSI2RX);
		return 12;
	    case SPI_SSI3:
		uDMAChannelAssign(UDMA_CH14_SSI3RX);
		return 14;
	    default:
		return 0;
	}
	break;
    case SPI_UDMA_TX:
	switch(module)
	{
	    case SPI_SSI0:
		return UDMA_CHANNEL_SSI0TX;
	    case SPI_SSI1:
		return UDMA_CHANNEL_SSI1TX;
	    case SPI_SSI2:
		uDMAChannelAssign(UDMA_CH13_SSI2TX);
		return 13;
	    case SPI_SSI3:
		uDMAChannelAssign(UDMA_CH15_SSI3TX);
		return 15;
	    default:
		return 0;
	}
	break;
    default:
	break;
    }

    return 0;
}

void setPeripheralParams(SpiCom spiComInst, SpiModule module)
{
    switch(module)
    {
    case SPI_SSI0: 
	spiComInst->spiPeripheral = SYSCTL_PERIPH_SSI0;
	spiComInst->gpioPeripheral = SYSCTL_PERIPH_GPIOA;
	spiComInst->clkPinConfig = GPIO_PA2_SSI0CLK;
	spiComInst->fssPinConfig = GPIO_PA3_SSI0FSS;
	spiComInst->rxPinConfig = GPIO_PA4_SSI0RX;
	spiComInst->txPinConfig = GPIO_PA5_SSI0TX;
	spiComInst->gpioPortBase = GPIO_PORTA_BASE;
	spiComInst->ssiBase = SSI0_BASE;
	spiComInst->clkPin = GPIO_PIN_2;
	spiComInst->fssPin = GPIO_PIN_3;
	spiComInst->rxPin = GPIO_PIN_4;
	spiComInst->txPin = GPIO_PIN_5;
	spiComInst->interrupt = INT_SSI0;
	break;
    case SPI_SSI1: 
	spiComInst->spiPeripheral = SYSCTL_PERIPH_SSI1;
	spiComInst->gpioPeripheral = SYSCTL_PERIPH_GPIOF;
	spiComInst->clkPinConfig = GPIO_PF2_SSI1CLK;
	spiComInst->fssPinConfig = GPIO_PF3_SSI1FSS;
	spiComInst->rxPinConfig = GPIO_PF0_SSI1RX;
	spiComInst->txPinConfig = GPIO_PF1_SSI1TX;
	spiComInst->gpioPortBase = GPIO_PORTF_BASE;
	spiComInst->ssiBase = SSI1_BASE;
	spiComInst->clkPin = GPIO_PIN_2;
	spiComInst->fssPin = GPIO_PIN_3;
	spiComInst->rxPin = GPIO_PIN_0;
	spiComInst->txPin = GPIO_PIN_1;
	spiComInst->interrupt = INT_SSI1;
	break;
    case SPI_SSI2: 
	spiComInst->spiPeripheral = SYSCTL_PERIPH_SSI2;
	spiComInst->gpioPeripheral = SYSCTL_PERIPH_GPIOB;
	spiComInst->clkPinConfig = GPIO_PB4_SSI2CLK;
	spiComInst->fssPinConfig = GPIO_PB5_SSI2FSS;
	spiComInst->rxPinConfig = GPIO_PB6_SSI2RX;
	spiComInst->txPinConfig = GPIO_PB7_SSI2TX;
	spiComInst->gpioPortBase = GPIO_PORTB_BASE;
	spiComInst->ssiBase = SSI2_BASE;
	spiComInst->clkPin = GPIO_PIN_2;
	spiComInst->fssPin = GPIO_PIN_3;
	spiComInst->rxPin = GPIO_PIN_0;
	spiComInst->txPin = GPIO_PIN_1;
	spiComInst->interrupt = INT_SSI2;
	break;
    case SPI_SSI3: 
	spiComInst->spiPeripheral = SYSCTL_PERIPH_SSI3;
	spiComInst->gpioPeripheral = SYSCTL_PERIPH_GPIOD;
	spiComInst->clkPinConfig = GPIO_PD0_SSI3CLK;
	spiComInst->fssPinConfig = GPIO_PD1_SSI3FSS;
	spiComInst->rxPinConfig = GPIO_PD2_SSI3RX;
	spiComInst->txPinConfig = GPIO_PD3_SSI3TX;
	spiComInst->gpioPortBase = GPIO_PORTD_BASE;
	spiComInst->ssiBase = SSI3_BASE;
	spiComInst->clkPin = GPIO_PIN_0;
	spiComInst->fssPin = GPIO_PIN_1;
	spiComInst->rxPin = GPIO_PIN_2;
	spiComInst->txPin = GPIO_PIN_3;
	spiComInst->interrupt = INT_SSI3;
	break;

    default:
	break;
    }

}

SpiCom SpiComCreate(SpiModule module, SpiMode spiMode, uint32_t buffersSize, SpiCsMode csMode)
{
    if(spiComInstances[module])
	return 0;

    SpiCom spiCom = malloc(sizeof(struct _SpiCom));
    ZeroBuffer(spiCom, sizeof(struct _SpiCom));

    setPeripheralParams(spiCom, module);
    spiCom->module = module;
    spiCom->buffersSize = buffersSize;
    spiCom->spiMode = spiMode;
    spiCom->csMode = csMode;
    spiCom->idleFrame = defaultIdleFrame;
    spiCom->idleFrameSize = DEFAULT_IDLE_FRAME_SIZE;
    spiCom->circBufferTx = (CircularBuffer*) malloc(sizeof(CircularBuffer));

    if(!spiCom->circBufferTx)
    {
	free(spiCom);
	return 0;
    }

    ZeroBuffer(spiCom->circBufferTx, sizeof(CircularBuffer));

    uint8_t* rawBuffTx = (uint8_t*) malloc(spiCom->buffersSize);

    if(!rawBuffTx)
    {
	free(spiCom->circBufferTx);
	free(spiCom);

	return 0;
    }

    ZeroBuffer(rawBuffTx, spiCom->buffersSize);

    CB_setBuffer(spiCom->circBufferTx, rawBuffTx, spiCom->buffersSize);

    spiCom->circBufferRx = (CircularBuffer*) malloc(sizeof(CircularBuffer));
    
    if(!spiCom->circBufferRx)
    {
	free(spiCom->circBufferTx);
	free(rawBuffTx);
	free(spiCom);
	return 0;
    }

    ZeroBuffer(spiCom->circBufferRx, sizeof(CircularBuffer));

    uint8_t* rawBuffRx = (uint8_t*) malloc(spiCom->buffersSize);

    if(!rawBuffRx)
    {
	free(spiCom->circBufferTx);
	free(rawBuffTx);
	free(spiCom->circBufferRx);
	free(spiCom);

	return 0;
    }

    ZeroBuffer(rawBuffRx, spiCom->buffersSize);

    CB_setBuffer(spiCom->circBufferRx, rawBuffRx, spiCom->buffersSize);

    spiComInstances[module] = spiCom;

    return spiCom;
}

SpiCom SpiComGetInstance(SpiModule module)
{
    return spiComInstances[module];
}

// Buffers cannot be read/written to/from flash
void SpiComSetIdleFrame(SpiCom spiComInst, void* frame, uint32_t size)
{
    spiComInst->idleFrame = frame;
    spiComInst->idleFrameSize = DEFAULT_IDLE_FRAME_SIZE;
}

void SpiComInit(SpiCom spiComInst)
{
    if(spiComInst->initiated)
        return;

    spiComInst->uDmaLastTxLen = 0;
    spiComInst->uDmaLastRxLen = 0;
    spiComInst->uDmaRxReadSize = UDMA_RX_READ_SIZE; 

    UdmaInit();

    SysCtlPeripheralEnable(spiComInst->spiPeripheral);
    SysCtlPeripheralEnable(spiComInst->gpioPeripheral);
    GPIOPinConfigure(spiComInst->clkPinConfig);
    GPIOPinConfigure(spiComInst->fssPinConfig);
    GPIOPinConfigure(spiComInst->rxPinConfig);
    GPIOPinConfigure(spiComInst->txPinConfig);

    uint32_t ssiPins = spiComInst->txPin | spiComInst->rxPin |spiComInst->clkPin;

    if(spiComInst->csMode == SPI_CS_AUTO) ssiPins |= spiComInst->fssPin; 

    GPIOPinTypeSSI(spiComInst->gpioPortBase, ssiPins);
    
    if(spiComInst->csMode == SPI_CS_PERMANENT || spiComInst->csMode == SPI_CS_MANUAL)
    {
	GPIOPinTypeGPIOOutput(spiComInst->gpioPortBase, spiComInst->fssPin);
    }

    SSIConfigSetExpClk(spiComInst->ssiBase,
                       SysCtlClockGet(),
                       SSI_FRF_MOTO_MODE_0,
                       spiComInst->spiMode == SPI_MASTER ? SSI_MODE_MASTER : SSI_MODE_SLAVE,
                       400000,
                       8);

    SSIEnable(spiComInst->ssiBase);

    SSIDMAEnable(spiComInst->ssiBase, SSI_DMA_RX | SSI_DMA_TX);
    SSIIntEnable(spiComInst->ssiBase, SSI_DMATX | SSI_DMARX | SSI_RXOR | SSI_RXTO);

    initInterrupts();

    IntPrioritySet(INT_UDMAERR, 6 << 5);    // priority 5 (3 top bits)
    IntEnable(INT_UDMAERR);

    IntPrioritySet(spiComInst->interrupt, 3 << 5);   // priority 3 (3 top bits)
    IntEnable(spiComInst->interrupt);

    spiComInst->udmaChannelRx = UdmaCreateChannel(spiModuleToUdmaChannelNum(spiComInst->module, SPI_UDMA_RX),
						 false, true);    
    UdmaChannelInit(spiComInst->udmaChannelRx);

    spiComInst->udmaChannelTx = UdmaCreateChannel(spiModuleToUdmaChannelNum(spiComInst->module, SPI_UDMA_TX),
						  true, false);    
    UdmaChannelInit(spiComInst->udmaChannelTx);


    SpiEmptyRxFifo(spiComInst);

    spiComInst->initiated = true;

    if(spiComInst->csMode == SPI_CS_PERMANENT)
    {
	GPIOPinWrite(spiComInst->gpioPortBase, spiComInst->fssPin, 1);
	spiComInst->csState = true;
    }

    if(!isDMAtransactionOngoingRx(spiComInst))
        startDmaTransactionRx(spiComInst);

    if(!isDMAtransactionOngoingTx(spiComInst))
        startDmaTransactionTx(spiComInst);
}

bool SpiComInitiated(SpiCom spiComInst)
{
    return spiComInst->initiated;
}

bool SpiComGetCsState(SpiCom spiComInst)
{
   return spiComInst->csState; 
}

void SpiComSetCsState(SpiCom spiComInst, bool state)
{
    if(spiComInst->csMode != SPI_CS_MANUAL)
	return;

    GPIOPinWrite(spiComInst->gpioPortBase, spiComInst->fssPin, state ? 1 : 0);
    spiComInst->csState = state;
}

bool SpiComSend(SpiCom spiComInst, const uint8_t* data, uint32_t length)
{
    if((!length) || (!spiComInst->initiated))
        return false;

    if(CB_isFull(spiComInst->circBufferTx))
    {
        return false;
    }

    if(CB_getAvailableSpace(spiComInst->circBufferTx) <= length)
    {
        return false;
    }

    if(!CB_pushData(spiComInst->circBufferTx, data, length))
    {
        return false;
    }

    if(!isDMAtransactionOngoingTx(spiComInst))
        startDmaTransactionTx(spiComInst);

    return true;
}

bool SpiComReceive(SpiCom spiComInst, void* data, uint32_t len)
{
    if(!spiComInst->initiated)
        return false;

    if(CB_isEmpty(spiComInst->circBufferRx))
        return false;

    uint32_t leftToPop = len;

    while(leftToPop > 0)
    {
        leftToPop -= CB_popData(spiComInst->circBufferRx, data + len - leftToPop, leftToPop);

    }

    return true;
}

bool isDMAtransactionOngoingRx(SpiCom spiComInst)
{
    return UdmaIsTransferOngoing(spiComInst->udmaChannelRx);
}

bool isDMAtransactionOngoingTx(SpiCom spiComInst)
{
    return UdmaIsTransferOngoing(spiComInst->udmaChannelTx);
}

bool startDmaTransactionRx(SpiCom spiComInst)
{

    if(CB_isFull(spiComInst->circBufferRx)) // TODO: consider loop
    {
        return false;
    }

    if(CB_getAvailableSpace(spiComInst) <= spiComInst->uDmaRxReadSize)
    {
        return false;
    }

    uint8_t* head = spiComInst->circBufferRx->head;
    uint8_t* tail = spiComInst->circBufferRx->tail;
    uint8_t* circBuffBeg = spiComInst->circBufferRx->bufferStart;
    uint8_t* circBuffEnd = spiComInst->circBufferRx->bufferEnd;

    uint8_t* bufferPtr = 0;
    uint32_t bufferSize = 0;

    bufferPtr = head;

    if(head >= tail)
    {

        if((head + spiComInst->uDmaRxReadSize) >= circBuffEnd)
        {
            bufferSize = circBuffEnd - head;
            if(tail == circBuffBeg)
                bufferSize -= 1;
        }
        else
            bufferSize = spiComInst->uDmaRxReadSize;
    }
    else
    {
        if((head + spiComInst->uDmaRxReadSize) >= tail)
            bufferSize = (tail - 1) - head;
        else
            bufferSize = spiComInst->uDmaRxReadSize;
    }


    spiComInst->uDmaLastRxLen = bufferSize;
    
    UdmaSetBuffers(spiComInst->udmaChannelRx, 
		  (void *)(spiComInst->ssiBase + SSI_O_DR),
		  bufferPtr,
		  bufferSize);

    UdmaStartTransfer(spiComInst->udmaChannelRx);

    return true;
}


bool startDmaTransactionTx(SpiCom spiComInst)
{
    uint8_t* bufferPtr = 0;
    uint32_t bufferSize = 0;

    if(CB_isEmpty(spiComInst->circBufferTx))
    {
        bufferPtr = spiComInst->idleFrame;
        bufferSize = spiComInst->idleFrameSize;
        spiComInst->uDmaLastTxLen = 0;
    }
    else
    {
        uint8_t* head = spiComInst->circBufferTx->head;
        uint8_t* tail = spiComInst->circBufferTx->tail;
        uint8_t* circBuffBeg = spiComInst->circBufferTx->bufferStart;
        uint8_t* circBuffEnd = spiComInst->circBufferTx->bufferEnd;

        if(head > tail)
        {
            bufferPtr = tail;
            bufferSize = head - tail;
        }
        else
        {
            bufferPtr = tail;
            bufferSize = circBuffEnd - tail;
        }

        spiComInst->uDmaLastTxLen = bufferSize;
    }

    UdmaSetBuffers(spiComInst->udmaChannelTx,
		  bufferPtr,
		  (void *)(spiComInst->ssiBase + SSI_O_DR),
		  bufferSize);

    UdmaStartTransfer(spiComInst->udmaChannelTx);

    return true;
}

void onDmaTransactionRxEnd(SpiCom spiComInst)
{
    if(isDMAtransactionOngoingRx(spiComInst))
        return;

    uint8_t* head = spiComInst->circBufferRx->head;
    uint8_t* circBuffBeg = spiComInst->circBufferRx->bufferStart;
    uint8_t* circBuffEnd = spiComInst->circBufferRx->bufferEnd;

    head += spiComInst->uDmaLastRxLen;

    if(head >= circBuffEnd)
        head = circBuffBeg;

    spiComInst->circBufferRx->head = head;

    startDmaTransactionRx(spiComInst);
}

void onDmaTransactionTxEnd(SpiCom spiComInst)
{
    if(isDMAtransactionOngoingTx(spiComInst))
        return;

    uint8_t* tail = spiComInst->circBufferTx->tail;
    uint8_t* circBuffBeg = spiComInst->circBufferTx->bufferStart;
    uint8_t* circBuffEnd = spiComInst->circBufferTx->bufferEnd;

    tail += spiComInst->uDmaLastTxLen;

    if(tail >= circBuffEnd)
        tail = circBuffBeg;

    spiComInst->circBufferTx->tail = tail;

    startDmaTransactionTx(spiComInst);
}


void SpiEmptyRxFifo(SpiCom spiComInst)
{
    uint32_t dummyData;
    while(SSIDataGetNonBlocking(spiComInst->ssiBase, &dummyData)) {} // empty RX FIFO
}


