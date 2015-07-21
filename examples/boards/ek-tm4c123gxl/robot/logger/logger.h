
#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	Info = 0x01,
	Warning = 0x02,
	Error = 0x04,
	Debug = 0x08
} LogLevel;

typedef enum
{
	Log_Robot = 0,
	Log_Motors,
	Log_TcpServer,
	Log_TcpServerHandler,
	Log_Wlan,
	Log_GpioExpander,
	Log_I2CManager,
	Log_I2CTask
} LogComponent;

void logger(LogLevel level, LogComponent component, const char* string);
bool getNextLogLine(uint32_t* timestamp, LogLevel* logLevel, void** strPtr);
uint16_t getLinesNumber();


#endif // LOGGER_H
