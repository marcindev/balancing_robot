#ifndef CONNECTION_H
#define CONNECTION_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <memory>
#include <queue>
#include <mutex>
#include <thread>
#include <time.h>
#include "message.h"


class Connection
{
public:
	Connection();
	Connection(const std::string& _ipAdress);
	~Connection(){}
	void start();
	bool establishConnection();
	void send(const Message& msg);
	bool receive(Message& msg);
	bool isConnected() { return _isConnected; }
private:
	void run();
	bool tryConnect(const std::string& ip);
	void checkConnection();
	void connectionLost();
	void sendHandShake();
	bool receiveHandShake();
	bool sendNextMsg();
	bool receiveNextMsg();
	bool receiveTcpMsg();
	void sendTcpMsg(void* msg);

	std::queue<Message> txMessages;
	std::queue<Message> rxMessages;

	std::shared_ptr<std::thread> _thread;
	std::mutex txMutex, rxMutex;

	time_t startTime, sendTime;
	bool _isConnected = false;

	int sockfd;
	sockaddr_in serv_addr;
	hostent *server;
	std::string ipAdress;
	char buffer[512];

	const unsigned PORT = 5001;
	const double CONN_TIMEOUT = 20.0;
	const double CONN_HANDSHAKE_PERIOD = 1.0;
};

#endif // CONNECTION_H
