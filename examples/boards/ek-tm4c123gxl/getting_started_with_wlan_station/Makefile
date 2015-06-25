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
VPATH=../../../../cc3100
VPATH+=../../../../cc3100/simplelink
VPATH+=../../../../cc3100/simplelink/source
VPATH+=../../../../cc3100/platform/tiva-c-launchpad

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../../../..
IPATH+=../common
IPATH+=../../../../cc3100
IPATH+=../../../../cc3100/simplelink
IPATH+=../../../../cc3100/simplelink/include
IPATH+=../../../../cc3100/platform/tiva-c-launchpad


#
# The default rule, which causes the CC3000 Basic WiFi Application to be built.
#
all: ${COMPILER}
all: ${COMPILER}/getting_started_with_wlan_station.axf

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
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/board.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/cli_uart.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/spi.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/uart.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/device.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/driver.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/flowcont.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/fs.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/netapp.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/netcfg.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/nonos.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/socket.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/spawn.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/wlan.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/getting_started_with_wlan_station.o
${COMPILER}/getting_started_with_wlan_station.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/getting_started_with_wlan_station.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/getting_started_with_wlan_station.axf: getting_started_with_wlan_station.ld
SCATTERgcc_getting_started_with_wlan_station=getting_started_with_wlan_station.ld
ENTRY_getting_started_with_wlan_station=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C123_RB1 -D_USE_CLI_ -g

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
