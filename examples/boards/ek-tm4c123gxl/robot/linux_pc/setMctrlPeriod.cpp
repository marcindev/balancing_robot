#include "setMctrlPeriod.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

SetMctrlPeriodCmd::SetMctrlPeriodCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

SetMctrlPeriodCmd::SetMctrlPeriodCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void SetMctrlPeriodCmd::run()
{
	if(args.empty())
	{
		cout << "Usage: setMctrlPeriod <periodMs>" << endl;
		return;
	}

	stringstream ss;
	uint32_t period;

	ss << args[0];
	ss >> period;

	if(ss.fail())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();

	shared_ptr<MctrlSetPeriodMsgReq> request(new MctrlSetPeriodMsgReq);
	*request = INIT_MCTRL_SET_PERIOD_MSG_REQ;
	request->periodMs = period;

	connection->send(shared_ptr<BaseMessage>(new Message<MctrlSetPeriodMsgReq>(request)));

	while(connection->isConnected())
	{
		shared_ptr<BaseMessage> msg;
		if(connection->receive(msg))
		{
			uint8_t msgId = msg->getMsgId();

			if(msgId != MCTRL_SET_PERIOD_MSG_RSP)
			{
				cout << "SetMctrlPeriodCmd: unrecognized msg " << hex << static_cast<int>(msgId) << endl;
				continue;
			}

			if(!handleResponse(*Message<MctrlSetPeriodMsgRsp>(*msg).getPayload()))
				break;
		}

	}

}

bool SetMctrlPeriodCmd::handleResponse(const MctrlSetPeriodMsgRsp& response)
{
	if(!response.status)
	{
		cout << "SetMctrlPeriodCmd: couldn't set the motion control period" << endl;
	}
	else
	{
		cout << "Period set successfully" << endl;
	}

	return false;
}

