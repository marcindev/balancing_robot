//**************************************
// Message definitions for IPC
// autor: Marcin Gozdziewski
//
//**************************************

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdbool.h>
#include <stdint.h>

// Currently max message size is 255 bytes
uint16_t getMsgSize(void* msg);

// Message ids

#define HANDSHAKE_MSG_REQ                               0x05
#define HANDSHAKE_MSG_RSP                               0x06
#define TEST_MSG_REQ                                    0x07
#define TEST_MSG_RSP                                    0x08
#define I2C_SEND_MSG_REQ                                0x09
#define I2C_SEND_MSG_RSP                                0x0A
#define I2C_RECEIVE_MSG_REQ                             0x0B
#define I2C_RECEIVE_MSG_RSP                             0x0C
#define I2C_SEND_N_RECEIVE_MSG_REQ                      0x0D
#define I2C_SEND_N_RECEIVE_MSG_RSP                      0x0E
#define MOTOR_SET_DUTY_CYCLE_MSG_REQ                    0x0F
#define MOTOR_SET_DUTY_CYCLE_MSG_RSP                    0x10
#define GET_LOGS_MSG_REQ                                0x11
#define GET_LOGS_MSG_RSP                                0x12
#define GET_TASKS_INFO_MSG_REQ                          0x13
#define GET_TASKS_INFO_MSG_RSP                          0x14
#define MOTOR_SET_DIRECTION_MSG_REQ                     0x15
#define MOTOR_SET_DIRECTION_MSG_RSP                     0x14
#define ENCODER_GET_COUNTER_MSG_REQ                     0x16
#define ENCODER_GET_COUNTER_MSG_RSP                     0x17
#define ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ          0x18
#define ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP          0x19
#define ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ              0x1A
#define ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP              0x1B
#define WHEEL_SET_SPEED_MSG_REQ                         0x1C
#define WHEEL_SET_SPEED_MSG_RSP                         0x1D
#define WHEEL_GET_SPEED_MSG_REQ                         0x1E
#define WHEEL_GET_SPEED_MSG_RSP                         0x1F
#define WHEEL_SET_ACCELERATION_MSG_REQ                  0x20
#define WHEEL_SET_ACCELERATION_MSG_RSP                  0x21
#define WHEEL_GET_ACCELERATION_MSG_REQ                  0x22
#define WHEEL_GET_ACCELERATION_MSG_RSP                  0x23
#define WHEEL_RUN_MSG_REQ                               0x24
#define WHEEL_RUN_MSG_RSP                               0x25
#define ENCODER_GET_SPEED_MSG_REQ                       0x26
#define ENCODER_GET_SPEED_MSG_RSP                       0x27
#define MOTOR_START_MSG_REQ                             0x28
#define MOTOR_START_MSG_RSP                             0x29
#define MOTOR_STOP_MSG_REQ                              0x2A
#define MOTOR_STOP_MSG_RSP                              0x2B
#define START_TASK_MSG_REQ                              0x2C
#define START_TASK_MSG_RSP                              0x2D
#define SERVER_STARTED_NOTIF_MSG_REQ                    0x2E
#define SERVER_STARTED_NOTIF_MSG_RSP                    0x2F
#define CONNECTION_STATUS_MSG_REQ                       0x30
#define CONNECTION_STATUS_MSG_RSP                       0x31
#define GET_FREE_HEAP_SIZE_MSG_REQ                      0x32
#define GET_FREE_HEAP_SIZE_MSG_RSP                      0x33
#define GET_TASK_LIST_MSG_REQ                           0x34
#define GET_TASK_LIST_MSG_RSP                           0x35
#define WHEEL_SET_SPEED_TCP_MSG_REQ                     0x36
#define WHEEL_SET_SPEED_TCP_MSG_RSP                     0x37
#define WHEEL_RUN_TCP_MSG_REQ                           0x38
#define WHEEL_RUN_TCP_MSG_RSP                           0x39
#define SET_TASK_PRIORITY_MSG_REQ                       0x3A
#define SET_TASK_PRIORITY_MSG_RSP                       0x3B
#define GET_POSTMORTEM_MSG_REQ                          0x3C
#define GET_POSTMORTEM_MSG_RSP                          0x3D
#define UPDATER_CMD_MSG_REQ                             0x3E
#define UPDATER_CMD_MSG_RSP                             0x3F
#define UPDATER_SEND_DATA_MSG_REQ                       0x40
#define UPDATER_SEND_DATA_MSG_RSP                       0x41
#define MPU_REG_READ_MSG_REQ                            0x42
#define MPU_REG_READ_MSG_RSP                            0x43
#define MPU_REG_WRITE_MSG_REQ                           0x44
#define MPU_REG_WRITE_MSG_RSP                           0x45
#define MPU_GET_DATA_MSG_REQ                            0x46
#define MPU_GET_DATA_MSG_RSP                            0x47
#define MPU_GET_DATA_TCP_MSG_REQ                        0x48
#define MPU_GET_DATA_TCP_MSG_RSP                        0x49
#define MCTRL_SET_PID_PARAM_MSG_REQ                     0x50
#define MCTRL_SET_PID_PARAM_MSG_RSP                     0x51
#define MCTRL_SET_PERIOD_MSG_REQ                        0x52
#define MCTRL_SET_PERIOD_MSG_RSP                        0x53
#define MCTRL_SET_PID_DIR_MSG_REQ                       0x54
#define MCTRL_SET_PID_DIR_MSG_RSP                       0x55
#define MCTRL_GET_DATA_MSG_REQ                          0x56
#define MCTRL_GET_DATA_MSG_RSP                          0x57


