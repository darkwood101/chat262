server: server.o
	g++ -std=c++17 -O2 -Wall -Wextra -Wshadow -Werror -o server server.cc -pthread
