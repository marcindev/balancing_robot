
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "logger.h"
#include <stdarg.h>

#define BUFFER_SIZE			50

#define INT_ARG				1
#define DOUBLE_ARG			2

#define MAX_ARGS_NUM		10

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
	*argTypes = g_buffer[nextInd].argTypes;
	*argsBufferPtr = g_buffer[nextInd].argsBuffer;
	*argsBuffSize = g_buffer[nextInd].argsBuffSize;

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
