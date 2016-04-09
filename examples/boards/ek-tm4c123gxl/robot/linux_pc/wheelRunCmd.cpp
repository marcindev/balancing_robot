#include "wheelRunCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

WheelRunCmd::WheelRunCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

WheelRunCmd::WheelRunCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void WheelRunCmd::run()
{
	if(args.empty())
	{
		cout << "Usage: wheelRun <wheelId> <direction> <rotations>" << endl;
		cout << "\twheelId {0, 1}" << endl;
		cout << "\tdirection {0, 1}" << endl;
		cout << "\trotations (float)" << endl;
		return;
	}

	if(args.size() != 3)
	{
		cout << "Incorrect number of parameters required 3" << endl;
		return;
	}

	stringstream ss;
	int wheelId = 0;

	ss << args[0];
	ss >> wheelId;

	if(!ss.good())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();
	ss << args[1];
	int direction;
	ss >> direction;

	if(!ss.good())
	{
		cout << "Incorrect argument: \"" << args[1] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();
	ss << args[2];
	float rotations;
	ss >> rotations;

	if(!ss.good())
	{
		cout << "Incorrect argument: \"" << args[2] << "\"" << endl;
		return;
	}


	shared_ptr<WheelRunTcpMsgReq> request(new WheelRunTcpMsgReq);
	*request = INIT_WHEEL_RUN_TCP_MSG_REQ;
	request->wheelId = wheelId;
	request->direction = direction;
	request->rotations = rotations;

	connection->send(shared_ptr<BaseMessage>(new Message<WheelRunTcpMsgReq>(request)));

	cout << "Wheel run request has been sent" << endl;
}
