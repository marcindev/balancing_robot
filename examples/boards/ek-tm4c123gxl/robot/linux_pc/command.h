#ifndef COMMAND_H
#define COMMAND_H

#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include "connection.h"

class Command
{
public:
	enum class ConfOption
	{
		make_exec,
		makefile_dir,
		binary_name,
		last_partition,
		last_target,
		linker_filename,
		axf_file_name
	};

	Command(std::shared_ptr<Connection> conn);
	Command(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);
	virtual ~Command();

	virtual void execute();
	virtual void wait();
protected:
	virtual void run() = 0;

	bool readConfig();
	bool writeConfig();
	bool executeProcess(const std::string& execFile, const std::vector<std::string>& args);
	std::string getConfValue(ConfOption option);
	void setConfValue(ConfOption option, const std::string& value);
	std::shared_ptr<Connection> connection;
	std::shared_ptr<std::thread> _thread;
	std::vector<std::string> args;
	std::map<ConfOption, std::string> confOptions;
	bool isConfigRead = false;
private:
	std::map<std::string, ConfOption> stringToConfOption;
};


#endif // COMMAND_H
