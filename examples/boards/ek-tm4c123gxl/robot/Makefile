#******************************************************************************
#
# Makefile - Rules for building the CC3100 Basic WiFi Application.
#
# Copyright (c) 2014 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 2.1.0.12573 of the EK-TM4C123GXL Firmware Package.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=TM4C123GH6PM

#
# The base directory for TivaWare.
#
ROOT=../../../..

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source/portable/GCC/ARM_CM4F
VPATH+=../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source/portable/MemMang/
VPATH+=../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source
VPATH+=../../../../cc3100
VPATH+=../../../../cc3100/simplelink
VPATH+=../../../../cc3100/simplelink/source
VPATH+=../../../../cc3100/platform/tiva-c-launchpad
VPATH+=./drivers
VPATH+=./drivers/i2c
VPATH+=./utils
VPATH+=./robot
VPATH+=./messages
VPATH+=./freertos
VPATH+=./logger
VPATH+=./server
VPATH+=./interrupts

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=..
IPATH+=../../../..
IPATH+=../common
IPATH+=../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source/portable/GCC/ARM_CM4F
IPATH+=../../../../third_party/FreeRTOSV8.2.1/FreeRTOS
IPATH+=../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source/include
IPATH+=../../../../third_party
IPATH+=../../../../cc3100
IPATH+=../../../../cc3100/simplelink
IPATH+=../../../../cc3100/simplelink/include
IPATH+=../../../../cc3100/platform/tiva-c-launchpad
IPATH+=./drivers
IPATH+=./drivers/i2c
IPATH+=./utils
IPATH+=./messages
IPATH+=./robot
IPATH+=./freertos
IPATH+=./logger
IPATH+=./server
IPATH+=./interrupts

#
# The default rule, which causes the CC3000 Basic WiFi Application to be built.
#
all: ${COMPILER}
all: ${COMPILER}/robot_main.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the CC3100 Basic WiFi Application.
#
${COMPILER}/robot_main.axf: ${COMPILER}/robot_main.o
${COMPILER}/robot_main.axf: ${COMPILER}/robot.o
${COMPILER}/robot_main.axf: ${COMPILER}/board.o
${COMPILER}/robot_main.axf: ${COMPILER}/spi.o
${COMPILER}/robot_main.axf: ${COMPILER}/uart.o
${COMPILER}/robot_main.axf: ${COMPILER}/device.o
${COMPILER}/robot_main.axf: ${COMPILER}/driver.o
${COMPILER}/robot_main.axf: ${COMPILER}/flowcont.o
${COMPILER}/robot_main.axf: ${COMPILER}/fs.o
${COMPILER}/robot_main.axf: ${COMPILER}/netapp.o
${COMPILER}/robot_main.axf: ${COMPILER}/netcfg.o
${COMPILER}/robot_main.axf: ${COMPILER}/nonos.o
${COMPILER}/robot_main.axf: ${COMPILER}/socket.o
${COMPILER}/robot_main.axf: ${COMPILER}/spawn.o
${COMPILER}/robot_main.axf: ${COMPILER}/wlan.o
${COMPILER}/robot_main.axf: ${COMPILER}/I2CWrapper.o
${COMPILER}/robot_main.axf: ${COMPILER}/MCP23017.o
${COMPILER}/robot_main.axf: ${COMPILER}/i2cManager.o
${COMPILER}/robot_main.axf: ${COMPILER}/i2cTask.o
${COMPILER}/robot_main.axf: ${COMPILER}/motor.o
${COMPILER}/robot_main.axf: ${COMPILER}/motorsTask.o
${COMPILER}/robot_main.axf: ${COMPILER}/i2cTask.o
${COMPILER}/robot_main.axf: ${COMPILER}/utils.o
${COMPILER}/robot_main.axf: ${COMPILER}/heap_5.o
${COMPILER}/robot_main.axf: ${COMPILER}/list.o
${COMPILER}/robot_main.axf: ${COMPILER}/port.o
${COMPILER}/robot_main.axf: ${COMPILER}/queue.o
${COMPILER}/robot_main.axf: ${COMPILER}/tasks.o
${COMPILER}/robot_main.axf: ${COMPILER}/messages.o
${COMPILER}/robot_main.axf: ${COMPILER}/logger.o
${COMPILER}/robot_main.axf: ${COMPILER}/SimpleLinkWrapper.o
${COMPILER}/robot_main.axf: ${COMPILER}/tcpServer.o
${COMPILER}/robot_main.axf: ${COMPILER}/tcpServerTask.o
${COMPILER}/robot_main.axf: ${COMPILER}/tcpServerHandlerTask.o
${COMPILER}/robot_main.axf: ${COMPILER}/encoder.o
${COMPILER}/robot_main.axf: ${COMPILER}/interrupts.o
${COMPILER}/robot_main.axf: ${COMPILER}/encoderSamplerTask.o
${COMPILER}/robot_main.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/robot_main.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/robot_main.axf: robot_main.ld
SCATTERgcc_robot_main=robot_main.ld
ENTRY_robot_main=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C123_RB1 -g -O0
#CFLAGSgcc=-DTARGET_IS_TM4C123_RB1 -D_USE_CLI_ -D_DISABLE_I2C_ACK -g

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
