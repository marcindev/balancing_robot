//**************************************
// Message definitions for IPC
// autor: Marcin Gozdziewski
//
//**************************************

#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdbool.h>
#include <stdint.h>

// Message ids

#define HANDSHAKE_MSG_REQ			0x05
#define HANDSHAKE_MSG_RSP			0x06
#define TEST_MSG_REQ				0x07
#define TEST_MSG_RSP				0x08
#define I2C_SEND_MSG_REQ			0x09
#define I2C_SEND_MSG_RSP			0x0A
#define I2C_RECEIVE_MSG_REQ			0x0B
#define I2C_RECEIVE_MSG_RSP			0x0C
#define I2C_SEND_N_RECEIVE_MSG_REQ	0x0D
#define I2C_SEND_N_RECEIVE_MSG_RSP	0x0E


//***********************
// Local messages
//***********************

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



//***********************
// Remote messages
//***********************

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
