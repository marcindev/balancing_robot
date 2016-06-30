#include "setPidDirection.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

SetPidDirectionCmd::SetPidDirectionCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

SetPidDirectionCmd::SetPidDirectionCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
        Command(conn, _args)
{

}


void SetPidDirectionCmd::run()
{
    if(args.empty())
    {
        cout << "Usage: SetPidDirection <direction>" << endl;
        cout << "\tdirection: {0 - Direct, 1 - Reverse}" << endl;
        return;
    }

    stringstream ss;
    uint32_t direction;

    ss << args[0];
    ss >> direction;

    if(ss.fail())
    {
        cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
        return;
    }

    ss.str("");
    ss.clear();

    shared_ptr<MctrlSetPidDirMsgReq> request(new MctrlSetPidDirMsgReq);
    *request = INIT_MCTRL_SET_PID_DIR_MSG_REQ;
    request->direction = static_cast<uint8_t>(direction);

    connection->send(shared_ptr<BaseMessage>(new Message<MctrlSetPidDirMsgReq>(request)));

    while(connection->isConnected())
    {
        shared_ptr<BaseMessage> msg;
        if(connection->receive(msg))
        {
            uint8_t msgId = msg->getMsgId();

            if(msgId != MCTRL_SET_PID_DIR_MSG_RSP)
            {
                cout << "SetPidDirectionCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
                continue;
            }

            if(!handleResponse(*Message<MctrlSetPidDirMsgRsp>(*msg).getPayload()))
                break;
        }

    }

}

bool SetPidDirectionCmd::handleResponse(const MctrlSetPidDirMsgRsp& response)
{
    if(!response.status)
    {
        cout << "SetPidDirectionCmd: couldn't set the PID direction" << endl;
    }
    else
    {
        cout << "Direction set successfully" << endl;
    }

    return false;
}

