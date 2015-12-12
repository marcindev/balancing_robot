
IPATH=../messages

all:
	if ! [ -d "./out" ]; then mkdir out; fi 
	g++ main.cpp ../messages/messages.c -std=c++11 -g -o ./out/robot
	
clean:
	rm -rf ./out