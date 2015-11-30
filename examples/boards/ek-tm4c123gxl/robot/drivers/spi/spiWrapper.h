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

#define SPI_MASTER 1
#define SPI_SLAVE  0

typedef struct
{
	uint32_t spiPeripheral,
			 gpioPeripheral,
			 clkPinConfig,
			 fssPinConfig,
			 rxPinConfig,
			 txPinConfig,
			 gpioPortBase,
			 ssiBase,
			 masterSlave,
			 clkPin,
			 fssPin,
			 rxPin,
			 txPin;
	bool enableInt;
	bool initiated;
	uint32_t errState;
} SpiComInstance;

void SpiComInit(SpiComInstance* i2cComInst);
bool SpiComInitiated(SpiComInstance* i2cComInst);
bool SpiComSend(SpiComInstance* i2cComInst, const uint8_t* data, uint32_t length);
bool SpiComReceive(SpiComInstance* i2cComInst, uint8_t* data, uint32_t length);



#endif // SPIWRAPPER_H
