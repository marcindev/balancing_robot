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

#define HANDSHAKE_MSG_REQ					0x05
#define HANDSHAKE_MSG_RSP					0x06
#define TEST_MSG_REQ						0x07
#define TEST_MSG_RSP						0x08
#define I2C_SEND_MSG_REQ					0x09
#define I2C_SEND_MSG_RSP					0x0A
#define I2C_RECEIVE_MSG_REQ					0x0B
#define I2C_RECEIVE_MSG_RSP					0x0C
#define I2C_SEND_N_RECEIVE_MSG_REQ			0x0D
#define I2C_SEND_N_RECEIVE_MSG_RSP			0x0E
#define MOTOR_SET_DUTY_CYCLE_MSG_REQ		0x0F
#define MOTOR_SET_DUTY_CYCLE_MSG_RSP		0x10
#define GET_LOGS_MSG_REQ					0x11
#define GET_LOGS_MSG_RSP					0x12
#define GET_TASKS_INFO_MSG_REQ				0x13
#define GET_TASKS_INFO_MSG_RSP				0x14
#define MOTOR_SET_DIRECTION_MSG_REQ			0x15
#define MOTOR_SET_DIRECTION_MSG_RSP			0x14
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




//***********************
// Remote messages
//***********************


typedef struct
{
	uint8_t msgId;
}GetLogsMsgReq;
extern const GetLogsMsgReq INIT_GET_LOGS_MSG_REQ;

typedef struct
{
	uint8_t msgId;
	uint16_t lineNum;
	uint16_t totalLineNum;
	uint32_t timestamp;
	uint8_t logLevel;
	uint8_t buffer[100];
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
