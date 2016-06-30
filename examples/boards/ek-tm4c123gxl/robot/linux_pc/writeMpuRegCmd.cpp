#include "writeMpuRegCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

WriteMpuRegCmd::WriteMpuRegCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

WriteMpuRegCmd::WriteMpuRegCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
        Command(conn, _args)
{

}


void WriteMpuRegCmd::run()
{
    if(args.empty())
    {
        cout << "Usage: writeMpuReg <regAddres> <regVal>" << endl;
        return;
    }

    stringstream ss;
    uint32_t mpuReg, regVal;

    ss << hex << args[0];
    ss >> mpuReg;

    if(ss.fail())
    {
        cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
        return;
    }

    ss.str("");
    ss.clear();

    ss << hex << args[1];
    ss >> regVal;

    if(ss.fail())
    {
        cout << "Incorrect argument: \"" << args[1] << "\"" << endl;
        return;
    }

    ss.str("");
    ss.clear();


    shared_ptr<MpuRegWriteMsgReq> request(new MpuRegWriteMsgReq);
    *request = INIT_MPU_REG_WRITE_MSG_REQ;
    request->regAddr = static_cast<uint8_t>(mpuReg);
    request->regVal = static_cast<uint8_t>(regVal);

    connection->send(shared_ptr<BaseMessage>(new Message<MpuRegWriteMsgReq>(request)));

    while(connection->isConnected())
    {
        shared_ptr<BaseMessage> msg;
        if(connection->receive(msg))
        {
            uint8_t msgId = msg->getMsgId();

            if(msgId != MPU_REG_WRITE_MSG_RSP)
            {
                cout << "WriteMpuRegCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
                continue;
            }

            if(!handleResponse(*Message<MpuRegWriteMsgRsp>(*msg).getPayload()))
                break;
        }

    }

}

bool WriteMpuRegCmd::handleResponse(const MpuRegWriteMsgRsp& response)
{
    if(!response.status)
    {
        cout << "WriteMpuRegCmd: couldn't read the register" << endl;
    }
    else
    {
        cout << "Register written successfully" << endl;
    }

    return false;
}

