CC=g++
CFLAGS = -Wall -std=c++11 -g
LIBS = -lpthread -lreadline
HEADERDIR = ../messages
OBJDIR = obj
LIBDIR = lib
OUTDIR = out
BINARY = $(OUTDIR)/robot
SOURCES := $(wildcard *.cpp)
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES:%.cpp=%.o))
SOURCES += ../messages/messages.c
OBJECTS += $(OBJDIR)/messages.o
DEPS 	:= $(addprefix $(OBJDIR)/,$(SOURCES:%.cpp=%.d))

.PHONY: all clean setup


all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BINARY) $(LIBS) 

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -I$(HEADERDIR) -I$(dir $<) -c -MMD -MP $< -o $@

$(OBJDIR)/messages.o: ../messages/messages.c
	$(CC) $(CFLAGS) -I$(HEADERDIR) -I$(dir $<) -c $< -o $@

library:
	ar cr $(LIBDIR)/librobot.a $(OBJECTS)
	
clean:
	rm $(BINARY) $(OBJECTS) $(DEPS)
	
-include $(DEPS)
	