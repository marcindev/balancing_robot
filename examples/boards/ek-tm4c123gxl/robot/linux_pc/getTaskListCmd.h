#ifndef GET_TASK_LIST_CMD_H
#define GET_TASK_LIST_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class GetTaskListCmd : public Command
{
public:
    GetTaskListCmd(std::shared_ptr<Connection> conn);
    GetTaskListCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
    void run();
private:

    struct Part
    {
        Part();
        unsigned id;
        std::string str;
    };

    bool handleResponse(const GetTaskListRsp& response);
    std::vector<Part> parts;

};

#endif // GET_TASK_LIST_CMD_H
