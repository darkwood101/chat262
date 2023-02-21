#include "common.h"
#include "interface.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

static void usage(const char* prog) {
    std::cerr << "usage: " << prog
              << " [-h] <ip address>\n"
                 "\n"
                 "Start the Chat262 client and connect to a Chat262 server on "
                 "IP address\n"
                 "<ip address>. The address should be in the xxx.xxx.xxx.xxx "
                 "format.\n"
                 "\n"
                 "Options:\n"
                 "\t-h\t\t Display this message and exit.\n";
}

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        std::cout << "Wrong number of arguments\n";
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Look for "-h"
    if (strcmp(argv[1], "-h") == 0 ||
        (argc == 3 && strcmp(argv[2], "-h") == 0)) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    } else if (argc == 3) {
        std::cout << "Unknown argument\n";
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    uint32_t n_ip_addr;
    // Parse the IP address
    if (inet_pton(AF_INET, argv[1], &n_ip_addr) != 1) {
        std::cout << "Invalid IP address\n";
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    interface i;
    if (i.start(n_ip_addr) == status::ok) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}