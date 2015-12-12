/*	Wrapper for Spi driver
 * 	autor: Marcin Gozdziewski
 */
#include "spiWrapper.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"

#define START_FRAME 0xFF

static void SpiEmptyRxFifo(SpiComInstance* spiComInst);
static bool isDMAtransactionOngoingRx(SpiComInstance* spiComInst);
static bool isDMAtransactionOngoingTx(SpiComInstance* spiComInst);


// The control table used by the uDMA controller.  This table must be aligned
// to a 1024 byte boundary.
#if defined(ewarm)
#pragma data_alignment=1024
uint8_t ui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(ui8ControlTable, 1024)
uint8_t ui8ControlTable[1024];
#else
uint8_t ui8ControlTable[256] __attribute__ ((aligned(128)));
#endif


void SpiComInit(SpiComInstance* spiComInst)
{
	if(spiComInst->initiated)
		return;

	spiComInst->uDmaLastTxLen = 0;
	spiComInst->uDmaLastRxLen = 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	IntEnable(INT_UDMAERR);
    uDMAEnable();
    uDMAControlBaseSet(ui8ControlTable);

	SysCtlPeripheralEnable(spiComInst->spiPeripheral);
	SysCtlPeripheralEnable(spiComInst->gpioPeripheral);
    GPIOPinConfigure(spiComInst->clkPinConfig);
    GPIOPinConfigure(spiComInst->fssPinConfig);
    GPIOPinConfigure(spiComInst->rxPinConfig);
    GPIOPinConfigure(spiComInst->txPinConfig);
    GPIOPinTypeSSI(spiComInst->gpioPortBase, spiComInst->txPin | spiComInst->rxPin |
    			   spiComInst->fssPin | spiComInst->clkPin);

    SSIConfigSetExpClk(spiComInst->ssiBase,
    				   SysCtlClockGet(),
					   SSI_FRF_MOTO_MODE_0,
                       spiComInst->masterSlave == SPI_MASTER ? SSI_MODE_MASTER : SSI_MODE_SLAVE,
					   400000,
					   8);

    SSIEnable(spiComInst->ssiBase);

    SSIDMAEnable(spiComInst->ssiBase, SSI_DMA_RX | SSI_DMA_TX);
    IntEnable(INT_SSI0);

    uDMAChannelAttributeDisable(spiComInst->uDmaChannelRx,
                                UDMA_ATTR_USEBURST |
                                UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_ALTSELECT |
                                UDMA_ATTR_REQMASK);



    uDMAChannelAttributeEnable(spiComInst->uDmaChannelRx,UDMA_ATTR_USEBURST);

    uDMAChannelAttributeDisable(spiComInst->uDmaChannelTx,
                                UDMA_ATTR_USEBURST |
                                UDMA_ATTR_HIGH_PRIORITY |
                                UDMA_ATTR_ALTSELECT |
                                UDMA_ATTR_REQMASK);


    uDMAChannelAttributeEnable(spiComInst->uDmaChannelTx,UDMA_ATTR_USEBURST);

//    if(spiComInst->enableInt)
//    	SSIIntEnable(spiComInst->ssiBase, SSI_DMATX | SSI_DMARX);




    uDMAChannelControlSet(spiComInst->uDmaChannelRx | UDMA_PRI_SELECT,
                                   UDMA_SIZE_8 | UDMA_SRC_INC_NONE |
                                   UDMA_DST_INC_8 | UDMA_ARB_4);

    uDMAChannelControlSet(spiComInst->uDmaChannelTx | UDMA_PRI_SELECT,
                                   UDMA_SIZE_8 | UDMA_SRC_INC_8 |
                                   UDMA_DST_INC_NONE | UDMA_ARB_4);


    // set tx signal pin
//	SysCtlPeripheralEnable(spiComInst->sigTxGpioPeripheral);
//	GPIOPinTypeGPIOOutput(spiComInst->sigTxPortBase, spiComInst->sigTxPin);
//	GPIOPinWrite(spiComInst->sigTxPortBase, spiComInst->sigTxPin, 0);

	SpiEmptyRxFifo(spiComInst);

    spiComInst->initiated = true;

	if(!isDMAtransactionOngoingRx(spiComInst))
		startDmaTransactionRx(spiComInst);
}

bool SpiComInitiated(SpiComInstance* spiComInst)
{
	return spiComInst->initiated;
}

bool SpiComSend(SpiComInstance* spiComInst, const uint8_t* data, uint32_t length)
{
	if((!length) || (!spiComInst->initiated))
		return false;

	if(CB_isFull(spiComInst->circBufferTx))
		return false;

	if(!CB_pushData(spiComInst->circBufferTx, data, length))
		return false;

	if(!isDMAtransactionOngoingTx(spiComInst))
		startDmaTransactionTx(spiComInst);

	return true;
}

