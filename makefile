all: server client

CFLAGS := -Wall -Wextra -Werror -O2 -std=c++17
LDFLAGS := -pthread

server: server.o chat262_protocol.o logger.o common.o
	g++ $(LDFLAGS) -o $@ $^

client: client.o chat262_protocol.o common.o
	g++ $(LDFLAGS) -o $@ $^

%.o: %.cc
	g++ $(CFLAGS) -c -o $@ $<

clean:
	$(RM) *.o client server

.PHONY: all clean
