
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "logger.h"
#include <stdarg.h>
#include "driverlib/eeprom.h"

#define BUFFER_SIZE			30

#define INT_ARG				1
#define DOUBLE_ARG			2

#define MAX_ARGS_NUM		10

#define PM_ELEM_BEG			0xF1
#define PM_STOP_MARK		0xBF
#define PM_LEN_OFFSET		1
#define PM_BEG_BLOCK		2

typedef struct
{
	uint32_t timestampMilis;
	LogLevel logLevel;
	LogComponent component;
	void* stringPtr;
	uint8_t argsNum;
	uint8_t* argTypes;
	uint8_t* argsBuffer;
	uint8_t argsBuffSize;
}LogElem;

static LogElem g_buffer[BUFFER_SIZE];
static uint16_t index = 0;
static bool isRollOver = false;

void logger(LogLevel level, LogComponent component, const char* string, ...)
{
	va_list arguments;

	if(index == BUFFER_SIZE)
	{
		index = 0;
		isRollOver = true;
	}
	g_buffer[index].timestampMilis = xTaskGetTickCount() / portTICK_RATE_MS;
	g_buffer[index].logLevel = level;
	g_buffer[index].component = component;
	g_buffer[index].stringPtr = string;

	uint32_t i = 0;
	uint8_t argsNum = 0;
	uint8_t argTypes[MAX_ARGS_NUM] = {0};
	bool isInArg = false;

	while(string[i] != 0)
	{
		if((i > 0) && !isInArg && (string[i] == '%') && (string[i - 1] != '%'))
		{
			argsNum++;
			isInArg = true;
		}
		else if(isInArg)
		{
			switch(string[i])
			{
			case 'd':
			case 'i':
			case 'u':
			case 'o':
			case 'x':
			case 'X':
			case 'c':
			case 'p':
			case 'n':
				isInArg = false;
				argTypes[argsNum - 1] = INT_ARG;
				break;

			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
			case 'a':
			case 'A':
				isInArg = false;
				argTypes[argsNum - 1] = DOUBLE_ARG;
				break;

			default:
				break;
			}
		}

		++i;
	}

	g_buffer[index].argsNum = argsNum;


	if(argsNum > 0)
	{
		uint32_t argsBuffSize = 0;
		uint8_t* argTypesPtr = (uint8_t*) pvPortMalloc(argsNum);

		for(int j = 0; j != argsNum; ++j)
		{
			argTypesPtr[j] = argTypes[j];

			if(argTypes[j] == DOUBLE_ARG)
				argsBuffSize += sizeof(double);
			else
				argsBuffSize += sizeof(uint32_t);

		}
		g_buffer[index].argsBuffSize = argsBuffSize;

		uint8_t* argsBuff = (uint8_t*) pvPortMalloc(argsBuffSize);
		va_start(arguments, argsNum);
		uint8_t* argsBuffPtr = argsBuff;

		for(int j = 0; j != argsNum; ++j)
		{
			uint8_t* bytePtr = 0;
			uint8_t argSize = 0;

			if(argTypes[j] == DOUBLE_ARG)
			{
				double arg = (double) va_arg(arguments, double);
				bytePtr = (uint8_t*) &arg;
				argSize = sizeof(double);

				for(int k = 0; k != argSize; ++k)
				{
					*argsBuffPtr = *bytePtr;
					++argsBuffPtr;
					++bytePtr;
				}
			}
			else
			{
				uint32_t arg = (uint32_t) va_arg(arguments, uint32_t);
				bytePtr = (uint8_t*) &arg;
				argSize = sizeof(uint32_t);

				for(int k = 0; k != argSize; ++k)
				{
					*argsBuffPtr = *bytePtr;
					++argsBuffPtr;
					++bytePtr;
				}
			}


		}

		va_end ( arguments );

		if(isRollOver)
		{
			if(g_buffer[index].argsBuffer)
				vPortFree(g_buffer[index].argsBuffer);

			if(g_buffer[index].argTypes)
				vPortFree(g_buffer[index].argTypes);
		}

		g_buffer[index].argsBuffer = argsBuff;
		g_buffer[index].argTypes = argTypesPtr;
	}
	else
	{
		if(g_buffer[index].argsBuffer)
			vPortFree(g_buffer[index].argsBuffer);

		if(g_buffer[index].argTypes)
			vPortFree(g_buffer[index].argTypes);

		g_buffer[index].argsBuffer = 0;
		g_buffer[index].argTypes = 0;
	}


	index++;
}

