CPP = g++
CFLAGS = -std=c++14 -Wall -Wextra -Wpedantic -Wunused-variable -fpic
OPTIMFLAGS = -O3 -O2
OBJFLAG = -c

SRCS = $(wildcard ./src/*.cpp)

build: server tcp_client udp_client

server: ./src/server*.cpp
	$(CPP) $(CFLAGS) $(OPTIMFLAGS) $(OBJFLAG) ./src/*.cpp
	$(CPP) server*.o -o server

tcp_client: ./src/tcp_client*.cpp
	$(CPP) $(CFLAGS) $(OPTIMFLAGS) $(OBJFLAG) ./src/*.cpp
	$(CPP) tcp_client*.o -o tcp_client

udp_client: ./src/udp_client*.cpp
	$(CPP) $(CFLAGS) $(OPTIMFLAGS) $(OBJFLAG) ./src/*.cpp
	$(CPP) udp_client*.o -o udp_client

OBJS = $(wildcard *.o)
BINS = server tcp_client udp_client server_helpers tcp_client_helpers udp_client_helpers

clean:
	rm -f $(OBJS)
	rm -f $(BINS)