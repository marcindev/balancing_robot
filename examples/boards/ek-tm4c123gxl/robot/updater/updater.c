#include "updater.h"
#include "config.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "driverlib/eeprom.h"
#include "driverlib/flash.h"
#include "driverlib/sysctl.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "logger.h"

#define FLASH_SIZE			0x00040000
#define BLOCK_SIZE			1024
#define WORD_SIZE			4
#define _128_BYTES			128
#define BLOCK_PARTS_NUM		8

#define CRC_DIVISOR			0x09

#define UPD_CHECK_TIMER_PERIOD 		    15000


// commands

#define START_UPDATE_UPD_CMD			0x01
#define ERASE_MEMORY_UPD_CMD			0x02
#define SEND_DATA_UPD_CMD				0x03
#define FINISH_DATA_TRANSFER_UPD_CMD	0x04
#define FREE_RESOURCES_UPD_CMD			0x05
#define RESET_ROBOT_UPD_CMD				0x06
#define AFTER_UPD_CHECK_UPD_CMD			0x07
#define GET_AVAIL_PARTITION_UPD_CMD	    0x08
#define MARK_PARTITION_AS_GOOD_UPD_CMD	0x09

// statuses

#define UPDATER_NOT_INIT_UPD_STAT		0x01
#define NOT_ENOUGH_SPACE_UPD_STAT		0x02
#define READY_FOR_UPDATE_UPD_STAT		0x03
#define ERASE_OK_UPD_STAT				0x04
#define ERASE_ERR_UPD_UPD_STAT			0x05
#define PROGRAM_DATA_OK_UPD_STAT		0x06
#define PROGRAM_DATA_ERR_UPD_STAT		0x07
#define DATA_CHECKSUM_NOK_UPD_STAT		0x08
#define MISSING_PACKET_UPD_STAT			0x09
#define FINISH_SUCCESS_UPD_STAT			0x0A
#define BINARY_CRC_ERR_UPD_STAT			0x0B
#define FREE_RESOURCES_OK_UPD_STAT		0x0C
#define UNRECOGNIZED_CMD_UPD_STAT		0x0D
#define	AVAIL_PARTITION_1_CMD_UPD_STAT	0x0E
#define AVAIL_PARTITION_2_CMD_UPD_STAT	0x0F
#define AFTER_UPD_CHECK_OK_UPD_STAT		0x10
#define MARK_PARTITION_AS_GOOD_UPD_STAT 0x11

static uint32_t g_partitionAddress;
static uint32_t g_partitionSize;
static uint32_t g_nextAddress;
static uint32_t g_nextPartNum = 0;

static uint32_t g_binarySize;
static bool g_updaterInitialized;

static uint32_t* g_CRC32Table;
static uint32_t g_receivedCrc;

static uint8_t* g_flashBlockBuffer;

static TimerHandle_t g_updateCheckTimer;

static bool erasePartition();
static bool programNextWord(uint32_t word);
static bool programNextBlock(uint32_t* data);
static uint8_t calcChecksum(uint8_t* data, uint32_t size);
static uint32_t calculateCRC32(uint8_t *data, uint32_t size, uint32_t ui32CRC);
static void initCRC32Table();
static void initUpdateCheckTimer();
static void updateCheckTimerCallback(TimerHandle_t pxTimer);

static uint8_t handleStartUpdate(uint32_t binarySize, uint32_t crc, uint32_t checksum);
static uint8_t handleEraseMemory();
static uint8_t handleSendData(uint32_t data, uint32_t checksum, uint32_t partNum);
static uint8_t handleFinishDataTransfer();
static uint8_t handleFreeResources();
static uint8_t handleResetRobot();
static uint8_t handleAfterUpdateCheck();
static uint8_t handleMarkPartitionAsGood();
static uint8_t handleGetAvailPartition();

