
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "../messages/messages.h"
#include <sstream>

using namespace std;

const size_t BUFFER_SIZE = 512;
size_t totalLineNumber = 0;
const string getLogsString = "getLogs";
string g_paramIp = "192.168.2.5";

int sockfd, n;
struct sockaddr_in serv_addr;
struct hostent *server;
char buffer[BUFFER_SIZE];
vector<string> vecLogs;

const uint16_t PORT = 5001;

bool establishConnection();
bool receiveTcpMsg();
void sendTcpMsg(uint8_t msgId, void* msg);
void handleMessages();
bool sortLine(const string& str1, const string& str2);
bool handleGetLogsRsp(GetLogsMsgRsp* response);
void getLogs(bool isMaster);

int main(int argc, char** argv)
{

	if(argc > 2)
	{
		cout << "Invalid number of parameters" << endl;
		return -1;
	}

	if(argc == 2)
		g_paramIp = string(argv[1]);


	while(!establishConnection()) { }

	string line;
	while(true)
	{
		cout << "command: ";
		getline(cin, line);
		string argument;
		cin >> argument;

		if(argument == getLogsString)
		{
			bool isMaster;
			cin >> argument;
			isMaster = argument != string("0");
			getLogs(isMaster);
		}
		else
		{
			cout << "\nWrong command!" << endl;
		}

		cout << endl;
	}

	return 0;
}

void getLogs(bool isMaster)
{
    memset(buffer, 0, BUFFER_SIZE);

    GetLogsMsgReq request = INIT_GET_LOGS_MSG_REQ;
    request.isMaster = isMaster;

    sendTcpMsg(request.msgId, (void*) &request);

	while(true)
	{

		if(receiveTcpMsg())
		{
			uint8_t msgId = buffer[0];
			if(msgId != GET_LOGS_MSG_RSP)
				cout << "Unrecognized msg: " << hex << static_cast<int>(msgId) << endl;

			if(!handleGetLogsRsp((GetLogsMsgRsp*) &buffer[0]))
				break;
		}

	}
}

void handleMessages()
{
	uint8_t msgId = buffer[0];

	switch(msgId)
	{
	case GET_LOGS_MSG_RSP:
		handleGetLogsRsp((GetLogsMsgRsp*) &buffer[0]);
		break;

	default:
		// Received not-recognized message
		cout << "Unrecognized msg: " << hex << static_cast<int>(msgId) << endl;
		break;
	}
}

bool handleGetLogsRsp(GetLogsMsgRsp* response)
{
	string line;
	stringstream ss;
	totalLineNumber = response->totalLineNum;

	ss << response->lineNum;
	line += ss.str() + " ";
	ss.str("");
	ss << response->timestamp;
	line += ss.str() + " ";
	ss.str("");
	switch(response->logLevel)
	{
	case 0x01:
		ss << "Info";
		break;
	case 0x02:
		ss << "Warning";
		break;
	case 0x04:
		ss << "Error";
		break;
	case 0x08:
		ss << "Debug";
		break;
	default:
	ss << "Unknown flag( " << static_cast<int>(response->logLevel) << " )";
	}
	line += ss.str() + " ";
	ss.str("");
	ss.clear();
	switch(response->component)
	{
	case 0:
		ss << "Log_Robot";
		break;
	case 1:
		ss << "Log_Motors";
		break;
	case 2:
		ss << "Log_Encoders";
		break;
	case 3:
		ss << "Log_TcpServer";
		break;
	case 4:
		ss << "Log_TcpServerHandler";
		break;
	case 5:
		ss << "Log_Wlan";
		break;
	case 6:
		ss << "Log_GpioExpander";
		break;
	case 7:
		ss << "Log_I2CManager";
		break;
	case 8:
		ss << "Log_I2CTask";
		break;
	case 9:
		ss << "Log_Wheels";
		break;
	case 10:
		ss << "Log_ServerSpiCom";
		break;
	default:
	ss << "Unknown flag( " << static_cast<int>(response->component) << " )";
	}
	line += ss.str() + " ";
	ss.str("");
	ss.clear();

	string strTemp(reinterpret_cast<char*>(response->strBuffer));
	uint64_t* argsBuffer = &response->argsBuffer[0];
	const size_t FORMAT_BUFF_SIZE = 200;
	char formatBuffer[FORMAT_BUFF_SIZE] = {0};
	size_t charsNum = 0;

	size_t pos = strTemp.find('%');

	if(pos != string::npos)
	{
		line += strTemp.substr(0, pos);

		for(int i = 0; i != response->argsNum; ++i)
		{
			size_t tempPos = strTemp.find('%', pos + 1);

			if(tempPos != string::npos)
			{
				if(tempPos - pos)
				{
					charsNum  = snprintf(formatBuffer,
										FORMAT_BUFF_SIZE,
										strTemp.substr(pos, tempPos - pos).c_str(),
										argsBuffer[i]
										);

					line += string(formatBuffer).substr(0, charsNum);
				}
				pos = tempPos;
			}
			else
			{
				charsNum  = snprintf(formatBuffer,
									FORMAT_BUFF_SIZE,
									strTemp.substr(pos).c_str(),
									argsBuffer[i]
									);

				line += string(formatBuffer).substr(0, charsNum);
				break;
			}

		}
	}
	else
	{
		line += strTemp;
	}

	vecLogs.push_back(line);


	if(vecLogs.size() == totalLineNumber)
	{
		sort(vecLogs.begin(), vecLogs.end(), sortLine);

		for(auto it = vecLogs.begin(); it != vecLogs.end(); ++it)
		{
			if(it->empty())
				return false;

			cout << *it << endl;
		}

		vecLogs.clear();

		return false;
	}

	return true;

}