// TODO: try with union messages or memcpy

//***********************
// Local messages
//***********************

typedef struct
{
    uint8_t msgId;
    uint8_t msgLen;
    uint8_t queueId;
    uint8_t slot;
}MsgHeader;

// i2c task messages
typedef struct
{
    MsgHeader header;
    uint8_t slaveAddress;
    uint32_t length;
    uint8_t* data;
}I2cSendMsgReq;
extern const I2cSendMsgReq INIT_I2C_SEND_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}I2cSendMsgRsp;
extern const I2cSendMsgRsp INIT_SEND_I2C_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t slaveAddress;
    uint32_t length;
}I2cReceiveMsgReq;
extern const I2cReceiveMsgReq INIT_I2C_RECEIVE_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint32_t length;
    uint8_t* data;
    uint8_t status;
}I2cReceiveMsgRsp;
extern const I2cReceiveMsgRsp INIT_I2C_RECEIVE_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t slaveAddress;
    uint32_t sentLength;
    uint8_t* data;
    uint32_t rcvLength;
}I2cSendAndReceiveMsgReq;
extern const I2cSendAndReceiveMsgReq INIT_I2C_SEND_N_RECEIVE_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint32_t length;
    uint8_t* data;
    uint8_t status;
}I2cSendAndReceiveMsgRsp;
extern const I2cSendAndReceiveMsgRsp INIT_I2C_SEND_N_RECEIVE_MSG_RSP;

// motor task messages
typedef struct
{
    MsgHeader header;
    uint8_t motorId;
    float dutyCycle;
}MotorSetDutyCycleMsgReq;
extern const MotorSetDutyCycleMsgReq INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}MotorSetDutyCycleMsgRsp;
extern const MotorSetDutyCycleMsgRsp INIT_MOTOR_SET_DUTY_CYCLE_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t motorId;
    uint8_t direction;
}MotorSetDirectionMsgReq;
extern const MotorSetDirectionMsgReq INIT_MOTOR_SET_DIRECTION_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}MotorSetDirectionMsgRsp;
extern const MotorSetDirectionMsgRsp INIT_MOTOR_SET_DIRECTION_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t motorId;
}MotorStartMsgReq;
extern const MotorStartMsgReq INIT_MOTOR_START_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}MotorStartMsgRsp;
extern const MotorStartMsgRsp INIT_MOTOR_START_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t motorId;
}MotorStopMsgReq;
extern const MotorStopMsgReq INIT_MOTOR_STOP_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}MotorStopMsgRsp;
extern const MotorStopMsgRsp INIT_MOTOR_STOP_MSG_RSP;

// encoder task messages

