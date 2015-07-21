
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"
#include "logger.h"

#define BUFFER_SIZE		100

typedef struct
{
	uint32_t timestampMilis;
	LogLevel logLevel;
	void* stringPtr;
}LogElem;

static LogElem g_buffer[BUFFER_SIZE];
static uint16_t index = 0;
static bool isRollOver = false;

void logger(LogLevel level, LogComponent component, const char* string)
{
	if(index == BUFFER_SIZE)
	{
		index = 0;
		isRollOver = true;
	}
	g_buffer[index].timestampMilis = xTaskGetTickCount() / portTICK_RATE_MS;
	g_buffer[index].logLevel = level;
	g_buffer[index].stringPtr = string;

	index++;
}

bool getNextLogLine(uint32_t* timestamp, LogLevel* logLevel, void** strPtr)
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
	*strPtr = g_buffer[nextInd].stringPtr;

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