bool sortLine(const string& str1, const string& str2)
{
	uint32_t lineNum1 = 0, lineNum2 = 0;
	stringstream ss;
	ss.str(str1);
	ss >> lineNum1;
	ss.clear();
	ss.str(str2);
	ss >> lineNum2;
	return lineNum1 < lineNum2;
}

bool establishConnection()
{
	static bool paramIpChecked = false;
	static int ipLastNum = 2;
	string ipPrefix("192.168.2.");
	stringstream ss;
	static string scanningString("Scanning for robot server ");
	scanningString += ".";
	cout << scanningString << "\r";
	cout.flush();


	if(ipLastNum == 255)
	{
		cout << "No more addresses, host not found in given range" << endl;
	}

	if(!paramIpChecked)
	{
		ipPrefix = g_paramIp;
		ipLastNum = 1;
		paramIpChecked = true;
	}
	else
	{
		ss << ipLastNum;
		ipPrefix += ss.str();
	}

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "ERROR opening socket" << endl;
        return false;
    }

    server = gethostbyname(ipPrefix.c_str());
    if (server == NULL)
    {
        cout << "ERROR, no such host: " << ipPrefix << endl;
        ipLastNum++;
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
        ipLastNum++;
        close(sockfd);
        return false;
    }

    cout << endl;
    cout << "Connection established with " << ipPrefix << endl;
    return true;
}



bool receiveTcpMsg()
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
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

	// receive 1 byte to check msgId
	status = read(sockfd, &(buffer[bufferIndex++]), 1);

	if(status == 0)
	{
		return false;
	}

	if(status < 0)
	{
		cout << "Couldn't receive TCP message" << endl;
		return false;
	}

	msgLen = getMsgSize((uint8_t) buffer[0]);

	if(msgLen == 0)
		return false;

	leftToRcv = msgLen - 1;

	while(leftToRcv > 0)
	{
		status = read(sockfd, &(buffer[bufferIndex]), leftToRcv);

		if(status == 0)
			return true;

		if(status < 0)
		{
			cout << "Couldn't receive TCP message" << endl;
			return false;
		}


		leftToRcv -= status;
		bufferIndex += status;
	}


	return true;
}

void sendTcpMsg(uint8_t msgId, void* msg)
{
	int16_t bufferIndex = 0;
	int16_t status = 0;
	int16_t msgLen = 0;
	int16_t leftToSend = 0;

	uint8_t* msgPtr = (uint8_t*) msg;

	msgLen = getMsgSize(msgId);
	leftToSend = msgLen;

	while(leftToSend > 0)
	{
		status = write(sockfd, &(msgPtr[bufferIndex]), leftToSend);
		write(sockfd,buffer,strlen(buffer));

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

