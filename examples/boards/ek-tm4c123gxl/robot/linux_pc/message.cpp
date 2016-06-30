#include "message.h"
#include <algorithm>

using namespace std;

shared_ptr<BaseMessage> BaseMessage::getMessageFromPayload(unsigned char* data)
{
    shared_ptr<BaseMessage> msg;
    size_t length = getMsgSize(data);

    switch(*data)
    {
    case GET_LOGS_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<GetLogsMsgReq>(shared_ptr<GetLogsMsgReq>(new GetLogsMsgReq)));
        break;
    case GET_LOGS_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<GetLogsMsgRsp>(shared_ptr<GetLogsMsgRsp>(new GetLogsMsgRsp)));
        break;
    case HANDSHAKE_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<HandshakeMsgReq>(shared_ptr<HandshakeMsgReq>(new HandshakeMsgReq)));
        break;
    case HANDSHAKE_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<HandshakeMsgRsp>(shared_ptr<HandshakeMsgRsp>(new HandshakeMsgRsp)));
        break;
    case GET_FREE_HEAP_SIZE_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<GetFreeHeapSizeReq>(shared_ptr<GetFreeHeapSizeReq>(new GetFreeHeapSizeReq)));
        break;
    case GET_FREE_HEAP_SIZE_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<GetFreeHeapSizeRsp>(shared_ptr<GetFreeHeapSizeRsp>(new GetFreeHeapSizeRsp)));
        break;
    case GET_TASK_LIST_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<GetTaskListReq>(shared_ptr<GetTaskListReq>(new GetTaskListReq)));
        break;
    case GET_TASK_LIST_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<GetTaskListRsp>(shared_ptr<GetTaskListRsp>(new GetTaskListRsp)));
        break;
    case WHEEL_SET_SPEED_TCP_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<WheelSetSpeedTcpMsgReq>(shared_ptr<WheelSetSpeedTcpMsgReq>(new WheelSetSpeedTcpMsgReq)));
        break;
    case WHEEL_SET_SPEED_TCP_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<WheelSetSpeedTcpMsgRsp>(shared_ptr<WheelSetSpeedTcpMsgRsp>(new WheelSetSpeedTcpMsgRsp)));
        break;
    case WHEEL_RUN_TCP_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<WheelRunTcpMsgReq>(shared_ptr<WheelRunTcpMsgReq>(new WheelRunTcpMsgReq)));
        break;
    case WHEEL_RUN_TCP_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<WheelRunTcpMsgRsp>(shared_ptr<WheelRunTcpMsgRsp>(new WheelRunTcpMsgRsp)));
        break;
    case SET_TASK_PRIORITY_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<SetTaskPriorityMsgReq>(shared_ptr<SetTaskPriorityMsgReq>(new SetTaskPriorityMsgReq)));
        break;
    case SET_TASK_PRIORITY_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<SetTaskPriorityMsgRsp>(shared_ptr<SetTaskPriorityMsgRsp>(new SetTaskPriorityMsgRsp)));
        break;
    case GET_POSTMORTEM_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<GetPostmortemMsgReq>(shared_ptr<GetPostmortemMsgReq>(new GetPostmortemMsgReq)));
        break;
    case GET_POSTMORTEM_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<GetPostmortemMsgRsp>(shared_ptr<GetPostmortemMsgRsp>(new GetPostmortemMsgRsp)));
        break;
    case UPDATER_CMD_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<UpdaterCmdMsgReq>(shared_ptr<UpdaterCmdMsgReq>(new UpdaterCmdMsgReq)));
        break;
    case UPDATER_CMD_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<UpdaterCmdMsgRsp>(shared_ptr<UpdaterCmdMsgRsp>(new UpdaterCmdMsgRsp)));
        break;
    case UPDATER_SEND_DATA_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<UpdaterSendDataMsgReq>(shared_ptr<UpdaterSendDataMsgReq>(new UpdaterSendDataMsgReq)));
        break;
    case UPDATER_SEND_DATA_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<UpdaterSendDataMsgRsp>(shared_ptr<UpdaterSendDataMsgRsp>(new UpdaterSendDataMsgRsp)));
        break;
    case MPU_REG_READ_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MpuRegReadMsgReq>(shared_ptr<MpuRegReadMsgReq>(new MpuRegReadMsgReq)));
        break;
    case MPU_REG_READ_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MpuRegReadMsgRsp>(shared_ptr<MpuRegReadMsgRsp>(new MpuRegReadMsgRsp)));
        break;
    case MPU_REG_WRITE_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MpuRegWriteMsgReq>(shared_ptr<MpuRegWriteMsgReq>(new MpuRegWriteMsgReq)));
        break;
    case MPU_REG_WRITE_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MpuRegWriteMsgRsp>(shared_ptr<MpuRegWriteMsgRsp>(new MpuRegWriteMsgRsp)));
        break;
    case MPU_GET_DATA_TCP_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MpuGetDataTcpMsgReq>(shared_ptr<MpuGetDataTcpMsgReq>(new MpuGetDataTcpMsgReq)));
        break;
    case MPU_GET_DATA_TCP_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MpuGetDataTcpMsgRsp>(shared_ptr<MpuGetDataTcpMsgRsp>(new MpuGetDataTcpMsgRsp)));
        break;
    case MCTRL_SET_PID_PARAM_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MctrlSetPidParamMsgReq>(shared_ptr<MctrlSetPidParamMsgReq>(new MctrlSetPidParamMsgReq)));
        break;
    case MCTRL_SET_PID_PARAM_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MctrlSetPidParamMsgRsp>(shared_ptr<MctrlSetPidParamMsgRsp>(new MctrlSetPidParamMsgRsp)));
        break;
    case MCTRL_SET_PID_DIR_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MctrlSetPidDirMsgReq>(shared_ptr<MctrlSetPidDirMsgReq>(new MctrlSetPidDirMsgReq)));
        break;
    case MCTRL_SET_PID_DIR_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MctrlSetPidDirMsgRsp>(shared_ptr<MctrlSetPidDirMsgRsp>(new MctrlSetPidDirMsgRsp)));
        break;
    case MCTRL_SET_PERIOD_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MctrlSetPeriodMsgReq>(shared_ptr<MctrlSetPeriodMsgReq>(new MctrlSetPeriodMsgReq)));
        break;
    case MCTRL_SET_PERIOD_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MctrlSetPeriodMsgRsp>(shared_ptr<MctrlSetPeriodMsgRsp>(new MctrlSetPeriodMsgRsp)));
        break;
    case MCTRL_GET_DATA_MSG_REQ:
        msg = shared_ptr<BaseMessage>(new Message<MctrlGetDataMsgReq>(shared_ptr<MctrlGetDataMsgReq>(new MctrlGetDataMsgReq)));
        break;
    case MCTRL_GET_DATA_MSG_RSP:
        msg = shared_ptr<BaseMessage>(new Message<MctrlGetDataMsgRsp>(shared_ptr<MctrlGetDataMsgRsp>(new MctrlGetDataMsgRsp)));
        break;
    default:
        msg = nullptr;
        break;
    }

    copy(data, data + length, reinterpret_cast<unsigned char*>(msg->getRawPayload()));

    return msg;
}
