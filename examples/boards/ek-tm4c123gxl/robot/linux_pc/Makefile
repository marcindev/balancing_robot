
IPATH=../messages

all:
	if ! [ -d "./out" ]; then mkdir out; fi 
	g++ main.cpp \
	../messages/messages.c \
	message.cpp \
	connection.cpp \
	command.cpp \
	commandFactory.cpp \
	getLogsCommand.cpp \
	-std=c++11 -g -o ./out/robot -lpthread
	
clean:
	rm -rf ./out