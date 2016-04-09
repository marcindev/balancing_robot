#ifndef SERVER_SPI_COM_H
#define SERVER_SPI_COM_H

#include <stdint.h>
#include <stdbool.h>


void initializeSpi();
bool receiveSpiMsg(void** msg);
bool sendSpiMsg(void* msg);
bool updateRoutingTable(void* msg);


#endif // SERVER_SPI_COM_H
