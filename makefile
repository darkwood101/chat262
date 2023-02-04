all: server client

CFLAGS := -Wall -Wextra -Werror -O2 -std=c++17
LDFLAGS := -pthread

server: server.o logger.o
	g++ $(LDFLAGS) -o $@ $^

client: client.o
	g++ $(LDFLAGS) -o $@ $^

%.o: %.cc
	g++ $(CFLAGS) -c -o $@ $<

clean:
	$(RM) *.o client server

.PHONY: all clean
