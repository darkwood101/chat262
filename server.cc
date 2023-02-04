#include "server.h"

#include "chat262_protocol.h"
#include "logger.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

server::server() : server_fd_(-1), n_ip_addr_(0) {
}

server::~server() {
    // If the socket descriptor was open, close it
    if (server_fd_ != -1) {
        close(server_fd_);
    }
}

status server::run(int argc, char const* const* argv) {
    cmdline_args args;
    try {
        args = parse_args(argc, argv);
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        usage(argv[0]);
        return status::error;
    }

    if (args.help_) {
        usage(argv[0]);
        return status::ok;
    }

    n_ip_addr_ = args.n_ip_addr_;
    str_ip_addr_ = argv[1];

    try {
        start_listening();
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        return status::error;
    }

    start_accepting();

    return status::ok;
}

server::cmdline_args server::parse_args(const int argc,
                                        char const* const* argv) const {
    if (argc != 2 && argc != 3) {
        throw std::invalid_argument("Wrong number of arguments");
    }

    cmdline_args args;
    // Look for "-h"
    if (strcmp(argv[1], "-h") == 0 ||
        (argc == 3 && strcmp(argv[2], "-h") == 0)) {
        args.help_ = true;
        return args;
    } else {
        args.help_ = false;
    }
    // Parse the IP address
    if (inet_pton(AF_INET, argv[1], &(args.n_ip_addr_)) != 1) {
        throw std::invalid_argument("Invalid IP address");
    }
    return args;
}

void server::usage(char const* prog) const {
    std::cerr << "usage: " << prog
              << " [-h] <ip address>\n"
                 "\n"
                 "Start the Chat262 server on IP address <ip address>.\n"
                 "The address should be in the xxx.xxx.xxx.xxx format.\n"
                 "\n"
                 "Options:\n"
                 "\t-h\t\t Display this message and exit.\n";
}

void server::start_listening() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error(std::string("Could not create socket: ") +
                                 std::string(strerror(errno)));
    }

    // Allow reuse of this address immediately. Otherwise, we might have to
    // wait.
    static constexpr int enable_addr_reuse = 1;
    if (setsockopt(server_fd_,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &enable_addr_reuse,
                   sizeof(enable_addr_reuse)) < 0) {
        throw std::runtime_error(
            std::string("Could not enable address reuse: ") +
            std::string(strerror(errno)));
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(chat262::port);
    server_addr.sin_addr.s_addr = n_ip_addr_;
    if (bind(server_fd_, (const sockaddr*) &server_addr, sizeof(server_addr)) <
        0) {
        throw std::runtime_error(std::string("Could not bind the socket: ") +
                                 std::string(strerror(errno)));
    }

    if (listen(server_fd_, 1) < 0) {
        throw std::runtime_error(
            std::string("Could not listen on the socket: ") +
            std::string(strerror(errno)));
    }
    logger::log_out("Listening on %s:%" PRIu16 "\n",
                    str_ip_addr_.c_str(),
                    chat262::port);
}

void server::start_accepting() {
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);

    while (true) {
        int client_fd =
            accept(server_fd_, (sockaddr*) &client_addr, &client_addr_len);
        std::thread t(&server::handle_client, this, client_fd, client_addr);
        t.detach();
    }
}

void server::handle_client(int client_fd, sockaddr_in client_addr) {
    // Make sure the connection was properly accepted
    if (client_fd < 0) {
        logger::log_err("Could not accept: %s\n", strerror(errno));
        return;
    }
    char client_ip[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET,
                   &client_addr.sin_addr,
                   client_ip,
                   sizeof(client_ip))) {
        logger::log_err("%s", "Could not read client's IP\n");
        return;
    }
    logger::log_out("Accepted connection from %s\n", client_ip);
}

int main(int argc, char** argv) {
    server s;
    if (s.run(argc, argv) == status::ok) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
