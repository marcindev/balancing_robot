#include "command.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include "processExec.h"

using namespace std;

const unsigned Command::DEF_TIMEOUT = 100;

Command::Command(std::shared_ptr<Connection> conn) : connection(conn)
{
    stringToConfOption["make_exec"] = ConfOption::make_exec;
    stringToConfOption["makefile_dir"] = ConfOption::makefile_dir;
    stringToConfOption["binary_name"] = ConfOption::binary_name;
    stringToConfOption["last_partition"] = ConfOption::last_partition;
    stringToConfOption["linker_filename"] = ConfOption::linker_filename;
    stringToConfOption["last_target"] = ConfOption::last_target;
    stringToConfOption["axf_file_name"] = ConfOption::axf_file_name;


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
    stringToConfOption["axf_file_name"] = ConfOption::axf_file_name;

    if(!readConfig())
        cout << "ERR: Couldn't read config!" << endl;
    else
        isConfigRead = true;

}

Command::~Command()
{
    writeConfig();

    stop();
}

void Command::execute()
{
    _thread.reset(new thread(&Command::run, this));
}


void Command::wait()
{
    if(!_thread)
        return;

    if(_thread->joinable())
        _thread->join();

}

void Command::stop()
{
    _isStopped = true;

    wait();
}

bool Command::isStopped()
{
    return _isStopped;
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
    ProcessExec process(execFile);

    return process.execute(args);
}
