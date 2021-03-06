#include "getLogsCommand.h"

#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include "stackEncoder.h"

using namespace std;

const unsigned GetLogsCommand::RESP_TIMEOUT = 200;

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

    isMaster = (argument == "0") ? false : true;

    shared_ptr<GetLogsMsgReq> getLogsMsgReq(new GetLogsMsgReq);
    *getLogsMsgReq = INIT_GET_LOGS_MSG_REQ;
    getLogsMsgReq->isMaster = isMaster;

    connection->send(shared_ptr<BaseMessage>(new Message<GetLogsMsgReq>(getLogsMsgReq)));

//  time_t startTime, currTime;
//  time(&startTime);

    unsigned timeoutCnt = 0;

    while(connection->isConnected())
    {
//      time(&currTime);
//      double timeDiff = difftime(currTime, startTime);
//
//      if(timeDiff > CONN_TIMEOUT)
//          break;

        shared_ptr<BaseMessage> msg;
        if(connection->receive(msg, RESP_TIMEOUT))
        {
            uint8_t msgId = msg->getMsgId();

            if(msgId != GET_LOGS_MSG_RSP)
                cout << "GetLogsCommand: unrecognized msg " << hex << static_cast<int>(msgId) << endl;

            if(!handleGetLogsRsp(*Message<GetLogsMsgRsp>(*msg).getPayload()))
                break;
	    
	    connection->send(shared_ptr<BaseMessage>(new Message<GetLogsMsgReq>(getLogsMsgReq)));
        }
        else
        {
            ++timeoutCnt;

            if(timeoutCnt == 10)
                break;
        }

    }

    if(vecLogs.empty())
        return;

    sort(vecLogs.begin(), vecLogs.end(), sortLine);

    uint8_t partition = getPartitionNum();

    StackEncoder stackEncoder(isMaster, partition);

    for(auto it = vecLogs.begin(); it != vecLogs.end(); ++it)
    {
        if(it->empty())
            continue;

        stackEncoder.decodeStacktrace(*it);
        cout << *it << endl;
    }

//  vecLogs.clear();

    execOnEvent(Event::finished);
}

std::string GetLogsCommand::getLogText() const
{
    ostringstream oss;

    copy(vecLogs.cbegin(), vecLogs.cend(), ostream_iterator<string>(oss, "\n"));

    return oss.str();
}

bool GetLogsCommand::handleGetLogsRsp(const GetLogsMsgRsp& response)
{
    string line;
    stringstream ss;
    uint16_t totalLineNumber = response.totalLineNum;
    logNum = totalLineNumber;
    
    if(!response.status)
	return false;

    execOnEvent(Event::log_line_received);

    ss << response.lineNum;
    line += ss.str() + " ";
    ss.str("");

    line += millisToTimeString(response.timestamp) + " ";

    switch(response.logLevel)
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
    ss << "Unknown flag( " << static_cast<int>(response.logLevel) << " )";
    }
    line += ss.str() + " ";
    ss.str("");
    ss.clear();
    switch(response.component)
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
    case 12:
        ss << "Log_Updater";
        break;
    case 13:
        ss << "Log_Mpu";
        break;
    case 14:
        ss << "Log_MotionCtrl";
        break;
    default:
    ss << "Unknown flag( " << static_cast<int>(response.component) << " )";
    }
    line += ss.str() + " ";
    ss.str("");
    ss.clear();

    string strTemp(reinterpret_cast<const char*>(response.strBuffer));
    const uint8_t* argsBuffer = reinterpret_cast<const uint8_t*>(&response.argsBuffer[0]);
    const size_t FORMAT_BUFF_SIZE = 200;
    char formatBuffer[FORMAT_BUFF_SIZE] = {0};
    size_t charsNum = 0;

    const uint8_t DOUBLE_ARG = 2;

    const uint8_t* argsBufferPtr = argsBuffer;

    size_t pos = strTemp.find('%');

    if(pos != string::npos)
    {
        line += strTemp.substr(0, pos);

        for(int i = 0; i != response.argsNum; ++i)
        {
            size_t tempPos = strTemp.find('%', pos + 1);

            if(tempPos != string::npos)
            {
                if(tempPos - pos)
                {
                    if(response.argTypes[i] == DOUBLE_ARG)
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
                if(response.argTypes[i] == DOUBLE_ARG)
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

uint8_t GetLogsCommand::getPartitionNum()
{
    uint8_t partition = 1;

    for(const auto& line : vecLogs)
    {
        if(line.find("Partition2") != string::npos)
        {
            size_t pos = line.find("=");

            if(pos == string::npos)
                break;

            ++pos;
            if(line[pos] == '1')
            {
                partition = 2;
                break;
            }

        }

        if(line.find("Partition1") != string::npos)
        {
            size_t pos = line.find("=");

            if(pos == string::npos)
                break;

            ++pos;
            if(line[pos] == '1')
            {
                partition = 1;
                break;
            }

        }
    }

    return partition;

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