void initUpdater()
{
	if(!isConfigRead())
		readConfig();

	uint32_t address1 = config.binary1.address;
	uint32_t address2 = config.binary2.address;
	bool* isActive1 = &config.binary1.isActive;
	bool* isActive2 = &config.binary2.isActive;
	bool* isChecked1 = &config.binary1.isChecked;
	bool* isChecked2 = &config.binary2.isChecked;
	bool* isGood1 = &config.binary1.isGood;
	bool* isGood2 = &config.binary2.isGood;
	uint16_t version1 = config.binary1.version;
	uint16_t version2 = config.binary2.version;

	if(*isActive1)
	{
		g_partitionAddress = address2;
		g_partitionSize = FLASH_SIZE - address2;

		if(version2 > version1)
		{
			if(!(*isChecked2))
			{
				initUpdateCheckTimer();
			}
		}
//		else
//		{
//			*isChecked2 = false;
//		}
	}
	else
	{
		g_partitionAddress = address1;
		g_partitionSize = address2 - address1;

		if(version1 > version2)
		{
			if(!(*isChecked1))
			{
				initUpdateCheckTimer();
			}
		}
//		else
//		{
//			*isChecked1 = false;
//		}
	}

	g_nextAddress = g_partitionAddress;

	g_updaterInitialized = true;

	logger(Info, Log_Updater, "[initUpdater] updater initialized");
	logger(Info, Log_Updater, "[initUpdater] Partition1: isActive=%d, version=%d, isChecked=%d, isGood=%d",
			*isActive1, version1, *isChecked1, *isGood1);
	logger(Info, Log_Updater, "[initUpdater] Partition2: isActive=%d, version=%d, isChecked=%d, isGood=%d",
			*isActive2, version2, *isChecked2, *isGood2);
	logger(Info, Log_Updater, "[initUpdater] current partition address: %#.8X",HWREG(NVIC_VTABLE));

}

uint8_t handleUpdateCommand(uint8_t command, uint32_t data1, uint32_t data2, uint32_t data3)
{
	switch(command)
	{
	case START_UPDATE_UPD_CMD:
	{
		return handleStartUpdate(data1, data2, data3);
	}
	case ERASE_MEMORY_UPD_CMD:
	{
		return handleEraseMemory();
	}
	case SEND_DATA_UPD_CMD:
	{
		return handleSendData(data1, data2, data3);
	}
	case FINISH_DATA_TRANSFER_UPD_CMD:
	{
		return handleFinishDataTransfer();
	}
	case FREE_RESOURCES_UPD_CMD:
	{
		return handleFreeResources();
	}
	case RESET_ROBOT_UPD_CMD:
	{
		return handleResetRobot();
	}
	case AFTER_UPD_CHECK_UPD_CMD:
	{
		return handleAfterUpdateCheck();
	}
	case MARK_PARTITION_AS_GOOD_UPD_CMD:
	{
		return handleMarkPartitionAsGood();
	}
	case GET_AVAIL_PARTITION_UPD_CMD:
	{
		return handleGetAvailPartition();
	}
	default:
		logger(Warning, Log_Updater, "[handleUpdateCommand] received unrecognized command %d", command);
		return UNRECOGNIZED_CMD_UPD_STAT;
	}
}


uint8_t handleStartUpdate(uint32_t binarySize, uint32_t crc, uint32_t checksum)
{
	logger(Info, Log_Updater, "[handleStartUpdate] binary size: %d", binarySize);

	if(!g_updaterInitialized)
	{
		logger(Error, Log_Updater, "[handleStartUpdate] updater not initialized");
		return UPDATER_NOT_INIT_UPD_STAT;
	}

	uint32_t data[2] = {binarySize, crc};

	if(calcChecksum((uint8_t*)&data, sizeof(data)) != (checksum & 0xFF))
	{
		logger(Error, Log_Updater, "[handleStartUpdate] checksum doesn't match");
		return DATA_CHECKSUM_NOK_UPD_STAT;
	}

	if(binarySize > g_partitionSize)
	{
		logger(Error, Log_Updater, "[handleStartUpdate] not enough space on current partition;"
				"binary: %d, partition: %d", binarySize, g_partitionSize);

		return NOT_ENOUGH_SPACE_UPD_STAT;
	}

	g_CRC32Table = (uint32_t*) pvPortMalloc(256 * sizeof(uint32_t));

	g_flashBlockBuffer = (uint8_t*) pvPortMalloc(BLOCK_SIZE * sizeof(uint8_t));

	initCRC32Table();

	g_binarySize = binarySize;
	g_receivedCrc = crc;

	return READY_FOR_UPDATE_UPD_STAT;
}

uint8_t handleEraseMemory()
{
	logger(Info, Log_Updater, "[handleEraseMemory] erasing partition");

	if(!erasePartition())
	{
		logger(Error, Log_Updater, "[handleEraseMemory] error while erasing");
		return ERASE_ERR_UPD_UPD_STAT;
	}

	return ERASE_OK_UPD_STAT;
}


uint8_t handleSendData(uint32_t data, uint32_t checksum, uint32_t partNum)
{
	if(partNum != g_nextPartNum)
	{
		logger(Error, Log_Updater, "[handleSendData] missing packet");
		return MISSING_PACKET_UPD_STAT;
	}

	if(calcChecksum(&data, WORD_SIZE) != (checksum & 0xFF))
	{
		logger(Error, Log_Updater, "[handleSendData] checksum doesn't match");
		return DATA_CHECKSUM_NOK_UPD_STAT;
	}

	if(!programNextWord(data))
	{
		logger(Error, Log_Updater, "[handleSendData] error while writing to flash");
		return PROGRAM_DATA_ERR_UPD_STAT;
	}

	++g_nextPartNum;

	return PROGRAM_DATA_OK_UPD_STAT;
}

