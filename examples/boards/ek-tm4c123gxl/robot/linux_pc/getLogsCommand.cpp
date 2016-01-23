#include "getLogsCommand.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

const double GetLogsCommand::CONN_TIMEOUT = 2.0;

GetLogsCommand::GetLogsCommand(shared_ptr<Connection> conn) : Command(conn)
{

}

GetLogsCommand::GetLogsCommand(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void GetLogsCommand::run()
{
	if(args.empty())
	{
		cout << "GetLogsCommand: master/slave parameter not specified" << endl;
		return;
	}

	std::string argument(args.front());

	if(argument != "0" && argument != "1")
	{
		cout << "GetLogsCommand: improper master/slave parameter" << endl;
		return;
	}

	bool isMaster = (argument == "0") ? false : true;

	GetLogsMsgReq* getLogsMsgReq = new GetLogsMsgReq;
	*getLogsMsgReq = INIT_GET_LOGS_MSG_REQ;
	getLogsMsgReq->isMaster = isMaster;
	shared_ptr<void> payload(getLogsMsgReq);

	Message msg(payload);
	connection->send(msg);

	time_t startTime, currTime;
	time(&startTime);

	while(connection->isConnected())
	{
		Message msg;
		if(connection->receive(msg))
		{
			uint8_t msgId = *(reinterpret_cast<uint8_t*>(msg.getRawPayload()));

			if(msgId != GET_LOGS_MSG_RSP)
				cout << "GetLogsCommand: unrecognized msg " << hex << static_cast<int>(msgId) << endl;

			time(&currTime);
			double timeDiff = difftime(currTime, startTime);

			if(!handleGetLogsRsp(reinterpret_cast<GetLogsMsgRsp*>(msg.getRawPayload()))
				|| timeDiff > CONN_TIMEOUT)
			{
					sort(vecLogs.begin(), vecLogs.end(), sortLine);

					for(auto it = vecLogs.begin(); it != vecLogs.end(); ++it)
					{
						if(it->empty())
							continue;

						cout << *it << endl;
					}

					vecLogs.clear();

					break;
			}
		}

	}

}

bool GetLogsCommand::handleGetLogsRsp(GetLogsMsgRsp* response)
{
	string line;
	stringstream ss;
	uint16_t totalLineNumber = response->totalLineNum;
	ss << response->lineNum;
	line += ss.str() + " ";
	ss.str("");


	line += millisToTimeString(response->timestamp) + " ";

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
	case 11:
		ss << "Log_Leds";
		break;
	default:
	ss << "Unknown flag( " << static_cast<int>(response->component) << " )";
	}
	line += ss.str() + " ";
	ss.str("");
	ss.clear();

	string strTemp(reinterpret_cast<char*>(response->strBuffer));
	uint8_t* argsBuffer = reinterpret_cast<uint8_t*>(&response->argsBuffer[0]);
	const size_t FORMAT_BUFF_SIZE = 200;
	char formatBuffer[FORMAT_BUFF_SIZE] = {0};
	size_t charsNum = 0;

	const uint8_t DOUBLE_ARG = 2;

	uint8_t* argsBufferPtr = argsBuffer;

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
					if(response->argTypes[i] == DOUBLE_ARG)
					{
						charsNum  = snprintf(formatBuffer,
										FORMAT_BUFF_SIZE,
										strTemp.substr(pos, tempPos - pos).c_str(),
										*((double*)argsBufferPtr)
						);

						argsBufferPtr += sizeof(double);
					}
					else
					{
						charsNum  = snprintf(formatBuffer,
										FORMAT_BUFF_SIZE,
										strTemp.substr(pos, tempPos - pos).c_str(),
										*((uint32_t*)argsBufferPtr)
						);

						argsBufferPtr += sizeof(uint32_t);
					}

					line += string(formatBuffer).substr(0, charsNum);
				}
				pos = tempPos;
			}
			else
			{
				if(response->argTypes[i] == DOUBLE_ARG)
				{
					charsNum  = snprintf(formatBuffer,
							FORMAT_BUFF_SIZE,
							strTemp.substr(pos).c_str(),
							*((double*)argsBufferPtr)
					);

					argsBufferPtr += sizeof(double);
				}
				else
				{
					charsNum  = snprintf(formatBuffer,
							FORMAT_BUFF_SIZE,
							strTemp.substr(pos).c_str(),
							*((uint32_t*)argsBufferPtr)
					);

					argsBufferPtr += sizeof(uint32_t);
				}

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
		return false;

	return true;

}

bool GetLogsCommand::sortLine(const string& str1, const string& str2)
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

string GetLogsCommand::millisToTimeString(uint32_t milliseconds)
{
	uint32_t fraction = milliseconds % 1000;
	uint32_t seconds = (milliseconds / 1000) % 60 ;
	uint32_t minutes = (milliseconds / (1000*60)) % 60;
	uint32_t hours   = (milliseconds / (1000*60*60)) % 24;

	stringstream ss;
	ss << setfill('0') << setw(2) << hours << ':';
	ss << setfill('0') << setw(2) << minutes << ':';
	ss << setfill('0') << setw(2) << seconds << '.';
	ss << setfill('0') << setw(3) << fraction;

	return ss.str();
}
