#ifndef SET_TASK_PRIORITY_CMD_H
#define SET_TASK_PRIORITY_CMD_H

#include "../messages/messages.h"
#include <memory>
#include <iostream>
#include <sstream>

#include "command.h"

class SetTaskPriorityCmd : public Command
{
public:
    SetTaskPriorityCmd(std::shared_ptr<Connection> conn);
    SetTaskPriorityCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
    void run();


};

#endif // SET_TASK_PRIORITY_CMD_H
