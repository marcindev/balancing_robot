#include "messages.h"


const I2cSendMsgReq INIT_I2C_SEND_MSG_REQ = {I2C_SEND_MSG_REQ};
const I2cSendMsgRsp INIT_SEND_I2C_MSG_RSP = {I2C_SEND_MSG_RSP};
const I2cReceiveMsgReq INIT_I2C_RECEIVE_MSG_REQ = {I2C_RECEIVE_MSG_REQ};
const I2cReceiveMsgRsp INIT_I2C_RECEIVE_MSG_RSP = {I2C_RECEIVE_MSG_RSP};
const I2cSendAndReceiveMsgReq INIT_I2C_SEND_N_RECEIVE_MSG_REQ = {I2C_SEND_N_RECEIVE_MSG_REQ};
const I2cSendAndReceiveMsgRsp INIT_I2C_SEND_N_RECEIVE_MSG_RSP = {I2C_SEND_N_RECEIVE_MSG_RSP};
const MotorSetDutyCycleMsgReq INIT_MOTOR_SET_DUTY_CYCLE_MSG_REQ = {MOTOR_SET_DUTY_CYCLE_MSG_REQ};
const MotorSetDutyCycleMsgRsp INIT_MOTOR_SET_DUTY_CYCLE_MSG_RSP = {MOTOR_SET_DUTY_CYCLE_MSG_RSP};
const GetLogsMsgReq INIT_GET_LOGS_MSG_REQ = {GET_LOGS_MSG_REQ};
const GetLogsMsgRsp INIT_GET_LOGS_MSG_RSP = {GET_LOGS_MSG_RSP};
const MotorSetDirectionMsgReq INIT_MOTOR_SET_DIRECTION_MSG_REQ = {MOTOR_SET_DIRECTION_MSG_REQ};
const MotorSetDirectionMsgRsp INIT_MOTOR_SET_DIRECTION_MSG_RSP = {MOTOR_SET_DIRECTION_MSG_RSP};

uint16_t getMsgSize(uint8_t msgId)
{
	switch(msgId)
	{
	case I2C_SEND_MSG_REQ:
		return sizeof(I2cSendMsgReq);
	case I2C_SEND_MSG_RSP:
		return sizeof(I2cSendMsgRsp);
	case I2C_RECEIVE_MSG_REQ:
		return sizeof(I2cReceiveMsgReq);
	case I2C_RECEIVE_MSG_RSP:
		return sizeof(I2cReceiveMsgRsp);
	case I2C_SEND_N_RECEIVE_MSG_REQ:
		return sizeof(I2cSendAndReceiveMsgReq);
	case I2C_SEND_N_RECEIVE_MSG_RSP:
		return sizeof(I2cSendAndReceiveMsgRsp);
	case MOTOR_SET_DUTY_CYCLE_MSG_REQ:
		return sizeof(MotorSetDutyCycleMsgReq);
	case MOTOR_SET_DUTY_CYCLE_MSG_RSP:
		return sizeof(MotorSetDutyCycleMsgRsp);
	case MOTOR_SET_DIRECTION_MSG_REQ:
		return sizeof(MotorSetDirectionMsgReq);
	case MOTOR_SET_DIRECTION_MSG_RSP:
		return sizeof(MotorSetDirectionMsgRsp);
	case GET_LOGS_MSG_REQ:
		return sizeof(GetLogsMsgReq);
	case GET_LOGS_MSG_RSP:
		return sizeof(GetLogsMsgRsp);
	default:
		return 0;
	}
}
