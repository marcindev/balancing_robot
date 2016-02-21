#ifndef GET_LOGS_COMMAND_H
#define GET_LOGS_COMMAND_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class GetLogsCommand : public Command
{
public:
	GetLogsCommand(std::shared_ptr<Connection> conn);
	GetLogsCommand(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
	bool handleGetLogsRsp(const GetLogsMsgRsp& response);
	static bool sortLine(const std::string& str1, const std::string& str2);
	std::vector<std::string> vecLogs;
private:
	std::string millisToTimeString(uint32_t milliseconds);

	static const double CONN_TIMEOUT;
};

#endif // GET_LOGS_COMMAND_H
