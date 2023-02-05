#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "chat262_protocol.h"
#include "common.h"
#include "user.h"

#include <iostream>
#include <string>

class client {
public:
    // Run the client with command-line arguments
    status run(int argc, char const* const* argv);

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

    template <typename T>
    T get_user_unsigned();

    std::string get_user_string(size_t min_len, size_t max_len);

    status connect_server();

    void send_msg(std::shared_ptr<chat262::message> msg) const;

    void start_ui();
    void login();
    void registration();

    user this_user_;

    // Connected socket file descriptor
    int server_fd_;
    // IP address in network byte order
    uint32_t n_ip_addr_;
    // IP address in string format
    std::string str_ip_addr_;
};

template <typename T>
T client::get_user_unsigned() {
    std::string line;
    T num;
    while (true) {
        std::cout << "Chat262> " << std::flush;
        std::getline(std::cin, line);
        if (std::cin.eof()) {
            std::cout << "\n";
            std::cin.clear();
            clearerr(stdin);
        }
        if (str_to_unsigned<T>(line, num) == status::ok) {
            break;
        }
    }
    return num;
}

#endif
