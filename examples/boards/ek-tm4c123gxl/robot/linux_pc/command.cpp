#include "command.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>

using namespace std;

Command::Command(std::shared_ptr<Connection> conn) : connection(conn)
{
	stringToConfOption["make_exec"] = ConfOption::make_exec;
	stringToConfOption["makefile_dir"] = ConfOption::makefile_dir;
	stringToConfOption["binary_name"] = ConfOption::binary_name;
	stringToConfOption["last_partition"] = ConfOption::last_partition;
	stringToConfOption["linker_filename"] = ConfOption::linker_filename;
	stringToConfOption["last_target"] = ConfOption::last_target;

	if(!readConfig())
		cout << "ERR: Couldn't read config!" << endl;
	else
		isConfigRead = true;
}

Command::Command(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args) :
		connection(conn),
		args(_args)
{
	stringToConfOption["make_exec"] = ConfOption::make_exec;
	stringToConfOption["makefile_dir"] = ConfOption::makefile_dir;
	stringToConfOption["binary_name"] = ConfOption::binary_name;
	stringToConfOption["last_partition"] = ConfOption::last_partition;
	stringToConfOption["linker_filename"] = ConfOption::linker_filename;
	stringToConfOption["last_target"] = ConfOption::last_target;

	if(!readConfig())
		cout << "ERR: Couldn't read config!" << endl;
	else
		isConfigRead = true;

}

Command::~Command()
{
	writeConfig();
}

void Command::execute()
{
	_thread.reset(new thread(&Command::run, this));
}


void Command::wait()
{
	if(!_thread)
		return;

	_thread->join();

}

string Command::getConfValue(ConfOption option)
{
	return confOptions[option];
}

void Command::setConfValue(ConfOption option, const string& value)
{
	confOptions[option] = value;
}

bool Command::readConfig()
{
	ifstream file("config.cfg");

	if(!file)
		return false;

	string line;

	while(getline(file, line))
	{
		stringstream ss(line);
		string option, value;

		ss >> option;
		map<string, ConfOption>::iterator it;
		it = stringToConfOption.find(option);

		if(it == stringToConfOption.end())
			continue;

		ss >> value;

		if(value.empty())
			continue;


		confOptions[stringToConfOption[option]] = value;

	}

	return true;
}

bool Command::writeConfig()
{
	ofstream file("config.cfg");

	if(!file)
		return false;

	map<ConfOption, string> confOptionToString;

	for(const auto& strOptPair :  stringToConfOption)
	{
		confOptionToString[strOptPair.second] = strOptPair.first;
	}

	for(const auto &optPair : confOptions)
	{
		string strOption;

		file << confOptionToString[optPair.first] << "\t\t" << optPair.second << "\n";
	}

	return true;
}

bool Command::executeProcess(const string& execFile, const vector<string>& args)
{

	char* chArgs[args.size() + 2];

	chArgs[0] = const_cast<char*>(execFile.c_str());

	int i = 1;

	for(const auto& arg : args)
	{
		chArgs[i] = const_cast<char*>(arg.c_str());
		++i;
	}

	chArgs[i] = NULL;

	pid_t pid = fork();

	switch(pid)
	{
	case -1:
		cout << "ERR: Couldn't fork a process!" << endl;
		return false;
	case 0:
		execv(execFile.c_str(), chArgs);
		cout << "ERR: Execl failed!" << endl;
		return false;
	default:
		int status;

		while (!WIFEXITED(status)) {
			waitpid(pid, &status, 0);
		}

		if(WEXITSTATUS(status))
		{
			cout << "ERR: Child process exited with status: " << WEXITSTATUS(status) << "!" << endl;
			return false;
		}

	}

	return true;
}
