
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
	Log_Encoders,
	Log_TcpServer,
	Log_TcpServerHandler,
	Log_Wlan,
	Log_GpioExpander,
	Log_I2CManager,
	Log_I2CTask,
	Log_Wheels,
	Log_ServerSpiCom,
	Log_Leds,
	Log_Updater
} LogComponent;

void logger(LogLevel level, LogComponent component, const char* string, ...);
bool getNextLogLine(uint32_t* timestamp,
					LogLevel* logLevel,
					LogComponent* component,
		            void** strPtr,
					uint8_t* argsNum,
					void** argsTypes,
					void** argsBufferPtr,
					uint8_t* argsBuffSize);
bool dumpPostMortem();
bool getNextPMLine(uint32_t* timestamp,
				   LogLevel* logLevel,
				   LogComponent* component,
					void** strPtr,
					uint8_t* argsNum,
					void* argTypes,
					void* argsBufferPtr,
					uint8_t* argsBuffSize);

uint16_t getLinesNumber();
void resetLogLineGetter();


#endif // LOGGER_H
