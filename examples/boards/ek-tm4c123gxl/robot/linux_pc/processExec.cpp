#include "processExec.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <algorithm>

using namespace std;

ProcessExec::ProcessExec(const std::string& _programName) :
        programName(_programName),
        isCaptureOutput(false),
        output(new string())
{

}


bool ProcessExec::execute(const vector<string>& args)
{
    char* chArgs[args.size() + 2];

    chArgs[0] = const_cast<char*>(programName.c_str());

    int i = 1;

    for(const auto& arg : args)
    {
        chArgs[i] = const_cast<char*>(arg.c_str());
        ++i;
    }

    chArgs[i] = NULL;

    int pipeFileDescs[2];

    if(isCaptureOutput)
    {
        if (pipe(pipeFileDescs) == -1)
        {
            cout << "ERR: couldn't create a pipe!" << endl;
            return false;
        }
    }

    pid_t pid = fork();

    switch(pid)
    {
    case -1:
    {
        cout << "ERR: Couldn't fork a process!" << endl;
        return false;
    }
    case 0:
    {
        if(isCaptureOutput)
        {
            while ((dup2(pipeFileDescs[PIPE_ENTRY], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
            close(pipeFileDescs[PIPE_ENTRY]);
            close(pipeFileDescs[PIPE_EXIT]);
        }

        execv(programName.c_str(), chArgs);
        cout << "ERR: Execl failed!" << endl;
        return false;
    }
    default:
    {
        if(isCaptureOutput)
        {
            close(pipeFileDescs[PIPE_ENTRY]);
            captureOutput(pipeFileDescs);
        }

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
    }

    return true;
}

bool ProcessExec::execute()
{
    vector<string> dummyArgs;

    return execute(dummyArgs);
}

void ProcessExec::setCaptureOutput(bool isOn)
{
    isCaptureOutput = isOn;
}

shared_ptr<const string> ProcessExec::getOutput() const
{
    return const_pointer_cast<const string>(output);
}

void ProcessExec::captureOutput(int pipeFileDescs[])
{
    char buffer[4096];

    *output = "";

    while (true)
    {
        ssize_t count = read(pipeFileDescs[PIPE_EXIT], buffer, sizeof(buffer));

        if (count == -1)
        {
            if (errno == EINTR)
                continue;
            else
                return;
        }
        else if (count == 0)
        {
            break;
        }
        else
        {
            *output += string(buffer, count);
        }
    }

    close(pipeFileDescs[PIPE_EXIT]);
}


