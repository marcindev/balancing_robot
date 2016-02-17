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
ROOT=../../../../..

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#

VPATH+=../../../../../utils/
VPATH+=./../utils
VPATH+=./../config
VPATH+=../../../../..



#
# Where to find header files that do not live in the source directory.
#
IPATH=.

IPATH+=./../utils
IPATH+=./../config
IPATH+=../../../../..

#
# The default rule, which causes the CC3000 Basic WiFi Application to be built.
#
all: ${COMPILER}
all: ${COMPILER}/bl_main.axf

master: ${COMPILER}
master: ${COMPILER}/bl_main.axf


slave: ${COMPILER}
slave: ${COMPILER}/bl_main.axf

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
# Rules for building bootloader
#

${COMPILER}/bl_main.axf: ${COMPILER}/bl_main.o
${COMPILER}/bl_main.axf: ${COMPILER}/config.o
${COMPILER}/bl_main.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/bl_main.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/bl_main.axf: bl_main.ld
SCATTERgcc_bl_main=bl_main.ld
ENTRY_bl_main=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C123_RB1  -g -O0

#CFLAGSgcc=-DTARGET_IS_TM4C123_RB1 -D_USE_CLI_ -D_DISABLE_I2C_ACK -g

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
