
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "logger.h"
#include <stdarg.h>

#define BUFFER_SIZE			100

typedef struct
{
	uint32_t timestampMilis;
	LogLevel logLevel;
	LogComponent component;
	void* stringPtr;
	uint8_t argsNum;
	uint8_t* argsBuffer;
}LogElem;

static LogElem g_buffer[BUFFER_SIZE];
static uint16_t index = 0;
static bool isRollOver = false;

inline void logger(LogLevel level, LogComponent component, const char* string, ...)
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

	while(string[i] != 0)
	{
		if((i > 0) && (string[i] == '%') && (string[i - 1] != '%'))
			argsNum++;

		++i;
	}

	uint8_t doubleWordSize = sizeof(uint64_t);
	g_buffer[index].argsNum = argsNum;

	if(argsNum > 0)
	{
		uint8_t* argsBuff = (uint8_t*) pvPortMalloc(argsNum * doubleWordSize);
		va_start(arguments, argsNum);
		uint8_t* argsBuffPtr = argsBuff;

		for(int j = 0; j != argsNum; ++j)
		{
			uint64_t arg = (uint64_t) va_arg(arguments, uint64_t);
			uint8_t* bytePtr = (uint8_t*) &arg;

			for(int k = 0; k != doubleWordSize; ++k)
			{
				*argsBuffPtr = *bytePtr;
				++argsBuffPtr;
				++bytePtr;
			}
		}

		va_end ( arguments );

		if(isRollOver && g_buffer[index].argsBuffer)
			vPortFree(g_buffer[index].argsBuffer);

		g_buffer[index].argsBuffer = argsBuff;
	}
	else
	{
		g_buffer[index].argsBuffer = 0;
	}


	index++;
}

bool getNextLogLine(uint32_t* timestamp, LogLevel* logLevel, LogComponent* component,
					void** strPtr, uint8_t* argsNum, void** argsBufferPtr)
{
	if((index == 0) && (!isRollOver))
		return false;

	static isStart = true;
	static uint16_t nextInd = 0;

	if(isStart)
	{
		nextInd = (isRollOver ? index : 0);
		isStart = false;
	}

	if(nextInd == BUFFER_SIZE)
		nextInd = 0;

	*timestamp = g_buffer[nextInd].timestampMilis;
	*logLevel = g_buffer[nextInd].logLevel;
	*component = g_buffer[nextInd].component;
	*strPtr = g_buffer[nextInd].stringPtr;
	*argsNum = g_buffer[nextInd].argsNum;
	*argsBufferPtr = g_buffer[nextInd].argsBuffer;

	if(nextInd == index)
	{
		isStart = true;
		return false;
	}

	nextInd++;

	return true;
}

uint16_t getLinesNumber()
{
	return (isRollOver ?  BUFFER_SIZE : index);
}
