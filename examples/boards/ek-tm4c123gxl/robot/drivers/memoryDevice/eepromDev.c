#include "eepromDev.h"
#include "driverlib/eeprom.h"

#define SMALL_BLOCK_SIZE     4

static int32_t eepromInit()
{
    if(EEPROMInit() == EEPROM_INIT_OK)
	return 0;

    return -1;
   
}

static int32_t eepromGetParams(MemDevParams* params)
{
    
    params->begAddr = EEPROMAddrFromBlock(0);
    params->endAddr = params->begAddr + EEPROMSizeGet();
    params->smallBlockSize = SMALL_BLOCK_SIZE;
    params->largeBlockSize = EEPROMSizeGet() / EEPROMBlockCountGet();
    params->eraseBlockSize = 0;

    return 0;
}

static int32_t eepromRead(MemAddress addr, MemData* data, MemSize size)
{
    EEPROMRead(data, addr, size);

    return 0;
}

static int32_t eepromWrite(MemAddress addr, MemData* data, MemSize size)
{
    if(EEPROMProgram(data, addr, size) != 0)
	return -1;

    return 0;
}


static MemoryDevice createEepromDev()
{
    MemoryDevice eeprom = MemDevCreate();

    MemDevRegisterInitFunc(eeprom, eepromInit);
    MemDevRegisterGetParamsFunc(eeprom, eepromGetParams);
    MemDevRegisterReadFunc(eeprom, eepromRead);
    MemDevRegisterWriteFunc(eeprom, eepromWrite);

    return eeprom;
}


MemoryDevice getEepromInstance()
{
    static MemoryDevice eeprom = 0;

    if(!eeprom)
	eeprom = createEepromDev();
   
    return eeprom;
}

