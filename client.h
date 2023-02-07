#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "chat262_protocol.h"
#include "common.h"
#include "interface.h"
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

    status connect_server();

    // Send the message `msg` to the server.
    // Throws `std::runtime_error` if the message cannot be sent (fatal).
    void send_msg(std::shared_ptr<chat262::message> msg) const;

    // Receive a message header from the server into `hdr`.
    // Throws `std::runtime_error` if the header cannot be received (fatal).
    void recv_hdr(chat262::message_header& hdr) const;

    // Receive a message body of length `body_len` from the server into `data`.
    // `body_len` should be chosen depending on the header that was received
    // first.
    // Throws `std::runtime_error` if the body cannot be received (fatal).
    void recv_body(uint32_t body_len, std::vector<uint8_t>& data) const;

    void start_ui();
    uint32_t login(const std::string& username, const std::string& password);
    uint32_t registration(const std::string& username,
                          const std::string& password);
    uint32_t logout();
    uint32_t list_accounts(std::vector<std::string>& usernames);

    interface interface_;

    user this_user_;

    // Connected socket file descriptor
    int server_fd_;
    // IP address in network byte order
    uint32_t n_ip_addr_;
    // IP address in string format
    std::string str_ip_addr_;
};

#endif
