#ifndef GET_LOGS_COMMAND_H
#define GET_LOGS_COMMAND_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class GetLogsCommand : public Command
{
public:
	enum Event
	{
		log_line_received,
		finished
	};

	GetLogsCommand(std::shared_ptr<Connection> conn);
	GetLogsCommand(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

	uint32_t getLogNum() const { return logNum; }
	std::string getLogText() const;
protected:
	void run();
	bool handleGetLogsRsp(const GetLogsMsgRsp& response);
	static bool sortLine(const std::string& str1, const std::string& str2);
	uint8_t getPartitionNum();

	std::vector<std::string> vecLogs;
	bool isMaster = false;
	uint32_t logNum = 0;
private:
	std::string millisToTimeString(uint32_t milliseconds);

	static const unsigned RESP_TIMEOUT;
};

#endif // GET_LOGS_COMMAND_H