uint8_t handleSendDataBlock(uint8_t* data, uint8_t checksum, uint32_t partNum)
{
	if(partNum != g_nextPartNum)
	{
		logger(Error, Log_Updater, "[handleSendData] missing packet");
		return MISSING_PACKET_UPD_STAT;
	}

	if(calcChecksum(data, _128_BYTES) != checksum)
	{
		logger(Error, Log_Updater, "[handleSendData] checksum doesn't match");
		return DATA_CHECKSUM_NOK_UPD_STAT;
	}

	uint8_t* nextBufferAddr = g_flashBlockBuffer + (g_nextPartNum % BLOCK_PARTS_NUM) * _128_BYTES;

	memcpy(nextBufferAddr, data, _128_BYTES);

	if(!((g_nextPartNum + 1) % BLOCK_PARTS_NUM))
	{
		if(!programNextBlock(g_flashBlockBuffer))
		{
			logger(Error, Log_Updater, "[handleSendData] error while writing to flash");
			return PROGRAM_DATA_ERR_UPD_STAT;
		}
	}

	++g_nextPartNum;

	return PROGRAM_DATA_OK_UPD_STAT;
}


uint8_t handleFinishDataTransfer()
{
	logger(Info, Log_Updater, "[handleFinishDataTransfer] calculating crc");
	uint32_t crc = calculateCRC32((uint8_t*)g_partitionAddress, g_binarySize, CRC_DIVISOR);

	if(crc != g_receivedCrc)
	{
		logger(Error, Log_Updater, "[handleFinishDataTransfer] crc doesn't match");
		return BINARY_CRC_ERR_UPD_STAT;
	}

	logger(Info, Log_Updater, "[handleFinishDataTransfer] CRC OK");

	uint16_t currBinVersion = 0;

	if(config.binary1.version > config.binary2.version)
	{
		currBinVersion = config.binary1.version + 1;
	}
	else
	{
		currBinVersion = config.binary2.version + 1;
	}

	if(g_partitionAddress == config.binary1.address)
	{
		config.binary1.version = currBinVersion;
		config.binary1.isChecked = false;
	}
	else
	{
		config.binary2.version = currBinVersion;
		config.binary2.isChecked = false;
	}

	writeConfig();

	logger(Info, Log_Updater, "[handleFinishDataTransfer] Current binary version: %d", currBinVersion);

	return FINISH_SUCCESS_UPD_STAT;
}

uint8_t handleFreeResources()
{
	logger(Info, Log_Updater, "[handleFreeResources] freeing allocated memory");
	vPortFree(g_CRC32Table);
	g_CRC32Table = 0;

	return FREE_RESOURCES_OK_UPD_STAT;
}

uint8_t handleResetRobot()
{
	logger(Info, Log_Updater, "[handleResetRobot] dumping postmortem and reseting robot");

	dumpPostMortem();
	SysCtlReset();

	return 0;
}

uint8_t handleAfterUpdateCheck()
{
	logger(Info, Log_Updater, "[handleAfterUpdateCheck] checking update validity");

	if( xTimerStop( g_updateCheckTimer, 0 ) != pdPASS )
	{
		logger(Error, Log_Updater, "[handleAfterUpdateCheck] Couldn't stop timer");
	}

	if(g_partitionAddress == config.binary1.address)
	{
		config.binary1.isChecked = true;
		config.binary1.isGood = false;
	}
	else
	{
		config.binary2.isChecked = true;
		config.binary2.isGood = false;
	}

	writeConfig();

	return AFTER_UPD_CHECK_OK_UPD_STAT;
}

uint8_t handleMarkPartitionAsGood()
{
	logger(Info, Log_Updater, "[handleMarkPartitionAsGood] marking partition as good");

	if(g_partitionAddress == config.binary1.address)
	{
		config.binary1.isGood = true;
	}
	else
	{
		config.binary2.isGood = true;
	}

	writeConfig();

	return MARK_PARTITION_AS_GOOD_UPD_STAT;
}

uint8_t handleGetAvailPartition()
{
	logger(Info, Log_Updater, "[handleGetAvailPartition] checking for inactive partition");

	if(!config.binary1.isActive)
		return AVAIL_PARTITION_1_CMD_UPD_STAT;

	return AVAIL_PARTITION_2_CMD_UPD_STAT;
}

