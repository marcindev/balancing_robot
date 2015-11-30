/*	Wrapper for Spi driver
 * 	autor: Marcin Gozdziewski
 */
#include "spiWrapper.h"


void SpiComInit(SpiComInstance* spiComInst)
{
	if(spiComInst->initiated)
		return;

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
					   1000000,
					   8);
    SSIEnable(spiComInst->ssiBase);

    if(spiComInst->enableInt)
    	SSIIntEnable(spiComInst->ssiBase, SSI_RXFF);

    uint32_t dummyData;
    while(SSIDataGetNonBlocking(SSI0_BASE, &dummyData)) {} // empty FIFO

    spiComInst->initiated = true;
}

bool SpiComInitiated(SpiComInstance* spiComInst)
{
	return spiComInst->initiated;
}

bool SpiComSend(SpiComInstance* spiComInst, const uint8_t* data, uint32_t length)
{
	if((!length) || (!spiComInst->initiated))
		return false;

	uint32_t dataIndex = 0;

	while(dataIndex < length)
	{
		SSIDataPut(spiComInst->ssiBase, &data[dataIndex++]);
	}

	return true;
}

bool SpiComReceive(SpiComInstance* spiComInst, uint8_t* data, uint32_t length)
{
	if((!length) || (!spiComInst->initiated))
		return false;

	uint32_t dataIndex = 0;

	while(dataIndex < length)
	{
		SSIDataGet(spiComInst->ssiBase, &data[dataIndex++]);
	}

	return true;
}

