all: server client

CFLAGS := -Wall -Wextra -Werror -Wshadow -O2 -std=c++17
LDFLAGS := -pthread

server: server.o chat262_protocol.o database.o logger.o common.o
	g++ $(LDFLAGS) -o $@ $^

client: client.o chat262_protocol.o interface.o
	g++ $(LDFLAGS) -o $@ $^

%.o: %.cc
	g++ $(CFLAGS) -c -o $@ $<

clean:
	$(RM) *.o client server

.PHONY: all clean
