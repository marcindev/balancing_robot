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
		cout << "Usage: setTaskPriority <isMaster> <taskId> <priority>" << endl;
		return;
	}

	if(args.size() != 3)
	{
		cout << "Incorrect number of parameters required 3" << endl;
		return;
	}

	std::string argument(args.front());

	if(argument != "0" && argument != "1")
	{
		cout << "GetTaskListCmd: improper master/slave parameter" << endl;
		return;
	}

	bool isMaster = (argument == "0") ? false : true;

	stringstream ss;
	unsigned taskId, priority;

	ss << args[1];
	ss >> taskId;

	if(!ss.good())
	{
		cout << "Incorrect argument: \"" << args[0] << "\"" << endl;
		return;
	}

	ss.str("");
	ss.clear();

	ss << args[2];
	ss >> priority;

	if(!ss.good())
	{
		cout << "Incorrect argument: \"" << args[1] << "\"" << endl;
		return;
	}


	shared_ptr<SetTaskPriorityMsgReq> request(new SetTaskPriorityMsgReq);
	*request = INIT_SET_TASK_PRIORITY_MSG_REQ;
	request->isMaster = isMaster;
	request->taskId = taskId;
	request->priority = priority;

	connection->send(shared_ptr<BaseMessage>(new Message<SetTaskPriorityMsgReq>(request)));

	cout << "Priority " << priority
		 << " for task no. " << taskId
		 << " has been set" << endl;
}
