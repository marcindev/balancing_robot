
#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <stdbool.h>
#include <stdint.h>

uint8_t initTcpServer();
uint8_t runTcpServer();
void sendMsgToUart(void* msg);
void receiveMsgFromUart(void* msg);
void initServerUart();


#endif // TCP_SERVER_H
