/*	Wrapper for I2C driver
 * 	autor: Marcin Gozdziewski
 */
#ifndef I2CWRAPPER_H
#define I2CWRAPPER_H

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

#define I2C_SPEED_400	1
#define I2C_SPEED_100 	0

typedef struct
{
	uint32_t i2cPeripheral,
			 gpioPeripheral,
			 sclPinConfig,
			 sdaPinConfig,
			 gpioPortBase,
			 sclPin,
			 sdaPin,
			 i2cBase,
			 slaveAddress;
	uint8_t speed;
	bool isMaster;
	bool initiated;
	uint32_t errState;
} I2CComInstance;

void I2CComInit(I2CComInstance* i2cComInst);
bool I2CComInitiated(I2CComInstance* i2cComInst);
bool I2CComSend(I2CComInstance* i2cComInst, uint8_t slaveAddress, const uint8_t* data, uint32_t length);
bool I2CComReceive(I2CComInstance* i2cComInst, uint8_t slaveAddress, uint8_t* data, uint32_t length);



#endif // I2CWRAPPER_H
