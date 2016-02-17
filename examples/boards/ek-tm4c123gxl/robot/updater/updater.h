#ifndef UPDATER_H
#define UPDATER_H

#include <stdbool.h>
#include <stdint.h>


void initUpdater();
uint32_t getPartitionSize();
uint8_t handleUpdateCommand(uint8_t command, uint32_t data1, uint32_t data2, uint32_t data3);

#endif // UPDATER_H
