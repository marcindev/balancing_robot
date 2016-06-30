#ifndef SET_PID_PARAM_CMD_H
#define SET_PID_PARAM_CMD_H

#include "../messages/messages.h"
#include <memory>

#include "command.h"

class SetPidParamCmd : public Command
{
public:
    enum Event
    {
        response_timeout,
        finished
    };

    SetPidParamCmd(std::shared_ptr<Connection> conn);
    SetPidParamCmd(std::shared_ptr<Connection> conn, const std::vector<std::string>& _args);

protected:
    void run();
private:

    bool handleResponse(const MctrlSetPidParamMsgRsp& response);

};

#endif // SET_PID_PARAM_CMD_H
