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
const EncoderGetCounterMsgReq INIT_ENCODER_GET_COUNTER_MSG_REQ = {ENCODER_GET_COUNTER_MSG_REQ};
const EncoderGetCounterMsgRsp INIT_ENCODER_GET_COUNTER_MSG_RSP = {ENCODER_GET_COUNTER_MSG_RSP};
const EncoderGetSpeedMsgReq INIT_ENCODER_GET_SPEED_MSG_REQ = {ENCODER_GET_SPEED_MSG_REQ};
const EncoderGetSpeedMsgRsp INIT_ENCODER_GET_SPEED_MSG_RSP = {ENCODER_GET_SPEED_MSG_RSP};
const EncoderNotifyAfterRotationsMsgReq INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ = {ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ};
const EncoderNotifyAfterRotationsMsgRsp INIT_ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP = {ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP};
const EncoderNotifyAfterSpeedMsgReq INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ = {ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ};
const EncoderNotifyAfterSpeedMsgRsp INIT_ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP = {ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP};
const WheelSetSpeedMsgReq INIT_WHEEL_SET_SPEED_MSG_REQ = {WHEEL_SET_SPEED_MSG_REQ};
const WheelSetSpeedMsgRsp INIT_WHEEL_SET_SPEED_MSG_RSP = {WHEEL_SET_SPEED_MSG_RSP};
const WheelGetSpeedMsgReq INIT_WHEEL_GET_SPEED_MSG_REQ = {WHEEL_GET_SPEED_MSG_REQ};
const WheelGetSpeedMsgRsp INIT_WHEEL_GET_SPEED_MSG_RSP = {WHEEL_GET_SPEED_MSG_RSP};
const WheelSetAccelerationMsgReq INIT_WHEEL_SET_ACCELERATION_MSG_REQ = {WHEEL_SET_ACCELERATION_MSG_REQ};
const WheelSetAccelerationMsgRsp INIT_WHEEL_SET_ACCELERATION_MSG_RSP = {WHEEL_SET_ACCELERATION_MSG_RSP};
const WheelGetAccelerationMsgReq INIT_WHEEL_GET_ACCELERATION_MSG_REQ = {WHEEL_GET_ACCELERATION_MSG_REQ};
const WheelGetAccelerationMsgRsp INIT_WHEEL_GET_ACCELERATION_MSG_RSP = {WHEEL_GET_ACCELERATION_MSG_RSP};
const MotorStartMsgReq INIT_MOTOR_START_MSG_REQ = {MOTOR_START_MSG_REQ};
const MotorStartMsgRsp INIT_MOTOR_START_MSG_RSP = {MOTOR_START_MSG_RSP};
const MotorStopMsgReq INIT_MOTOR_STOP_MSG_REQ = {MOTOR_STOP_MSG_REQ};
const MotorStopMsgRsp INIT_MOTOR_STOP_MSG_RSP = {MOTOR_STOP_MSG_RSP};
const WheelRunMsgReq INIT_WHEEL_RUN_MSG_REQ = {WHEEL_RUN_MSG_REQ};
const WheelRunMsgRsp INIT_WHEEL_RUN_MSG_RSP = {WHEEL_RUN_MSG_RSP};


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
	case ENCODER_GET_COUNTER_MSG_REQ:
		return sizeof(EncoderGetCounterMsgReq);
	case ENCODER_GET_COUNTER_MSG_RSP:
		return sizeof(EncoderGetCounterMsgRsp);
	case ENCODER_GET_SPEED_MSG_REQ:
		return sizeof(EncoderGetSpeedMsgReq);
	case ENCODER_GET_SPEED_MSG_RSP:
		return sizeof(EncoderGetSpeedMsgRsp);
	case ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_REQ:
		return sizeof(EncoderNotifyAfterRotationsMsgReq);
	case ENCODER_NOTIFY_AFTER_ROTATIONS_MSG_RSP:
		return sizeof(EncoderNotifyAfterRotationsMsgRsp);
	case ENCODER_NOTIFY_AFTER_SPEED_MSG_REQ:
		return sizeof(EncoderNotifyAfterSpeedMsgReq);
	case ENCODER_NOTIFY_AFTER_SPEED_MSG_RSP:
		return sizeof(EncoderNotifyAfterSpeedMsgRsp);
	case WHEEL_SET_SPEED_MSG_REQ:
		return sizeof(WheelSetSpeedMsgReq);
	case WHEEL_SET_SPEED_MSG_RSP:
		return sizeof(WheelSetSpeedMsgRsp);
	case WHEEL_GET_SPEED_MSG_REQ:
		return sizeof(WheelGetSpeedMsgReq);
	case WHEEL_GET_SPEED_MSG_RSP:
		return sizeof(WheelGetSpeedMsgRsp);
	case WHEEL_SET_ACCELERATION_MSG_REQ:
		return sizeof(WheelSetAccelerationMsgReq);
	case WHEEL_SET_ACCELERATION_MSG_RSP:
		return sizeof(WheelSetAccelerationMsgRsp);
	case WHEEL_GET_ACCELERATION_MSG_REQ:
		return sizeof(WheelGetAccelerationMsgReq);
	case WHEEL_GET_ACCELERATION_MSG_RSP:
		return sizeof(WheelGetAccelerationMsgRsp);
	case MOTOR_START_MSG_REQ:
		return sizeof(MotorStartMsgReq);
	case MOTOR_START_MSG_RSP:
		return sizeof(MotorStartMsgRsp);
	case MOTOR_STOP_MSG_REQ:
		return sizeof(MotorStopMsgReq);
	case MOTOR_STOP_MSG_RSP:
		return sizeof(MotorStopMsgRsp);
	case GET_LOGS_MSG_REQ:
		return sizeof(GetLogsMsgReq);
	case GET_LOGS_MSG_RSP:
		return sizeof(GetLogsMsgRsp);
	default:
		return 0;
	}
}