bool SpiComReceive(SpiComInstance* spiComInst, uint8_t* data, uint32_t length)
{
	if((!length) || (!spiComInst->initiated))
		return false;

	if(CB_isEmpty(spiComInst->circBufferRx))
		return false;

	if(!CB_popData(spiComInst->circBufferRx, data, length))
		return false;

	if(!isDMAtransactionOngoingRx(spiComInst))
		startDmaTransactionRx(spiComInst);

	return true;
}

bool isDMAtransactionOngoingRx(SpiComInstance* spiComInst)
{
	return uDMAChannelIsEnabled(spiComInst->uDmaChannelRx);
}

bool isDMAtransactionOngoingTx(SpiComInstance* spiComInst)
{
    return uDMAChannelIsEnabled(spiComInst->uDmaChannelTx);
}

bool startDmaTransactionRx(SpiComInstance* spiComInst)
{

	if(CB_isFull(spiComInst->circBufferRx))
		return false;

	uint8_t* head = spiComInst->circBufferRx->head;
	uint8_t* tail = spiComInst->circBufferRx->tail;
	uint8_t* circBuffBeg = spiComInst->circBufferRx->bufferStart;
	uint8_t* circBuffEnd = spiComInst->circBufferRx->bufferEnd;

    uint8_t* bufferPtr = 0;
    uint32_t bufferSize = 0;

    if(head >= tail)
    {
    	bufferPtr = head;

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

    uDMAChannelTransferSet(spiComInst->uDmaChannelRx | UDMA_PRI_SELECT,
                            UDMA_MODE_BASIC,
                            (void *)(spiComInst->ssiBase + SSI_O_DR),
							bufferPtr,
							bufferSize);

    uDMAChannelEnable(spiComInst->uDmaChannelRx);

    return true;
}


bool startDmaTransactionTx(SpiComInstance* spiComInst)
{


	if(CB_isEmpty(spiComInst->circBufferTx))
		return false;

	uint8_t* head = spiComInst->circBufferTx->head;
	uint8_t* tail = spiComInst->circBufferTx->tail;
	uint8_t* circBuffBeg = spiComInst->circBufferTx->bufferStart;
	uint8_t* circBuffEnd = spiComInst->circBufferTx->bufferEnd;

    uint8_t* bufferPtr = 0;
    uint32_t bufferSize = 0;

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

    uDMAChannelTransferSet(spiComInst->uDmaChannelTx | UDMA_PRI_SELECT,
                            UDMA_MODE_BASIC,
							bufferPtr,
							(void *)(spiComInst->ssiBase + SSI_O_DR),
							bufferSize);

    uDMAChannelEnable(spiComInst->uDmaChannelTx);

    return true;
}

void onDmaTransactionRxEnd(SpiComInstance* spiComInst)
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

void onDmaTransactionTxEnd(SpiComInstance* spiComInst)
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


void SpiEmptyRxFifo(SpiComInstance* spiComInst)
{
    uint32_t dummyData;
    while(SSIDataGetNonBlocking(spiComInst->ssiBase, &dummyData)) {} // empty RX FIFO
}

/*
bool SpiComSend(SpiComInstance* spiComInst, const uint8_t* data, uint32_t length)
{
	if((!length) || (!spiComInst->initiated))
		return false;

//	GPIOPinWrite(spiComInst->sigTxPortBase, spiComInst->sigTxPin, spiComInst->sigTxPin);
	uint32_t dataIndex = 0;

	while(dataIndex < length)
	{
		SSIDataPut(spiComInst->ssiBase, data[dataIndex++]);
		SpiEmptyRxFifo(spiComInst);
	}

//	GPIOPinWrite(spiComInst->sigTxPortBase, spiComInst->sigTxPin, 0);

	while(SSIBusy(spiComInst->ssiBase)){}

	SpiEmptyRxFifo(spiComInst);

	return true;
}

bool SpiComReceive(SpiComInstance* spiComInst, uint8_t* data, uint32_t length)
{
	if((!length) || (!spiComInst->initiated))
		return false;

	uint32_t dataIndex = 0;

	if(spiComInst->masterSlave)
		SpiEmptyRxFifo(spiComInst);

	while(dataIndex < length)
	{
		uint32_t word = 66;

		SSIDataPut(spiComInst->ssiBase, word);

		while(SSIBusy(spiComInst->ssiBase)){}

		SSIDataGet(spiComInst->ssiBase, &word);
		data[dataIndex++] = 0xFF & word;
	}
	SpiEmptyRxFifo(spiComInst);

	return true;
}


*/