typedef struct
{
    MsgHeader header;
    uint8_t encoderId;
}EncoderGetCounterMsgReq;
extern const EncoderGetCounterMsgReq INIT_ENCODER_GET_COUNTER_MSG_REQ;

typedef struct
{
    MsgHeader header;
    int64_t counterVal;
}EncoderGetCounterMsgRsp;
extern const EncoderGetCounterMsgRsp INIT_ENCODER_GET_COUNTER_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t encoderId;
}EncoderGetSpeedMsgReq;
extern const EncoderGetSpeedMsgReq INIT_ENCODER_GET_SPEED_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint64_t speed;
}EncoderGetSpeedMsgRsp;
extern const EncoderGetSpeedMsgRsp INIT_ENCODER_GET_SPEED_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t encoderId;
    uint8_t direction;
    int64_t rotations;
}EncoderNotifyAfterRotationsMsgReq;
extern const EncoderNotifyAfterRotationsMsgReq INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t encoderId;
    int64_t actualRotations;
}EncoderNotifyAfterRotationsMsgRsp;
extern const EncoderNotifyAfterRotationsMsgRsp INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t encoderId;
    uint64_t speed;
}EncoderNotifyAfterSpeedMsgReq;
extern const EncoderNotifyAfterSpeedMsgReq INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t encoderId;
    uint64_t actualSpeed;
}EncoderNotifyAfterSpeedMsgRsp;
extern const EncoderNotifyAfterSpeedMsgRsp INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP;

// wheel task messages

typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
    float speed; // angular
}WheelSetSpeedMsgReq;
extern const WheelSetSpeedMsgReq INIT_WHEEL_SET_SPEED_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}WheelSetSpeedMsgRsp;
extern const WheelSetSpeedMsgRsp INIT_WHEEL_SET_SPEED_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
}WheelGetSpeedMsgReq;
extern const WheelGetSpeedMsgReq INIT_WHEEL_GET_SPEED_MSG_REQ;

typedef struct
{
    MsgHeader header;
    float speed; // angular
    uint8_t status;
}WheelGetSpeedMsgRsp;
extern const WheelGetSpeedMsgRsp INIT_WHEEL_GET_SPEED_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
    float speed; // angular
}WheelSetAccelerationMsgReq;
extern const WheelSetAccelerationMsgReq INIT_WHEEL_SET_ACCELERATION_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}WheelSetAccelerationMsgRsp;
extern const WheelSetAccelerationMsgRsp INIT_WHEEL_SET_ACCELERATION_MSG_RSP;


typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
}WheelGetAccelerationMsgReq;
extern const WheelGetAccelerationMsgReq INIT_WHEEL_GET_ACCELERATION_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
    float speed; // angular
}WheelGetAccelerationMsgRsp;
extern const WheelGetAccelerationMsgRsp INIT_WHEEL_GET_ACCELERATION_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
    uint8_t direction;
    float rotations;
}WheelRunMsgReq;
extern const WheelRunMsgReq INIT_WHEEL_RUN_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}WheelRunMsgRsp;
extern const WheelRunMsgRsp INIT_WHEEL_RUN_MSG_RSP;

// task messages

typedef struct
{
    MsgHeader header;
}StartTaskMsgReq;
extern const StartTaskMsgReq INIT_START_TASK_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}StartTaskMsgRsp;
extern const StartTaskMsgRsp INIT_START_TASK_MSG_RSP;

typedef struct
{
    MsgHeader header;
}ServerStartedNotifMsgReq;
extern const ServerStartedNotifMsgReq INIT_SERVER_STARTED_NOTIF_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}ServerStartedNotifMsgRsp;
extern const ServerStartedNotifMsgRsp INIT_SERVER_STARTED_NOTIF_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t slot;
    uint8_t state;
}ConnectionStatusMsgReq;
extern const ConnectionStatusMsgReq INIT_CONNECTION_STATUS_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}ConnectionStatusMsgRsp;
extern const ConnectionStatusMsgRsp INIT_CONNECTION_STATUS_MSG_RSP;

// MPU messages

