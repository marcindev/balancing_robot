#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include "getMpuDataCmd.h"

using namespace std;

GetMpuDataCmd::GetMpuDataCmd(shared_ptr<Connection> conn) :
        Command(conn),
        option(OneShot),
        period(0)

{

}

GetMpuDataCmd::GetMpuDataCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
        Command(conn, _args),
        option(OneShot),
        period(0)
{
    handleOptions();

    if(isDump)
        file.open(fileName);
}


void GetMpuDataCmd::run()
{
    if(isHelp)
    {
        printHelp();
        return;
    }

    shared_ptr<MpuGetDataTcpMsgReq> request(new MpuGetDataTcpMsgReq);
    *request = INIT_MPU_GET_DATA_TCP_MSG_REQ;

    connection->send(shared_ptr<BaseMessage>(new Message<MpuGetDataTcpMsgReq>(request)));

    while(connection->isConnected())
    {
        shared_ptr<BaseMessage> msg;
        if(connection->receive(msg))
        {

            uint8_t msgId = msg->getMsgId();

            if(msgId != MPU_GET_DATA_TCP_MSG_RSP)
            {
                cout << "getMpuDataCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
                continue;
            }

            handleResponse(*Message<MpuGetDataTcpMsgRsp>(*msg).getPayload());

            switch(option)
            {
            case OneShot:
                return;
            case Continuously:
                break;
            case Periodically:
                this_thread::sleep_for(chrono::milliseconds(period));
                break;
            }

            connection->send(shared_ptr<BaseMessage>(new Message<MpuGetDataTcpMsgReq>(request)));

        }

    }

}

void GetMpuDataCmd::printHelp()
{
    cout << "Usage: getMpuData [options]" << endl;
    cout << "Options:" << endl;
    cout << "--help, -h  : prints help" << endl;
    cout << "--continuous, -c   : receives data continuously" << endl;
    cout << "--periodic, -p <period>  : receives data periodically, period in milliseconds must be specified" << endl;
    cout << "--dump, -d <filename>  : dumps data to file" << endl;

}

void GetMpuDataCmd::handleOptions()
{
    size_t cnt = 0;

    for(const auto& arg : args)
    {
        ++cnt;

        if(arg[0] == '-')
        {


            if(arg == "--help" || arg == "-h")
            {
                isHelp = true;
                break;
            }

            if(arg == "--continuous" || arg == "-c")
            {
                option = Continuously;
            }

            if(arg == "--periodic" || arg == "-p")
            {
                option = Periodically;

                if(cnt >= args.size())
                {
                    isHelp = true;
                }
                else
                {
                    stringstream ss;

                    ss << args[cnt];
                    ss >> period;

                    if(ss.fail())
                    {
                        cout << "Incorrect argument: \"" << args[cnt] << "\"" << endl;

                        isHelp = true;
                        return;
                    }

                }

            }

            if(arg == "--dump" || arg == "-d")
            {
                isDump = true;

                if(cnt >= args.size())
                {
                    isHelp = true;
                }
                else
                {
                    fileName = args[cnt];
                }
            }

        }



    }
}

bool GetMpuDataCmd::handleResponse(const MpuGetDataTcpMsgRsp& response)
{
    if(!response.status)
    {
        cout << "getMpuDataCmd: couldn't read the register" << endl;
    }
    else
    {

        if(isDump)
        {
            file << fixed << setprecision(3);
            file << response.accelX << " "
                 << response.accelY << " "
                 << response.gyroX << " "
                 << response.gyroY << " "
                 << response.gyroZ << " "
                 << endl;
        }
        else
        {
            cout << fixed << setprecision(3);
            cout << "accelX: " << response.accelX
                 << " accelY: " << response.accelY
                 << " gyroX: " << response.gyroX
                 << " gyroY: " << response.gyroY
                 << " gyroZ: " << response.gyroZ << "           \r";

            cout.flush();
        }
    }



    return false;
}

