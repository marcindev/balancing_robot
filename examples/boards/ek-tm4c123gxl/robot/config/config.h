#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
	struct Binary
	{
		uint32_t address;
		bool isActive;
		bool isChecked;
		bool isGood;
		uint16_t version;
	}binary1, binary2;
}Config;

Config config;

bool isConfigRead();
void readConfig();
void writeConfig();


#endif // CONFIG_H
