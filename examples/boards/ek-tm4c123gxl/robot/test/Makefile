
#CMOCK=$(shell readlink -f ./cmock)
CWD=$(shell pwd)


# production code search paths
VPATH=..
VPATH+=../drivers
VPATH+=../drivers/i2c
VPATH+=../drivers/spi
VPATH+=../utils
VPATH+=../robot
VPATH+=../messages
VPATH+=../freertos
VPATH+=../logger
VPATH+=../server
VPATH+=../interrupts
VPATH+=../updater
VPATH+=../config
VPATH+=../watchdog

# test code search paths
VPATH+=.
VPATH+=./mocks
VPATH+=./helpers
VPATH+=./tests/utils

# includes for production code
IPATH=../..
IPATH+=..
IPATH+=../../../../..
IPATH+=../../common
IPATH+=../../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source/portable/GCC/ARM_CM4F
IPATH+=../../../../../third_party/FreeRTOSV8.2.1/FreeRTOS
IPATH+=../../../../../third_party/FreeRTOSV8.2.1/FreeRTOS/Source/include
IPATH+=../../../../../third_party
IPATH+=../../../../../cc3100
IPATH+=../../../../../cc3100/simplelink
IPATH+=../../../../../cc3100/simplelink/include
IPATH+=../../../../../cc3100/platform/tiva-c-launchpad
IPATH+=../../../../../cc3100/oslib
IPATH+=../../../../../utils/
IPATH+=../drivers
IPATH+=../drivers/i2c
IPATH+=../drivers/spi
IPATH+=../utils
IPATH+=../messages
IPATH+=../robot
IPATH+=../freertos
IPATH+=../logger
IPATH+=../server
IPATH+=../interrupts
IPATH+=../updater
IPATH+=../config
IPATH+=../watchdog

#includes for test code
IPATH+=.
IPATH+=./mocks
IPATH+=./helpers
IPATH+=./cmock/include

#modules source files to test
SRCPRODC=stateMachine.c
SRCPRODC+=graph.c 
SRCPRODC+=linkedList.c
#test source files
#SRCTEST = $(notdir $(shell find $(CWD) -name '*.cpp'))
#SRCTEST= $(shell find $(CWD) -name '*.cpp')
#SRCTEST+=$(notdir $(SRCTESTDIR))
SRCTEST=$(wildcard *.cpp)
SRCTEST+=rtosMock.cpp
SRCTEST+=graphMock.cpp
SRCTEST+=linkedListMock.cpp
SRCTEST+=stateMachineMock.cpp
SRCTEST+=stateMachineTest.cpp
SRCTEST+=graphTest.cpp
SRCTEST+=notTestedMocked.cpp
SRCTEST+=linkedListTest.cpp

MOCKDIR = ./mocks
WRAPPREFIX = -Wl,--wrap,
WRAPFUNC = $(shell sed -n 's/\(.*\)\(__wrap_\)\(.*\)(.*).*/\3/p' $(MOCKDIR)/*.cpp)
MOCKFLAGS = $(addprefix $(WRAPPREFIX), $(WRAPFUNC))
INCPREFIX = -I
INCLUDES = $(addprefix $(INCPREFIX), $(IPATH))
CXXFLAGS = -Wall -std=c++11 -g # -D_UT
CFLAGS = -Wall -g 
#CFLAGS+=$(shell $(CMOCK)/bin/cmock-config --cflags)
#LDFLAGS=$(shell $(CMOCK)/bin/cmock-config --libs $(GMOCK) $(GTEST))
LDFLAGS=$(MOCKFLAGS)
LIBS = -lpthread -lgmock #-L$(CWD)/$(OBJDIR) -lprod -lgtest 
OBJDIR = obj
LIBDIR = lib
OUTDIR = out
BINARY = $(OUTDIR)/allTests
SOURCES := $(SRCTEST) $(SRCPRODC)
OBJECTSCPP := $(addprefix $(OBJDIR)/, $(SRCTEST:%.cpp=%.o))
OBJECTSC := $(addprefix $(OBJDIR)/, $(SRCPRODC:%.c=%.o))
DEPS 	:= $(addprefix $(OBJDIR)/, $(SRCTEST:%.cpp=%.d))
DEPS 	+= $(addprefix $(OBJDIR)/, $(SOURCES:%.c=%.d))


.PHONY: all clean setup run

run: $(BINARY)
	$(BINARY) $(RUN_OPT)
	

all: $(BINARY)

$(BINARY): $(OBJECTSCPP) # $(OBJDIR)/libprod.a
	mkdir -p $(OUTDIR)
	$(CXX) $(LDFLAGS) -o $(BINARY) $(OBJECTSCPP) $(LIBS) 
#$(OBJDIR)/libprod.a: $(OBJECTSC)
#	mkdir -p $(OBJDIR)
#	ar -rv $@ $^
#
$(OBJDIR)/%.o: %.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES)  -c -MMD -MP $< -o $@

#$(OBJDIR)/%.o: %.c
#	mkdir -p $(OBJDIR)
#	$(CC) $(CFLAGS) $(INCLUDES) -c -MMD -MP $< -o $@
#
clean:
	rm $(BINARY) $(OBJECTSC) $(OBJECTSCPP) $(DEPS) $(OBJDIR)/libprod.so

#$(BINARY): $(OBJECTSCPP) $(OBJDIR)/libprod.so
#	mkdir -p $(OUTDIR)
#	$(CXX) -o $(BINARY) $(OBJECTSCPP) $(LDFLAGS) $(LIBS) 
#
#$(OBJDIR)/libprod.so: $(OBJECTSC)
#	mkdir -p $(OBJDIR)
#	$(CC) -shared -Wl,-soname,$(CWD)/$@ -o $@ $^
#
#$(OBJDIR)/%.o: %.cpp
#	mkdir -p $(OBJDIR)
#	$(CXX) $(CXXFLAGS) $(INCLUDES)  -c -MMD -MP $< -o $@
#
#$(OBJDIR)/%.o: %.c
#	mkdir -p $(OBJDIR)
#	$(CC) $(CFLAGS) $(INCLUDES) -c -fPIC -MMD -MP $< -o $@
#
#clean:
#	rm $(BINARY) $(OBJECTSC) $(OBJECTSCPP) $(DEPS) $(OBJDIR)/libprod.so
#

-include $(DEPS)




