#ifndef _SERVER_H_
#define _SERVER_H_

#include "common.h"

#include <cstdint>
#include <string>

class server {
public:
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

    void start_listening();

    // Listen socket file descriptor
    int socket_fd_;
    // IP address in network byte order
    uint32_t n_ip_addr_;
    // IP address in string format
    std::string str_ip_addr_;
};

#endif
