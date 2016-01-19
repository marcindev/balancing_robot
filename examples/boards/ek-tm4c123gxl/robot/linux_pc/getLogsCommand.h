#ifndef GET_LOGS_COMMAND_H
#define GET_LOGS_COMMAND_H

#include "command.h"
#include "../messages/messages.h"
#include <memory>

class GetLogsCommand : public Command
{
public:
	GetLogsCommand(std::shared_ptr<Connection> conn);
	GetLogsCommand(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
private:
	bool handleGetLogsRsp(GetLogsMsgRsp* response);
	static bool sortLine(const std::string& str1, const std::string& str2);
	std::string millisToTimeString(uint32_t milliseconds);
	std::vector<std::string> vecLogs;
};

#endif // GET_LOGS_COMMAND_H
