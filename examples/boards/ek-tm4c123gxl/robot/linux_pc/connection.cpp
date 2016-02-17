#include "connection.h"

#include "../messages/messages.h"
#include <sstream>
#include <chrono>


#include "command.h"
#include "commandFactory.h"

using namespace std;


Connection::Connection() :
		startTime(0),
		sendTime(0),
		sockfd(0),
		server(nullptr)
{

}

Connection::Connection(const std::string& _ipAdress) :
		startTime(0),
		sendTime(0),
		sockfd(0),
		server(nullptr),
		ipAdress(_ipAdress)
{

}

void Connection::start()
{
	_thread.reset(new thread(&Connection::run, this));
}

void Connection::run()
{
	if(!establishConnection())
		return;

	while(true)
	{
		if(!_isConnected && !isInReset)
			return;

		sendNextMsg();
		receiveNextMsg();
		sendHandShake();
//		checkConnection();
	}
}

bool Connection::establishConnection()
{

	if(!ipAdress.empty() && tryConnect(ipAdress))
		return true;

	int ipLastNum = 2;
	std::string scanningString("Scanning for robot server ");

	do
	{
		if(ipLastNum == 255)
		{
			cout << "No more addresses, host not found in given range" << endl;
			return false;
		}

		string ipPrefix("192.168.2.");
		stringstream ss;

		scanningString += ".";
		cout << scanningString << "\r";
		cout.flush();
		ss << ipLastNum;
		ipAdress = ipPrefix + ss.str();
		ipLastNum++;

	}while(!tryConnect(ipAdress));

	return true;

}

bool Connection::resetConnection()
{
	bool status = false;

	isInReset = true;
	disconnect();
	this_thread::sleep_for(chrono::seconds(10));
	status = establishConnection();
	isInReset = false;

	return status;
}

bool Connection::tryConnect(const std::string& strIp)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "ERROR opening socket" << endl;
        return false;
    }

    server = gethostbyname(strIp.c_str());
    if (server == NULL)
    {
        cout << "ERROR, no such host: " << strIp << endl;
        close(sockfd);
        return false;
    }

    //cout << "Host found: " << ipPrefix << endl;

    memset((void *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(PORT);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        //cout << "ERROR connecting to " << ipPrefix << endl;
        close(sockfd);
        return false;
    }

    _isConnected = true;
	time(&startTime);
    cout << endl;
    cout << "Connection established with " << strIp << endl;
    return true;
}

void Connection::wait()
{
	if(!_thread)
		return;

	_thread->join();

}

void Connection::disconnect()
{
	close(sockfd);
	_isConnected = false;

}

void Connection::checkConnection()
{
	time_t currTime;
	time(&currTime);

	if(difftime(currTime, startTime) > CONN_TIMEOUT)
		connectionLost();
}

void Connection::connectionLost()
{
	_isConnected = false;
	cout << "Connection lost!" << endl;
}

void Connection::send(const Message& msg)
{
	lock_guard<mutex> txLock(txMutex);
	txMessages.push(msg);
}

bool Connection::receive(Message& msg)
{
	lock_guard<mutex> rxLock(rxMutex);

	if(rxMessages.empty())
		return false;

	msg = rxMessages.front();
	rxMessages.pop();

	return true;
}

void Connection::sendHandShake()
{

	time_t currTime;
	time(&currTime);

	if(difftime(currTime, sendTime) <= CONN_HANDSHAKE_PERIOD)
		return;

	HandshakeMsgReq* handShakeReq = new HandshakeMsgReq;
	*handShakeReq = INIT_HANDSHAKE_MSG_REQ;
	shared_ptr<void> payload(handShakeReq);

	Message msg(payload);

	send(msg);

	time(&sendTime);
}

bool Connection::receiveHandShake()
{
	uint8_t msgId = buffer[0];

	if(msgId == HANDSHAKE_MSG_RSP)
	{
		time(&startTime);
		return true;
	}

	return false;
}

bool Connection::sendNextMsg()
{
	Message msg;

	{
		lock_guard<mutex> txLock(txMutex);
		if(txMessages.empty())
			return false;

		msg = txMessages.front();
		txMessages.pop();
	}

	sendTcpMsg(msg.getRawPayload());

	return true;
}

bool Connection::receiveNextMsg()
{
	if(receiveTcpMsg())
	{
		if(receiveHandShake())
			return false;

		size_t payloadSize = getMsgSize(&buffer[0]);
		shared_ptr<void> payload(new char[payloadSize]);
		memcpy(payload.get(), &buffer[0], payloadSize);
		Message msg(payload);

		{
			lock_guard<mutex> rxLock(rxMutex);
			rxMessages.push(msg);
		}

		return true;
	}

	return false;
}

bool Connection::receiveTcpMsg()
{
	const int16_t MSG_LEN_OFFSET = 2;
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t leftToRcv = 0;

	fd_set         input;
	FD_ZERO(&input);
	FD_SET(sockfd, &input);
	struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 5000;
	status = select(sockfd + 1, &input, NULL, NULL, &timeout);

	if(status <= 0)
	{
		return false;
	}

	if (!FD_ISSET(sockfd, &input))
	   return false;

	leftToRcv = MSG_LEN_OFFSET;

	while(leftToRcv > 0)
	{
		if(!_isConnected)
			return false;

		status = read(sockfd, &(buffer[bufferIndex]), leftToRcv);

		if(status == 0)
			return false;

		if(status < 0)
		{
			cout << "Couldn't receive TCP message" << endl;
			return false;
		}


		leftToRcv -= status;
		bufferIndex += status;

		if(bufferIndex == MSG_LEN_OFFSET)
			leftToRcv = getMsgSize(&buffer[0]) - MSG_LEN_OFFSET;
	}


	return true;
}

void Connection::sendTcpMsg(void* msg)
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
	int16_t leftToSend = 0;

	uint8_t* msgPtr = (uint8_t*) msg;

	msgLen = getMsgSize(msg);
	leftToSend = msgLen;

	while(leftToSend > 0)
	{
		if(!_isConnected)
			return;

		status = write(sockfd, &(msgPtr[bufferIndex]), leftToSend);

		if(status == 0)
			return;

		if(status < 0)
		{
			cout << "Couldn't send TCP message" << endl;
			return;
		}

		leftToSend -= status;
		bufferIndex += status;
	}

}