bool getNextLogLine(uint32_t* timestamp, LogLevel* logLevel, LogComponent* component,
					void** strPtr, uint8_t* argsNum, void** argTypes, void** argsBufferPtr,
					uint8_t* argsBuffSize)
{
	if((index == 0) && (!isRollOver))
		return false;

	static isStart = true;
	static uint16_t nextInd = 0;
	static totalLinesNum = 0;
	static lineNum = 0;

	if(isStart)
	{
		nextInd = (isRollOver ? index : 0);
		totalLinesNum = getLinesNumber();
		lineNum = 1;
	}

	if(nextInd == BUFFER_SIZE)
		nextInd = 0;

	*timestamp = g_buffer[nextInd].timestampMilis;
	*logLevel = g_buffer[nextInd].logLevel;
	*component = g_buffer[nextInd].component;
	*strPtr = g_buffer[nextInd].stringPtr;
	*argsNum = g_buffer[nextInd].argsNum;
	*argTypes = g_buffer[nextInd].argTypes;
	*argsBufferPtr = g_buffer[nextInd].argsBuffer;
	*argsBuffSize = g_buffer[nextInd].argsBuffSize;

	if(!isStart && ((nextInd == index) || (lineNum > totalLinesNum)))
	{
		isStart = true;
		return false;
	}
	else
	{
		isStart = false;
	}

	nextInd++;
	lineNum++;

	return true;
}

uint16_t getLinesNumber()
{
	return (isRollOver ?  BUFFER_SIZE : index);
}

bool dumpPostMortem()
{
	const uint16_t MAX_ELEM_BUFF_SIZE = 30;
	const uint16_t WORD_SIZE = sizeof(uint32_t);
	uint32_t pmBuffer[MAX_ELEM_BUFF_SIZE];

	memset(pmBuffer, 0, MAX_ELEM_BUFF_SIZE * WORD_SIZE);

	int nextInd = index - 1;
	uint32_t stopWord = 0;
	*((uint8_t*)&stopWord) = PM_STOP_MARK;

	uint32_t blockSize = EEPROMSizeGet() / EEPROMBlockCountGet();
	int32_t leftFlashSpace = EEPROMSizeGet() - blockSize * PM_BEG_BLOCK;
	uint32_t currFlashAddr = EEPROMAddrFromBlock(PM_BEG_BLOCK);

	while(true)
	{
		uint8_t *pmBufferBeg = (uint8_t*)pmBuffer,
				*pmBufferPtr = (uint8_t*)pmBuffer;

		if(nextInd < 0)
		{
			if(isRollOver)
			{
				nextInd = BUFFER_SIZE - 1;
			}
			else
			{
				EEPROMProgram(&stopWord, currFlashAddr, WORD_SIZE);
				break;
			}
		}

		if(nextInd == index)
		{
			EEPROMProgram(&stopWord, currFlashAddr, WORD_SIZE);
			break;
		}

		*pmBufferPtr++ = PM_ELEM_BEG;
		pmBufferPtr++; // leave off for further length value
		*pmBufferPtr++ = g_buffer[nextInd].logLevel;
		*pmBufferPtr++ = g_buffer[nextInd].component;
		*((uint32_t*)pmBufferPtr)= g_buffer[nextInd].timestampMilis;
		pmBufferPtr += WORD_SIZE;
		*((uint32_t*)pmBufferPtr) = g_buffer[nextInd].stringPtr;
		pmBufferPtr += WORD_SIZE;
		uint8_t argsNum = g_buffer[nextInd].argsNum;
		*pmBufferPtr++ =  argsNum;
		memcpy(pmBufferPtr, g_buffer[nextInd].argTypes, argsNum);
		pmBufferPtr += argsNum;
		uint8_t argsBuffSize = g_buffer[nextInd].argsBuffSize;
		*pmBufferPtr++ = argsBuffSize;
		memcpy(pmBufferPtr, g_buffer[nextInd].argsBuffer, argsBuffSize);
		pmBufferPtr += argsBuffSize;

		uint16_t u8WriteSize = pmBufferPtr - pmBufferBeg;
		uint16_t u32WriteSize = u8WriteSize / WORD_SIZE;

		if(u8WriteSize % WORD_SIZE)
			u32WriteSize++;

		u8WriteSize = u32WriteSize * WORD_SIZE;

		*(pmBufferBeg + PM_LEN_OFFSET) = u8WriteSize;

		leftFlashSpace -= u8WriteSize;

		if(leftFlashSpace <= WORD_SIZE)
		{
			EEPROMProgram(&stopWord, currFlashAddr, WORD_SIZE);
			break;
		}

		if(EEPROMProgram(pmBuffer, currFlashAddr, u8WriteSize))
			return false;

		currFlashAddr += u8WriteSize;
		nextInd--;
	}

	return true;
}

