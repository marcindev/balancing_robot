#ifndef PROCESS_EXEC_H
#define PROCESS_EXEC_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>

class ProcessExec
{
public:
	ProcessExec(const std::string& _programName);
	bool execute(const std::vector<std::string>& args);
	bool execute();
	void setCaptureOutput(bool isOn);
	std::shared_ptr<const std::string> getOutput() const;
private:
	void captureOutput(int pipeFileDescs[]);
	std::string programName;
	bool isCaptureOutput;
	std::shared_ptr<std::string> output;

	const int PIPE_ENTRY = 1,
			  PIPE_EXIT = 0;

};


#endif // PROCESS_EXEC_H
