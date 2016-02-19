
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <readline/readline.h>
#include <readline/history.h>

#include "commandFactory.h"
#include "connection.h"


using namespace std;

int main(int argc, char** argv)
{
	const char* histFileName = "cmd_history";

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
    char* input;
    using_history();
    rl_bind_key('\t', rl_complete);
	read_history(histFileName);

	while(true)
	{
        input = readline("command: ");

        if (!input)
            break;

		string line(input);
		string commandName, argument;
		vector<string> argsVec;

		stringstream ss(line);
		ss >> commandName;

		while(ss >> argument)
		{
			argsVec.push_back(argument);
		}

//		cin.clear();
//		cin.ignore();

		if(line == "q" || line == "quit")
		{
			connection->disconnect();
			connection->wait();
			free(input);
			break;
		}

		shared_ptr<Command> command(commFactory.createCommand(commandName, argsVec));

		if(command)
		{
			add_history(input);
			command->execute();
			command->wait();
		}

        free(input);
		cout << endl;

	}

	write_history(histFileName);

	return 0;
}




