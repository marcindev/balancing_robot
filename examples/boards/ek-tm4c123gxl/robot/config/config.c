#include "config.h"
#include "driverlib/eeprom.h"

#define CONFIG_EEPROM_BLOCK		1

#define BINARY1_ADDRESS_32_OFFSET		0
#define BINARY1_ADDRESS_32_TYPE			uint32_t
#define BINARY2_ADDRESS_32_OFFSET		4
#define BINARY2_ADDRESS_32_TYPE		    uint32_t
#define BINARY_FLAGS_8_OFFSET			8
#define BINARY_FLAGS_TYPE				uint8_t
#define BINARY1_VERSION_16_OFFSET		9
#define BINARY1_VERSION_TYPE				uint16_t
#define BINARY2_VERSION_16_OFFSET		11
#define BINARY2_VERSION_TYPE				uint16_t

#define BINARY1_ISACTIVE_MASK			0x1
#define BINARY1_ISCHECKED_MASK			0x2
#define BINARY1_ISGOOD_MASK				0x4
#define BINARY2_ISACTIVE_MASK			0x8
#define BINARY2_ISCHECKED_MASK			0x10
#define BINARY2_ISGOOD_MASK				0x20

static bool g_isConfigRead = false;

bool isConfigRead()
{
	return g_isConfigRead;
}


void readConfig()
{
	uint32_t blockSize = EEPROMSizeGet() / EEPROMBlockCountGet();
	uint32_t flashAddr = EEPROMAddrFromBlock(CONFIG_EEPROM_BLOCK);

	uint8_t confBuffer[blockSize];

	EEPROMRead(confBuffer, flashAddr, blockSize);


	config.binary1.address = *((BINARY1_ADDRESS_32_TYPE*)(confBuffer + BINARY1_ADDRESS_32_OFFSET));
	config.binary2.address = *((BINARY2_ADDRESS_32_TYPE*)(confBuffer + BINARY2_ADDRESS_32_OFFSET));
	config.binary1.version = *((BINARY1_VERSION_TYPE*)(confBuffer + BINARY1_VERSION_16_OFFSET));
	config.binary2.version = *((BINARY2_VERSION_TYPE*)(confBuffer + BINARY2_VERSION_16_OFFSET));


	uint8_t binaryFlags = *((BINARY_FLAGS_TYPE*)(confBuffer + BINARY_FLAGS_8_OFFSET));

	config.binary1.isActive = binaryFlags & BINARY1_ISACTIVE_MASK;
	config.binary1.isChecked = binaryFlags & BINARY1_ISCHECKED_MASK;
	config.binary1.isGood = binaryFlags & BINARY1_ISGOOD_MASK;
	config.binary2.isActive = binaryFlags & BINARY2_ISACTIVE_MASK;
	config.binary2.isChecked = binaryFlags & BINARY2_ISCHECKED_MASK;
	config.binary2.isGood = binaryFlags & BINARY2_ISGOOD_MASK;


	g_isConfigRead = true;
}

void writeConfig()
{
	uint32_t blockSize = EEPROMSizeGet() / EEPROMBlockCountGet();
	uint32_t flashAddr = EEPROMAddrFromBlock(CONFIG_EEPROM_BLOCK);

	uint8_t confBuffer[blockSize];

	*((BINARY1_ADDRESS_32_TYPE*)(confBuffer + BINARY1_ADDRESS_32_OFFSET)) = config.binary1.address;
	*((BINARY2_ADDRESS_32_TYPE*)(confBuffer + BINARY2_ADDRESS_32_OFFSET)) =	config.binary2.address;

	*((BINARY1_VERSION_TYPE*)(confBuffer + BINARY1_VERSION_16_OFFSET)) = config.binary1.version;
	*((BINARY2_VERSION_TYPE*)(confBuffer + BINARY2_VERSION_16_OFFSET)) = config.binary2.version;


	uint8_t binaryFlags = 0;

	if(config.binary1.isActive)
		binaryFlags |= BINARY1_ISACTIVE_MASK;

	if(config.binary1.isChecked)
		binaryFlags |= BINARY1_ISCHECKED_MASK;

	if(config.binary1.isGood)
		binaryFlags |= BINARY1_ISGOOD_MASK;

	if(config.binary2.isActive)
		binaryFlags |= BINARY2_ISACTIVE_MASK;

	if(config.binary2.isChecked)
		binaryFlags |= BINARY2_ISCHECKED_MASK;

	if(config.binary2.isGood)
		binaryFlags |= BINARY2_ISGOOD_MASK;

	*((BINARY_FLAGS_TYPE*)(confBuffer + BINARY_FLAGS_8_OFFSET)) = binaryFlags;

	EEPROMProgram(confBuffer, flashAddr, blockSize);
}


