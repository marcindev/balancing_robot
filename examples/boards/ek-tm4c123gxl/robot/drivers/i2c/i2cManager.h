//********************************************
// Interface for communication with i2c driver
//
//********************************************


#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

typedef struct
{
	// Do not modify
	uint8_t taskId;
}I2cManager;

bool initI2cManager(I2cManager* i2cMng);
bool i2cSend(I2cManager* i2cMng, uint8_t slaveAddress, const uint8_t* data, uint32_t length);
bool i2cReceive(I2cManager* i2cMng, uint8_t slaveAddress, uint8_t* data, uint32_t length);
bool i2cSendAndReceive(I2cManager* i2cMng,
					uint8_t slaveAddress,
					uint8_t* sentData,
					uint32_t sentLength,
					uint8_t* recvData,
					uint32_t recvLength
					);


#endif // I2C_MANAGER_H
