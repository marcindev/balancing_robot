//**************************************
// Message definitions for IPC
// autor: Marcin Gozdziewski
//
//**************************************

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdbool.h>
#include <stdint.h>

uint16_t getMsgSize(uint8_t msgId);

// Message ids

#define HANDSHAKE_MSG_REQ								0x05
#define HANDSHAKE_MSG_RSP								0x06
#define TEST_MSG_REQ									0x07
#define TEST_MSG_RSP									0x08
#define I2C_SEND_MSG_REQ								0x09
#define I2C_SEND_MSG_RSP								0x0A
#define I2C_RECEIVE_MSG_REQ								0x0B
#define I2C_RECEIVE_MSG_RSP								0x0C
#define I2C_SEND_N_RECEIVE_MSG_REQ						0x0D
#define I2C_SEND_N_RECEIVE_MSG_RSP						0x0E
#define MOTOR_SET_DUTY_CYCLE_MSG_REQ					0x0F
#define MOTOR_SET_DUTY_CYCLE_MSG_RSP					0x10
#define GET_LOGS_MSG_REQ								0x11
#define GET_LOGS_MSG_RSP								0x12
#define GET_TASKS_INFO_MSG_REQ							0x13
#define GET_TASKS_INFO_MSG_RSP							0x14
#define MOTOR_SET_DIRECTION_MSG_REQ						0x15
#define MOTOR_SET_DIRECTION_MSG_RSP						0x14
#define ENCODER_GET_COUNTER_MSG_REQ						0x16
#define ENCODER_GET_COUNTER_MSG_RSP						0x17
#define ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ			0x18
#define ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP			0x19
#define ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ				0x1A
#define ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP				0x1B
#define WHEEL_SET_SPEED_MSG_REQ							0x1C
#define WHEEL_SET_SPEED_MSG_RSP							0x1D
#define WHEEL_GET_SPEED_MSG_REQ							0x1E
#define WHEEL_GET_SPEED_MSG_RSP							0x1F
#define WHEEL_SET_ACCELERATION_MSG_REQ					0x20
#define WHEEL_SET_ACCELERATION_MSG_RSP					0x21
#define WHEEL_GET_ACCELERATION_MSG_REQ					0x22
#define WHEEL_GET_ACCELERATION_MSG_RSP					0x23
#define WHEEL_RUN_MSG_REQ								0x24
#define WHEEL_RUN_MSG_RSP								0x25
#define ENCODER_GET_SPEED_MSG_REQ						0x26
#define ENCODER_GET_SPEED_MSG_RSP						0x27
#define MOTOR_START_MSG_REQ								0x28
#define MOTOR_START_MSG_RSP								0x29
#define MOTOR_STOP_MSG_REQ								0x2A
#define MOTOR_STOP_MSG_RSP								0x2B
#define START_TASK_MSG_REQ								0x2C
#define START_TASK_MSG_RSP								0x2D


// TODO: try with union messages or memcpy

//***********************
// Local messages
//***********************

// i2c task messages
typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t slaveAddress;
	uint32_t length;
	uint8_t* data;
}I2cSendMsgReq;
extern const I2cSendMsgReq INIT_I2C_SEND_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}I2cSendMsgRsp;
extern const I2cSendMsgRsp INIT_SEND_I2C_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t slaveAddress;
	uint32_t length;
}I2cReceiveMsgReq;
extern const I2cReceiveMsgReq INIT_I2C_RECEIVE_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint32_t length;
	uint8_t* data;
	uint8_t status;
}I2cReceiveMsgRsp;
extern const I2cReceiveMsgRsp INIT_I2C_RECEIVE_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t slaveAddress;
	uint32_t sentLength;
	uint8_t* data;
	uint32_t rcvLength;
}I2cSendAndReceiveMsgReq;
extern const I2cSendAndReceiveMsgReq INIT_I2C_SEND_N_RECEIVE_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint32_t length;
	uint8_t* data;
	uint8_t status;
}I2cSendAndReceiveMsgRsp;
extern const I2cSendAndReceiveMsgRsp INIT_I2C_SEND_N_RECEIVE_MSG_RSP;

// motor task messages
typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t motorId;
	uint8_t dutyCycle;
}MotorSetDutyCycleMsgReq;
extern const MotorSetDutyCycleMsgReq INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}MotorSetDutyCycleMsgRsp;
extern const MotorSetDutyCycleMsgRsp INIT_MOTOR_SET_DUTY_CYCLE_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t motorId;
	uint8_t direction;
}MotorSetDirectionMsgReq;
extern const MotorSetDirectionMsgReq INIT_MOTOR_SET_DIRECTION_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}MotorSetDirectionMsgRsp;
extern const MotorSetDirectionMsgRsp INIT_MOTOR_SET_DIRECTION_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t motorId;
}MotorStartMsgReq;
extern const MotorStartMsgReq INIT_MOTOR_START_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}MotorStartMsgRsp;
extern const MotorStartMsgRsp INIT_MOTOR_START_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t motorId;
}MotorStopMsgReq;
extern const MotorStopMsgReq INIT_MOTOR_STOP_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}MotorStopMsgRsp;
extern const MotorStopMsgRsp INIT_MOTOR_STOP_MSG_RSP;

