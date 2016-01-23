#ifndef GET_FREE_HEAP_SIZE_COMMAND_H
#define GET_FREE_HEAP_SIZE_COMMAND_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class GetFreeHeapSizeCommand : public Command
{
public:
	GetFreeHeapSizeCommand(std::shared_ptr<Connection> conn);
	GetFreeHeapSizeCommand(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
	void run();
private:
	void handleResponse(GetFreeHeapSizeRsp* response);
	std::vector<std::string> vecLogs;
};

#endif // GET_FREE_HEAP_SIZE_COMMAND_H
