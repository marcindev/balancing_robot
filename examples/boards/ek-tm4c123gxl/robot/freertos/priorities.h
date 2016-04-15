//*****************************************************************************
//
// priorities.h - Priorities for the various FreeRTOS tasks.
//
//
//*****************************************************************************

#ifndef PRIORITIES_H
#define PRIORITIES_H

//*****************************************************************************
//
// The priorities of the various tasks.
//
//*****************************************************************************

#define PRIORITY_I2C_TASK  		  					5
#define PRIORITY_MOTORS_TASK						2
#define PRIORITY_ROBOT_TASK							1
#define PRIORITY_TCP_SERVER_HANDLER_TASK			1
#define PRIORITY_TCP_SERVER_TASK					1
#define PRIORITY_ENCODERS_TASK						3
#define PRIORITY_MPU_TASK							2
#define PRIORITY_ENCODER_SAMPLER_TASK				4
#define PRIORITY_MOTION_CONTROL_TASK				4
#define PRIORITY_WHEELS_TASK						2
#define PRIORITY_SIMPLE_LINK_TASK					6
#define PRIORITY_SERVER_SPI_TASK					1
#define PRIORITY_UPDATER_TASK						1


#endif // PRIORITIES_H