bool getNextPMLine(uint32_t* timestamp, LogLevel* logLevel, LogComponent* component,
					void** strPtr, uint8_t* argsNum, void* argTypes, void* argsBufferPtr,
					uint8_t* argsBuffSize)
{
	const uint16_t MAX_ELEM_BUFF_SIZE = 30;
	uint16_t WORD_SIZE = sizeof(uint32_t);
	const uint8_t MAX_ARGS_BUFFER_SIZE = 64 * 10;
	uint32_t pmBuffer[MAX_ELEM_BUFF_SIZE];

	memset(pmBuffer, 0, MAX_ELEM_BUFF_SIZE * WORD_SIZE);

	static uint32_t blockSize = 0;
	static int32_t leftFlashSpace = 0;
	static uint32_t currFlashAddr = 0;
	static isStart = true;

	blockSize = EEPROMSizeGet() / EEPROMBlockCountGet();

	uint8_t* pmBufferPtr = (uint8_t*)pmBuffer;

	if(isStart)
	{
		currFlashAddr = EEPROMAddrFromBlock(PM_BEG_BLOCK);
		leftFlashSpace = EEPROMSizeGet() - blockSize * PM_BEG_BLOCK;
		isStart = false;
	}

	while(true)
	{
		EEPROMRead(pmBufferPtr, currFlashAddr, WORD_SIZE);
		leftFlashSpace -= WORD_SIZE;
		currFlashAddr += WORD_SIZE;

		if(*pmBufferPtr == PM_ELEM_BEG)
			break;

		if(*pmBufferPtr == PM_STOP_MARK || leftFlashSpace <= WORD_SIZE)
		{
			isStart = true;
			return false;
		}
	}

	pmBufferPtr++;

	uint8_t elemLen = *pmBufferPtr++;

	if(elemLen - WORD_SIZE > leftFlashSpace)
	{
		isStart = true;
		return false;
	}

	*logLevel = *pmBufferPtr++;
	*component = *pmBufferPtr++;

	EEPROMRead(pmBufferPtr, currFlashAddr, elemLen - WORD_SIZE);
	leftFlashSpace -= elemLen - WORD_SIZE;
	currFlashAddr += elemLen - WORD_SIZE;

	*timestamp = *((uint32_t*)pmBufferPtr);
	pmBufferPtr += WORD_SIZE;
	*strPtr = *((uint32_t*)pmBufferPtr);
	pmBufferPtr += WORD_SIZE;
	*argsNum = *pmBufferPtr++;

	if(*argsNum > MAX_ARGS_NUM)
		*argsNum = MAX_ARGS_NUM;

	memcpy(argTypes, pmBufferPtr, *argsNum);
	pmBufferPtr += *argsNum;

	*argsBuffSize = *pmBufferPtr++;

	if(*argsBuffSize > MAX_ARGS_BUFFER_SIZE)
		*argsBuffSize = MAX_ARGS_BUFFER_SIZE;

	memcpy(argsBufferPtr, pmBufferPtr, *argsBuffSize);
	pmBufferPtr += *argsBuffSize;


	return true;
}

