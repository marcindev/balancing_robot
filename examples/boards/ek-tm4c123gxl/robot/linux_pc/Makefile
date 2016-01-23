CC=g++
CFLAGS = -Wall -std=c++11 -g
LIBS = -lpthread -lreadline
HEADERDIR = ../messages
OBJDIR = obj
OUTDIR = out
BINARY = $(OUTDIR)/robot
SOURCES := $(wildcard *.cpp)
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES:%.cpp=%.o))
SOURCES += ../messages/messages.c
OBJECTS += $(OBJDIR)/messages.o

.PHONY: all clean setup

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BINARY) $(LIBS) 

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@

$(OBJDIR)/messages.o: ../messages/messages.c
	$(CC) $(CFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@
	
clean:
	rm $(BINARY) $(OBJECTS)