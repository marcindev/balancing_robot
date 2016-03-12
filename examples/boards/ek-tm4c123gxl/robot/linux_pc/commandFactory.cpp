#include "getFreeHeapSizeCommand.h"
#include "commandFactory.h"

#include "getFreeHeapSizeCommand.h"
#include "getLogsCommand.h"
#include "getTaskListCmd.h"
#include "setWheelSpeedCmd.h"
#include "wheelRunCmd.h"
#include "setTaskPriorityCmd.h"
#include "getPostmortemCmd.h"
#include "updaterCmd.h"
#include "getFuncNameCmd.h"

using namespace std;

CommandFactory::CommandFactory(std::shared_ptr<Connection> conn) : connection(conn)
{

}

shared_ptr<Command> CommandFactory::createCommand(const string& strCommand,
		const vector<std::string>& args)
{
	if(!connection)
		cout << "CommandFactory: Null connection pointer!" << endl;

	if(!connection->isConnected())
	{
		cout << "CommandFactory: No connection!" << endl;
		return std::shared_ptr<Command>(0);
	}

	if(strCommand == "getLogs")
		return shared_ptr<Command>(new GetLogsCommand(connection, args));

	if(strCommand == "getFreeHeap")
		return shared_ptr<Command>(new GetFreeHeapSizeCommand(connection, args));

	if(strCommand == "getTaskList")
		return shared_ptr<Command>(new GetTaskListCmd(connection, args));

	if(strCommand == "setWheelSpeed")
		return shared_ptr<Command>(new SetWheelSpeedCmd(connection, args));

	if(strCommand == "wheelRun")
		return shared_ptr<Command>(new WheelRunCmd(connection, args));

	if(strCommand == "setTaskPriority")
		return shared_ptr<Command>(new SetTaskPriorityCmd(connection, args));

	if(strCommand == "getPostmortem")
		return shared_ptr<Command>(new GetPostmortemCmd(connection, args));

	if(strCommand == "update")
		return shared_ptr<Command>(new UpdaterCmd(connection, args));

	if(strCommand == "getFuncName")
		return shared_ptr<Command>(new GetFuncNameCmd(connection, args));


	cout << "Wrong command!" << endl;
	return std::shared_ptr<Command>(0);

}
