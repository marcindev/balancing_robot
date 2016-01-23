#include "setTaskPriorityCmd.h"

#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

SetTaskPriorityCmd::SetTaskPriorityCmd(shared_ptr<Connection> conn) : Command(conn)
{

}

SetTaskPriorityCmd::SetTaskPriorityCmd(shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		Command(conn, _args)
{

}


void SetTaskPriorityCmd::run()
{
	if(args.empty())
	{
		cout << "Usage: setTaskPriority <taskId> <priority>" << endl;
		return;
	}

	if(args.size() != 2)
	{
		cout << "Incorrect number of parameters required 2" << endl;
		return;
	}

	stringstream ss;
	unsigned taskId, priority;

	ss << args[0];
	ss >> taskId;

	if(ss.good())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();

	ss << args[1];
	ss >> priority;

	if(ss.good())
	{
		cout << "Incorrect argument: \"" << args[1] << "\"" << endl;
		return;
	}


	SetTaskPriorityMsgReq* request = new SetTaskPriorityMsgReq;
	*request = INIT_SET_TASK_PRIORITY_MSG_REQ;
	request->taskId = taskId;
	request->priority = priority;

	shared_ptr<void> payload(request);

	Message msg(payload);
	connection->send(msg);

	cout << "Priority " << priority
		 << " for task no. " << taskId
		 << " has been set" << endl;
}
