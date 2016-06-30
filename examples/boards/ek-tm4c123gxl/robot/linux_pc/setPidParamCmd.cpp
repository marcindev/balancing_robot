#include "setPidParamCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

SetPidParamCmd::SetPidParamCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

SetPidParamCmd::SetPidParamCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
        Command(conn, _args)
{

}


void SetPidParamCmd::run()
{
    if(args.empty())
    {
        cout << "Usage: setPidParam <param> <value>" << endl;
        cout << "\tparam: {0 - Proportional, 1 - Integral, 2 - Derivative, 3 - SetPoint}" << endl;
        return;
    }

    stringstream ss;
    uint32_t param;
    float value;

    ss << args[0];
    ss >> param;

    if(ss.fail())
    {
        cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
        return;
    }

    ss.str("");
    ss.clear();

    ss << args[1];
    ss >> value;

    if(ss.fail())
    {
        cout << "Incorrect argument: \"" << args[1] << "\"" << endl;
        return;
    }

    ss.str("");
    ss.clear();


    shared_ptr<MctrlSetPidParamMsgReq> request(new MctrlSetPidParamMsgReq);
    *request = INIT_MCTRL_SET_PID_PARAM_MSG_REQ;
    request->param = static_cast<uint8_t>(param);
    request->value = value;

    connection->send(shared_ptr<BaseMessage>(new Message<MctrlSetPidParamMsgReq>(request)));

    while(connection->isConnected())
    {
        shared_ptr<BaseMessage> msg;
        if(connection->receive(msg, DEF_TIMEOUT))
        {
            uint8_t msgId = msg->getMsgId();

            if(msgId != MCTRL_SET_PID_PARAM_MSG_RSP)
            {
                cout << "SetPidParamCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
                continue;
            }

            if(!handleResponse(*Message<MctrlSetPidParamMsgRsp>(*msg).getPayload()))
                break;
        }
        else
        {
            execOnEvent(Event::response_timeout);
            break;
        }

    }

    execOnEvent(Event::finished);
}

bool SetPidParamCmd::handleResponse(const MctrlSetPidParamMsgRsp& response)
{
    if(!response.status)
    {
        cout << "SetPidParamCmd: couldn't set the parameter" << endl;
    }
    else
    {
        cout << "Parameter set successfully" << endl;
    }

    return false;
}

