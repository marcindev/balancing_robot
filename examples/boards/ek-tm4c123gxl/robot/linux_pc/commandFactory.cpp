#include "commandFactory.h"
#include "getLogsCommand.h"

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


	cout << "Wrong command!" << endl;
	return std::shared_ptr<Command>(0);

}
