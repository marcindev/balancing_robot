#include "setWheelSpeedCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

SetWheelSpeedCmd::SetWheelSpeedCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

SetWheelSpeedCmd::SetWheelSpeedCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void SetWheelSpeedCmd::run()
{
	if(args.empty())
	{
		cout << "Usage: setWheelSpeed <wheelId> <speed>" << endl;
		return;
	}

	if(args.size() != 2)
	{
		cout << "Incorrect number of parameters required 2" << endl;
		return;
	}

	stringstream ss;
	int wheelId = 0;

	ss << args[0];
	ss >> wheelId;

	if(ss.good())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();
	ss << args[1];
	float speed;
	ss >> speed;

	if(ss.good())
	{
		cout << "Incorrect argument: \"" << args[1] << "\"" << endl;
		return;
	}


	WheelSetSpeedTcpMsgReq* request = new WheelSetSpeedTcpMsgReq;
	*request = INIT_WHEEL_SET_SPEED_TCP_MSG_REQ;
	request->wheelId = wheelId;
	request->speed = speed;

	shared_ptr<void> payload(request);

	Message msg(payload);
	connection->send(msg);

	cout << "Speed has been set" << endl;
}