uint32_t getPartitionSize()
{
	return g_partitionSize;
}

bool erasePartition()
{
	uint32_t currAddress = g_partitionAddress;
	uint32_t leftSize = g_partitionSize;

	while(leftSize >= BLOCK_SIZE)
	{
		if(FlashErase(currAddress))
			return false;

		currAddress += BLOCK_SIZE;
		leftSize -= BLOCK_SIZE;
	}

	return true;
}

bool programNextWord(uint32_t word)
{
	if(FlashProgram(&word, g_nextAddress, WORD_SIZE))
		return false;

	g_nextAddress += WORD_SIZE;

	return true;
}

bool programNextBlock(uint32_t* data)
{
	if(FlashProgram(data, g_nextAddress, BLOCK_SIZE))
		return false;

	g_nextAddress += BLOCK_SIZE;

	return true;
}

uint8_t calcChecksum(uint8_t* data, uint32_t size)
{
	uint32_t checksum = 0;

	while(size--)
	{
		checksum += *data++;
	}

	return checksum & 0xFF;
}



uint32_t reflect(uint32_t ui32Ref, uint8_t ui8Ch)
{
    uint_fast32_t ui32Value;
    int_fast16_t i16Loop;

    ui32Value = 0;

    for(i16Loop = 1; i16Loop < (ui8Ch + 1); i16Loop++)
    {
        if(ui32Ref & 1)
        {
            ui32Value |= 1 << (ui8Ch - i16Loop);
        }
        ui32Ref >>= 1;
    }

    return ui32Value;
}

void initCRC32Table()
{
    uint_fast32_t ui32Polynomial;
    int_fast16_t i16Loop, i16Bit;

    //
    // This is the ANSI X 3.66 polynomial as required by the DFU
    // specification.
    //
    ui32Polynomial = 0x04c11db7;

    for(i16Loop = 0; i16Loop <= 0xFF; i16Loop++)
    {
    	g_CRC32Table[i16Loop]=reflect(i16Loop, 8) << 24;
          for (i16Bit = 0; i16Bit < 8; i16Bit++)
          {
        	  g_CRC32Table[i16Loop] = ((g_CRC32Table[i16Loop] << 1) ^
                                            (g_CRC32Table[i16Loop] &
                                             ((uint32_t)1 << 31) ?
                                             ui32Polynomial : 0));
          }
          g_CRC32Table[i16Loop] = reflect(g_CRC32Table[i16Loop], 32);
    }
}


uint32_t calculateCRC32(uint8_t *data, uint32_t size, uint32_t ui32CRC)
{
    uint32_t ui32Count;
    uint8_t *pui8Buffer;
    uint8_t ui8Char;

    pui8Buffer = data;
    ui32Count = size;

    while(ui32Count--)
    {
        ui8Char = *pui8Buffer++;
        ui32CRC = (ui32CRC >> 8) ^ g_CRC32Table[(ui32CRC & 0xFF) ^
                  ui8Char];
    }

    return ui32CRC;
}

void initUpdateCheckTimer()
{
	if(g_updateCheckTimer)
		return;

	g_updateCheckTimer = xTimerCreate(
			"UpdateCheckTimer",
			pdMS_TO_TICKS(UPD_CHECK_TIMER_PERIOD),
			pdTRUE,
			( void * ) 0,
			updateCheckTimerCallback
	);

	if(g_updateCheckTimer == NULL)
	{
		logger(Error, Log_Updater, "[initUpdateCheckTimer] Couldn't create timer");
		return;
	}

	logger(Info, Log_Updater, "[initUpdateCheckTimer] timer created");

	if( xTimerStart( g_updateCheckTimer, 0 ) != pdPASS )
	{
		logger(Error, Log_Updater, "[initUpdateCheckTimer] Couldn't start timer");
	}

	logger(Debug, Log_Updater, "[initUpdateCheckTimer] timer started");
}

void updateCheckTimerCallback(TimerHandle_t pxTimer)
{
	logger(Error, Log_Updater, "[updateCheckTimerCallback] timer expired; marking partition as not good");


	if(g_partitionAddress == config.binary1.address)
	{
		config.binary1.isChecked = true;
		config.binary1.isGood = false;
	}
	else
	{
		config.binary2.isChecked = true;
		config.binary2.isGood = false;
	}

	writeConfig();

	dumpPostMortem();


	if( xTimerStop( pxTimer, 0 ) != pdPASS )
	{
		logger(Error, Log_Updater, "[handleAfterUpdateCheck] Couldn't stop timer");
	}
}

