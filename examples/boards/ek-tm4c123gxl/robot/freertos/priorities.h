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

#define PRIORITY_I2C_TASK  		  					1
#define PRIORITY_MOTORS_TASK						2
#define PRIORITY_ROBOT_TASK							3
#define PRIORITY_TCP_SERVER_HANDLER_TASK			7
#define PRIORITY_TCP_SERVER_TASK					5


#endif // PRIORITIES_H