// encoder task messages

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t encoderId;
}EncoderGetCounterMsgReq;
extern const EncoderGetCounterMsgReq INIT_ENCODER_GET_COUNTER_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	int64_t counterVal;
}EncoderGetCounterMsgRsp;
extern const EncoderGetCounterMsgRsp INIT_ENCODER_GET_COUNTER_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t encoderId;
}EncoderGetSpeedMsgReq;
extern const EncoderGetSpeedMsgReq INIT_ENCODER_GET_SPEED_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint64_t speed;
}EncoderGetSpeedMsgRsp;
extern const EncoderGetSpeedMsgRsp INIT_ENCODER_GET_SPEED_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t encoderId;
	uint8_t direction;
	int64_t rotations;
}EncoderNotifyAfterRotationsMsgReq;
extern const EncoderNotifyAfterRotationsMsgReq INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t encoderId;
	int64_t actualRotations;
}EncoderNotifyAfterRotationsMsgRsp;
extern const EncoderNotifyAfterRotationsMsgRsp INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t encoderId;
	uint64_t speed;
}EncoderNotifyAfterSpeedMsgReq;
extern const EncoderNotifyAfterSpeedMsgReq INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t encoderId;
	uint64_t actualSpeed;
}EncoderNotifyAfterSpeedMsgRsp;
extern const EncoderNotifyAfterSpeedMsgRsp INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP;

// wheel task messages

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t wheelId;
	float speed; // angular
}WheelSetSpeedMsgReq;
extern const WheelSetSpeedMsgReq INIT_WHEEL_SET_SPEED_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}WheelSetSpeedMsgRsp;
extern const WheelSetSpeedMsgRsp INIT_WHEEL_SET_SPEED_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t wheelId;
}WheelGetSpeedMsgReq;
extern const WheelGetSpeedMsgReq INIT_WHEEL_GET_SPEED_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	float speed; // angular
	uint8_t status;
}WheelGetSpeedMsgRsp;
extern const WheelGetSpeedMsgRsp INIT_WHEEL_GET_SPEED_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t wheelId;
	float speed; // angular
}WheelSetAccelerationMsgReq;
extern const WheelSetAccelerationMsgReq INIT_WHEEL_SET_ACCELERATION_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}WheelSetAccelerationMsgRsp;
extern const WheelSetAccelerationMsgRsp INIT_WHEEL_SET_ACCELERATION_MSG_RSP;


typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t wheelId;
}WheelGetAccelerationMsgReq;
extern const WheelGetAccelerationMsgReq INIT_WHEEL_GET_ACCELERATION_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
	float speed; // angular
}WheelGetAccelerationMsgRsp;
extern const WheelGetAccelerationMsgRsp INIT_WHEEL_GET_ACCELERATION_MSG_RSP;

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
	uint8_t wheelId;
	uint8_t direction;
	float rotations;
}WheelRunMsgReq;
extern const WheelRunMsgReq INIT_WHEEL_RUN_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}WheelRunMsgRsp;
extern const WheelRunMsgRsp INIT_WHEEL_RUN_MSG_RSP;

// task messages

typedef struct
{
	uint8_t msgId;
	uint8_t sender;
}StartTaskMsgReq;
extern const StartTaskMsgReq INIT_START_TASK_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t status;
}StartTaskMsgRsp;
extern const StartTaskMsgRsp INIT_START_TASK_MSG_RSP;



//***********************
// Remote messages
//***********************
typedef struct
{
	uint8_t msgId;
	uint8_t slot;
}TcpMsgHeader;


typedef struct
{
	uint8_t msgId;
	uint8_t slot;
	uint8_t isMaster;
}GetLogsMsgReq;
extern const GetLogsMsgReq INIT_GET_LOGS_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint8_t slot;
	uint8_t isMaster;
	uint16_t lineNum;
	uint16_t totalLineNum;
	uint32_t timestamp;
	uint8_t logLevel;
	uint8_t component;
	uint8_t argsNum;
	uint64_t argsBuffer[10];
	uint8_t strBuffer[100];
}GetLogsMsgRsp;
extern const GetLogsMsgRsp INIT_GET_LOGS_MSG_RSP;

typedef struct
{
	uint8_t msgId;
}GetTasksInfoMsgReq;
extern const GetTasksInfoMsgReq INIT_GET_TASKS_INFO_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint16_t lineNum;
	uint16_t totalLineNum;
	uint32_t timestamp;
	uint8_t logLevel;
	uint8_t buffer[100];
}GetTasksInfoMsgRsp;
extern const GetTasksInfoMsgRsp INIT_GET_TASKS_INFO_MSG_RSP;

/*
typedef struct
{
	enum { MSGID = HANDSHAKE_MSG_REQ} msgId;
} HandshakeMsgReq;

typedef struct
{
	enum { MSGID = HANDSHAKE_MSG_RSP} msgId;
} HandshakeMsgRsp;

typedef struct
{
	enum { MSGID = TEST_MSG_REQ} msgId;
	uint8_t size;
	uint8_t* data;
} TestMsgReq;

typedef struct
{
	enum { MSGID = TEST_MSG_RSP} msgId;
	uint8_t size;
	uint8_t* data;
} TestMsgRsp;
*/

#endif // MESSAGES_H
