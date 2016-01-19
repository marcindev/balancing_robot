
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include "connection.h"
#include "commandFactory.h"


using namespace std;

int main(int argc, char** argv)
{
	if(argc > 2)
	{
		cout << "Invalid number of parameters" << endl;
		return -1;
	}


	shared_ptr<Connection> connection;

	if(argc == 2)
		connection.reset(new Connection(string(argv[1])));
	else
		connection.reset(new Connection());

	connection->start();


	CommandFactory commFactory(connection);


	while(true)
	{
		string line;
		cout << "command: ";
		string commandName, argument;
		vector<string> argsVec;

		getline(cin, line);

		stringstream ss(line);
		ss >> commandName;

		while(ss >> argument)
		{
			argsVec.push_back(argument);
		}

//		cin.clear();
//		cin.ignore();

		shared_ptr<Command> command(commFactory.createCommand(commandName, argsVec));

		if(command)
		{
			command->execute();
			command->wait();
		}

		cout << endl;
	}

	return 0;
}




