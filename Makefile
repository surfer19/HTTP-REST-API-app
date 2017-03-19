CC=g++
RM=rm -vf
CPPFLAGS=-std=c++11 -Wall -pedantic -Wextra
.PHONY: all clean
all: client server
client: client.cpp
		${CC} ${CPPFLAGS} client.cpp -o ftrest
server: server.cpp
		${CC} ${CPPFLAGS} server.cpp -o ftrestd
clean:
		${RM} *.o
