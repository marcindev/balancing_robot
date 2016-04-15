#include "messages.h"


const I2cSendMsgReq INIT_I2C_SEND_MSG_REQ = {I2C_SEND_MSG_REQ, sizeof(I2cSendMsgReq)};
const I2cSendMsgRsp INIT_SEND_I2C_MSG_RSP = {I2C_SEND_MSG_RSP, sizeof(I2cSendMsgRsp)};
const I2cReceiveMsgReq INIT_I2C_RECEIVE_MSG_REQ = {I2C_RECEIVE_MSG_REQ, sizeof(I2cReceiveMsgReq)};
const I2cReceiveMsgRsp INIT_I2C_RECEIVE_MSG_RSP = {I2C_RECEIVE_MSG_RSP, sizeof(I2cReceiveMsgRsp)};
const I2cSendAndReceiveMsgReq INIT_I2C_SEND_N_RECEIVE_MSG_REQ = {I2C_SEND_N_RECEIVE_MSG_REQ, sizeof(I2cSendAndReceiveMsgReq)};
const I2cSendAndReceiveMsgRsp INIT_I2C_SEND_N_RECEIVE_MSG_RSP = {I2C_SEND_N_RECEIVE_MSG_RSP, sizeof(I2cSendAndReceiveMsgRsp)};
const MotorSetDutyCycleMsgReq INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ = {MOTOR_SET_DUTY_CYCLE_MSG_REQ, sizeof(MotorSetDutyCycleMsgReq)};
const MotorSetDutyCycleMsgRsp INIT_MOTOR_SET_DUTY_CYCLE_MSG_RSP = {MOTOR_SET_DUTY_CYCLE_MSG_RSP, sizeof(MotorSetDutyCycleMsgRsp)};
const GetLogsMsgReq INIT_GET_LOGS_MSG_REQ = {GET_LOGS_MSG_REQ, sizeof(GetLogsMsgReq)};
const GetLogsMsgRsp INIT_GET_LOGS_MSG_RSP = {GET_LOGS_MSG_RSP, sizeof(GetLogsMsgRsp)};
const MotorSetDirectionMsgReq INIT_MOTOR_SET_DIRECTION_MSG_REQ = {MOTOR_SET_DIRECTION_MSG_REQ, sizeof(MotorSetDirectionMsgReq)};
const MotorSetDirectionMsgRsp INIT_MOTOR_SET_DIRECTION_MSG_RSP = {MOTOR_SET_DIRECTION_MSG_RSP, sizeof(MotorSetDirectionMsgRsp)};
const EncoderGetCounterMsgReq INIT_ENCODER_GET_COUNTER_MSG_REQ = {ENCODER_GET_COUNTER_MSG_REQ, sizeof(EncoderGetCounterMsgReq)};
const EncoderGetCounterMsgRsp INIT_ENCODER_GET_COUNTER_MSG_RSP = {ENCODER_GET_COUNTER_MSG_RSP, sizeof(EncoderGetCounterMsgRsp)};
const EncoderGetSpeedMsgReq INIT_ENCODER_GET_SPEED_MSG_REQ = {ENCODER_GET_SPEED_MSG_REQ, sizeof(EncoderGetSpeedMsgReq)};
const EncoderGetSpeedMsgRsp INIT_ENCODER_GET_SPEED_MSG_RSP = {ENCODER_GET_SPEED_MSG_RSP, sizeof(EncoderGetSpeedMsgRsp)};
const EncoderNotifyAfterRotationsMsgReq INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ = {ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ, sizeof(EncoderNotifyAfterRotationsMsgReq)};
const EncoderNotifyAfterRotationsMsgRsp INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP = {ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP, sizeof(EncoderNotifyAfterRotationsMsgRsp)};
const EncoderNotifyAfterSpeedMsgReq INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ = {ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ, sizeof(EncoderNotifyAfterSpeedMsgReq)};
const EncoderNotifyAfterSpeedMsgRsp INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP = {ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP, sizeof(EncoderNotifyAfterSpeedMsgRsp)};
const WheelSetSpeedMsgReq INIT_WHEEL_SET_SPEED_MSG_REQ = {WHEEL_SET_SPEED_MSG_REQ, sizeof(WheelSetSpeedMsgReq)};
const WheelSetSpeedMsgRsp INIT_WHEEL_SET_SPEED_MSG_RSP = {WHEEL_SET_SPEED_MSG_RSP, sizeof(WheelSetSpeedMsgRsp)};
const WheelGetSpeedMsgReq INIT_WHEEL_GET_SPEED_MSG_REQ = {WHEEL_GET_SPEED_MSG_REQ, sizeof(WheelGetSpeedMsgReq)};
const WheelGetSpeedMsgRsp INIT_WHEEL_GET_SPEED_MSG_RSP = {WHEEL_GET_SPEED_MSG_RSP, sizeof(WheelGetSpeedMsgRsp)};
const WheelSetAccelerationMsgReq INIT_WHEEL_SET_ACCELERATION_MSG_REQ = {WHEEL_SET_ACCELERATION_MSG_REQ, sizeof(WheelSetAccelerationMsgReq)};
const WheelSetAccelerationMsgRsp INIT_WHEEL_SET_ACCELERATION_MSG_RSP = {WHEEL_SET_ACCELERATION_MSG_RSP, sizeof(WheelSetAccelerationMsgRsp)};
const WheelGetAccelerationMsgReq INIT_WHEEL_GET_ACCELERATION_MSG_REQ = {WHEEL_GET_ACCELERATION_MSG_REQ, sizeof(WheelGetAccelerationMsgReq)};
const WheelGetAccelerationMsgRsp INIT_WHEEL_GET_ACCELERATION_MSG_RSP = {WHEEL_GET_ACCELERATION_MSG_RSP, sizeof(WheelGetAccelerationMsgRsp)};
const MotorStartMsgReq INIT_MOTOR_START_MSG_REQ = {MOTOR_START_MSG_REQ, sizeof(MotorStartMsgReq)};
const MotorStartMsgRsp INIT_MOTOR_START_MSG_RSP = {MOTOR_START_MSG_RSP, sizeof(MotorStartMsgRsp)};
const MotorStopMsgReq INIT_MOTOR_STOP_MSG_REQ = {MOTOR_STOP_MSG_REQ, sizeof(MotorStopMsgReq)};
const MotorStopMsgRsp INIT_MOTOR_STOP_MSG_RSP = {MOTOR_STOP_MSG_RSP, sizeof(MotorStopMsgRsp)};
const WheelRunMsgReq INIT_WHEEL_RUN_MSG_REQ = {WHEEL_RUN_MSG_REQ, sizeof(WheelRunMsgReq)};
const WheelRunMsgRsp INIT_WHEEL_RUN_MSG_RSP = {WHEEL_RUN_MSG_RSP, sizeof(WheelRunMsgRsp)};
const StartTaskMsgReq INIT_START_TASK_MSG_REQ = {START_TASK_MSG_REQ, sizeof(StartTaskMsgReq)};
const StartTaskMsgRsp INIT_START_TASK_MSG_RSP = {START_TASK_MSG_RSP, sizeof(StartTaskMsgRsp)};
const ServerStartedNotifMsgReq INIT_SERVER_STARTED_NOTIF_MSG_REQ = {SERVER_STARTED_NOTIF_MSG_REQ, sizeof(ServerStartedNotifMsgReq)};
const ServerStartedNotifMsgRsp INIT_SERVER_STARTED_NOTIF_MSG_RSP = {SERVER_STARTED_NOTIF_MSG_RSP, sizeof(ServerStartedNotifMsgRsp)};
const ConnectionStatusMsgReq INIT_CONNECTION_STATUS_MSG_REQ = {CONNECTION_STATUS_MSG_REQ, sizeof(ConnectionStatusMsgReq)};
const ConnectionStatusMsgRsp INIT_CONNECTION_STATUS_MSG_RSP = {CONNECTION_STATUS_MSG_RSP, sizeof(ConnectionStatusMsgRsp)};
const HandshakeMsgReq INIT_HANDSHAKE_MSG_REQ = {HANDSHAKE_MSG_REQ, sizeof(HandshakeMsgReq)};
const HandshakeMsgRsp INIT_HANDSHAKE_MSG_RSP = {HANDSHAKE_MSG_RSP, sizeof(HandshakeMsgRsp)};
const GetFreeHeapSizeReq INIT_GET_FREE_HEAP_SIZE_MSG_REQ = {GET_FREE_HEAP_SIZE_MSG_REQ, sizeof(GetFreeHeapSizeReq)};
const GetFreeHeapSizeRsp INIT_GET_FREE_HEAP_SIZE_MSG_RSP = {GET_FREE_HEAP_SIZE_MSG_RSP, sizeof(GetFreeHeapSizeRsp)};
const GetTaskListReq INIT_GET_TASK_LIST_MSG_REQ = {GET_TASK_LIST_MSG_REQ, sizeof(GetTaskListReq)};
const GetTaskListRsp INIT_GET_TASK_LIST_MSG_RSP = {GET_TASK_LIST_MSG_RSP, sizeof(GetTaskListRsp)};
const WheelSetSpeedTcpMsgReq INIT_WHEEL_SET_SPEED_TCP_MSG_REQ = {WHEEL_SET_SPEED_TCP_MSG_REQ, sizeof(WheelSetSpeedTcpMsgReq)};
const WheelSetSpeedTcpMsgRsp INIT_WHEEL_SET_SPEED_TCP_MSG_RSP = {WHEEL_SET_SPEED_TCP_MSG_RSP, sizeof(WheelSetSpeedTcpMsgRsp)};
const WheelRunTcpMsgReq INIT_WHEEL_RUN_TCP_MSG_REQ = {WHEEL_RUN_TCP_MSG_REQ, sizeof(WheelRunTcpMsgReq)};
const WheelRunTcpMsgRsp INIT_WHEEL_RUN_TCP_MSG_RSP = {WHEEL_RUN_TCP_MSG_RSP, sizeof(WheelRunTcpMsgRsp)};
const SetTaskPriorityMsgReq INIT_SET_TASK_PRIORITY_MSG_REQ = {SET_TASK_PRIORITY_MSG_REQ, sizeof(SetTaskPriorityMsgReq)};
const SetTaskPriorityMsgRsp INIT_SET_TASK_PRIORITY_MSG_RSP = {SET_TASK_PRIORITY_MSG_RSP, sizeof(SetTaskPriorityMsgRsp)};
const GetPostmortemMsgReq INIT_GET_POSTMORTEM_MSG_REQ = {GET_POSTMORTEM_MSG_REQ, sizeof(GetPostmortemMsgReq)};
const GetPostmortemMsgRsp INIT_GET_POSTMORTEM_MSG_RSP = {GET_POSTMORTEM_MSG_RSP, sizeof(GetPostmortemMsgRsp)};
const UpdaterCmdMsgReq INIT_UPDATER_CMD_MSG_REQ = {UPDATER_CMD_MSG_REQ, sizeof(UpdaterCmdMsgReq)};
const UpdaterCmdMsgRsp INIT_UPDATER_CMD_MSG_RSP = {UPDATER_CMD_MSG_RSP, sizeof(UpdaterCmdMsgRsp)};
const UpdaterSendDataMsgReq INIT_UPDATER_SEND_DATA_MSG_REQ = {UPDATER_SEND_DATA_MSG_REQ, sizeof(UpdaterSendDataMsgReq)};
const UpdaterSendDataMsgRsp INIT_UPDATER_SEND_DATA_MSG_RSP = {UPDATER_SEND_DATA_MSG_RSP, sizeof(UpdaterSendDataMsgRsp)};
const MpuRegReadMsgReq INIT_MPU_REG_READ_MSG_REQ = {MPU_REG_READ_MSG_REQ, sizeof(MpuRegReadMsgReq)};
const MpuRegReadMsgRsp INIT_MPU_REG_READ_MSG_RSP = {MPU_REG_READ_MSG_RSP, sizeof(MpuRegReadMsgRsp)};
const MpuRegWriteMsgReq INIT_MPU_REG_WRITE_MSG_REQ = {MPU_REG_WRITE_MSG_REQ, sizeof(MpuRegWriteMsgReq)};
const MpuRegWriteMsgRsp INIT_MPU_REG_WRITE_MSG_RSP = {MPU_REG_WRITE_MSG_RSP, sizeof(MpuRegWriteMsgRsp)};
const MpuGetDataMsgReq INIT_MPU_GET_DATA_MSG_REQ = {MPU_GET_DATA_MSG_REQ, sizeof(MpuGetDataMsgReq)};
const MpuGetDataMsgRsp INIT_MPU_GET_DATA_MSG_RSP = {MPU_GET_DATA_MSG_RSP, sizeof(MpuGetDataMsgRsp)};
const MpuGetDataTcpMsgReq INIT_MPU_GET_DATA_TCP_MSG_REQ = {MPU_GET_DATA_TCP_MSG_REQ, sizeof(MpuGetDataTcpMsgReq)};
const MpuGetDataTcpMsgRsp INIT_MPU_GET_DATA_TCP_MSG_RSP = {MPU_GET_DATA_TCP_MSG_RSP, sizeof(MpuGetDataTcpMsgRsp)};



uint16_t getMsgSize(void* msg)
{
	return ((MsgHeader*)msg)->msgLen;
}
