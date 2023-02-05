#ifndef _SERVER_H_
#define _SERVER_H_

#include "chat262_protocol.h"
#include "common.h"
#include "user.h"

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <unordered_map>

class server {
public:
    server();
    ~server();

    server(const server&) = delete;
    server(server&&) = delete;
    server& operator=(const server&) = delete;
    server& operator=(server&&) = delete;

    // Run the server with command-line arguments
    status run(const int argc, char const* const* argv);

private:
    // Command-line arguments
    struct cmdline_args {
        bool help_;
        uint32_t n_ip_addr_;
    };
    // Parse command-line arguments (`argc`, `argv`).
    // Returns the filled `cmdline_args` structure on success.
    // Throws `std::invalid_argument` exception on error.
    cmdline_args parse_args(const int argc, char const* const* argv) const;

    // Print usage information to standard error
    void usage(char const* prog) const;

    // Open the server socket for incoming connections
    status start_listening();

    // Forever accept incoming connections
    __attribute__((noreturn)) void start_accepting();

    // Handle the accepted connection. Runs in a separate thread.
    void handle_client(int client_fd, sockaddr_in client_addr);

    chat262::message_header recv_hdr(int client_fd);
    std::vector<uint8_t> recv_body(int client_fd, uint32_t body_len);

    void handle_registration(const std::vector<uint8_t>& body_data);
    void handle_login(const std::vector<uint8_t>& body_data);

    std::unordered_map<std::string, user> users_;

    // Listen socket file descriptor
    int server_fd_;

    // IP address in network byte order
    uint32_t n_ip_addr_;

    // IP address in string format
    std::string str_ip_addr_;
};

#endif