typedef struct
{
    MsgHeader header;
}MpuGetDataMsgReq;
extern const MpuGetDataMsgReq INIT_MPU_GET_DATA_MSG_REQ;

typedef struct
{
    MsgHeader header;
    float accelX;
    float accelY;
    float gyroX;
    float gyroY;
    float gyroZ;
    bool isAccValid;
    uint8_t status;
}MpuGetDataMsgRsp;
extern const MpuGetDataMsgRsp INIT_MPU_GET_DATA_MSG_RSP;


//***********************
// Remote messages
//***********************


typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
}__attribute__((packed, aligned(1))) GetLogsMsgReq;
extern const GetLogsMsgReq INIT_GET_LOGS_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint16_t lineNum;
    uint16_t totalLineNum;
    uint32_t timestamp;
    uint8_t logLevel;
    uint8_t component;
    uint8_t argsNum;
    uint8_t argTypes[12]; // to keep alignment
    uint64_t argsBuffer[10];
    uint8_t strBuffer[128];
    uint8_t status;
}__attribute__((packed, aligned(1))) GetLogsMsgRsp;
extern const GetLogsMsgRsp INIT_GET_LOGS_MSG_RSP;

typedef struct
{
    MsgHeader header;
}__attribute__((packed, aligned(1))) HandshakeMsgReq;
extern const HandshakeMsgReq INIT_HANDSHAKE_MSG_REQ;

typedef struct
{
    MsgHeader header;
}__attribute__((packed, aligned(1))) HandshakeMsgRsp;
extern const HandshakeMsgRsp INIT_HANDSHAKE_MSG_RSP;


typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
}__attribute__((packed, aligned(1))) GetFreeHeapSizeReq;
extern const GetFreeHeapSizeReq INIT_GET_FREE_HEAP_SIZE_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint32_t heapSize;
}__attribute__((packed, aligned(1))) GetFreeHeapSizeRsp;
extern const GetFreeHeapSizeRsp INIT_GET_FREE_HEAP_SIZE_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
}__attribute__((packed, aligned(1))) GetTaskListReq;
extern const GetTaskListReq INIT_GET_TASK_LIST_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t partId;
    uint8_t totalParts;
    uint8_t strBuffer[200];
}__attribute__((packed, aligned(1))) GetTaskListRsp;
extern const GetTaskListRsp INIT_GET_TASK_LIST_MSG_RSP;


typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
    float speed; // angular
}__attribute__((packed, aligned(1))) WheelSetSpeedTcpMsgReq;
extern const WheelSetSpeedTcpMsgReq INIT_WHEEL_SET_SPEED_TCP_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}__attribute__((packed, aligned(1))) WheelSetSpeedTcpMsgRsp;
extern const WheelSetSpeedTcpMsgRsp INIT_WHEEL_SET_SPEED_TCP_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t wheelId;
    uint8_t direction;
    float rotations;
}__attribute__((packed, aligned(1))) WheelRunTcpMsgReq;
extern const WheelRunTcpMsgReq INIT_WHEEL_RUN_TCP_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}__attribute__((packed, aligned(1))) WheelRunTcpMsgRsp;
extern const WheelRunTcpMsgRsp INIT_WHEEL_RUN_TCP_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t taskId;
    uint8_t priority;
}__attribute__((packed, aligned(1))) SetTaskPriorityMsgReq;
extern const SetTaskPriorityMsgReq INIT_SET_TASK_PRIORITY_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t status;
}__attribute__((packed, aligned(1))) SetTaskPriorityMsgRsp;
extern const SetTaskPriorityMsgRsp INIT_SET_TASK_PRIORITY_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
}__attribute__((packed, aligned(1))) GetPostmortemMsgReq;
extern const GetPostmortemMsgReq INIT_GET_POSTMORTEM_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t ctrlByte;   // 0x01 - isNormal, 0x02 - isLast, 0x04 - isEmpty
    uint16_t lineNum;
    uint32_t timestamp;
    uint8_t logLevel;
    uint8_t component;
    uint8_t argsNum;
    uint8_t argTypes[10];
    uint64_t argsBuffer[10];
    uint8_t strBuffer[128];
}__attribute__((packed, aligned(1))) GetPostmortemMsgRsp;
extern const GetPostmortemMsgRsp INIT_GET_POSTMORTEM_MSG_RSP;


typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t command;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
}__attribute__((packed, aligned(1))) UpdaterCmdMsgReq;
extern const UpdaterCmdMsgReq INIT_UPDATER_CMD_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t status;
}__attribute__((packed, aligned(1))) UpdaterCmdMsgRsp;
extern const UpdaterCmdMsgRsp INIT_UPDATER_CMD_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t checksum;
    uint8_t data[128];
    uint32_t partNum;
}__attribute__((packed, aligned(1))) UpdaterSendDataMsgReq;
extern const UpdaterSendDataMsgReq INIT_UPDATER_SEND_DATA_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t isMaster;
    uint8_t status;
}__attribute__((packed, aligned(1))) UpdaterSendDataMsgRsp;
extern const UpdaterSendDataMsgRsp INIT_UPDATER_SEND_DATA_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t regAddr;
}__attribute__((packed, aligned(1))) MpuRegReadMsgReq;
extern const MpuRegReadMsgReq INIT_MPU_REG_READ_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t regVal;
    uint8_t status;
}__attribute__((packed, aligned(1))) MpuRegReadMsgRsp;
extern const MpuRegReadMsgRsp INIT_MPU_REG_READ_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t regAddr;
    uint8_t regVal;
}__attribute__((packed, aligned(1))) MpuRegWriteMsgReq;
extern const MpuRegWriteMsgReq INIT_MPU_REG_WRITE_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}__attribute__((packed, aligned(1))) MpuRegWriteMsgRsp;
extern const MpuRegWriteMsgRsp INIT_MPU_REG_WRITE_MSG_RSP;

typedef struct
{
    MsgHeader header;
}__attribute__((packed, aligned(1))) MpuGetDataTcpMsgReq;
extern const MpuGetDataTcpMsgReq INIT_MPU_GET_DATA_TCP_MSG_REQ;

typedef struct
{
    MsgHeader header;
    float accelX;
    float accelY;
    float gyroX;
    float gyroY;
    float gyroZ;
    uint8_t status;
}__attribute__((packed, aligned(1))) MpuGetDataTcpMsgRsp;
extern const MpuGetDataTcpMsgRsp INIT_MPU_GET_DATA_TCP_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t param;
    float value;
}__attribute__((packed, aligned(1))) MctrlSetPidParamMsgReq;
extern const MctrlSetPidParamMsgReq INIT_MCTRL_SET_PID_PARAM_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}__attribute__((packed, aligned(1))) MctrlSetPidParamMsgRsp;
extern const MctrlSetPidParamMsgRsp INIT_MCTRL_SET_PID_PARAM_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint32_t periodMs;
}__attribute__((packed, aligned(1))) MctrlSetPeriodMsgReq;
extern const MctrlSetPeriodMsgReq INIT_MCTRL_SET_PERIOD_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}__attribute__((packed, aligned(1))) MctrlSetPeriodMsgRsp;
extern const MctrlSetPeriodMsgRsp INIT_MCTRL_SET_PERIOD_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t direction;
}__attribute__((packed, aligned(1))) MctrlSetPidDirMsgReq;
extern const MctrlSetPidDirMsgReq INIT_MCTRL_SET_PID_DIR_MSG_REQ;

typedef struct
{
    MsgHeader header;
    uint8_t status;
}__attribute__((packed, aligned(1))) MctrlSetPidDirMsgRsp;
extern const MctrlSetPidDirMsgRsp INIT_MCTRL_SET_PID_DIR_MSG_RSP;

typedef struct
{
    MsgHeader header;
    uint8_t param;
}__attribute__((packed, aligned(1))) MctrlGetDataMsgReq;
extern const MctrlGetDataMsgReq INIT_MCTRL_GET_DATA_MSG_REQ;

typedef struct
{
    MsgHeader header;
    float data;
    uint8_t status;
}__attribute__((packed, aligned(1))) MctrlGetDataMsgRsp;
extern const MctrlGetDataMsgRsp INIT_MCTRL_GET_DATA_MSG_RSP;



#endif // MESSAGES_H
