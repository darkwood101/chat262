#ifndef _SERVER_H_
#define _SERVER_H_

#include "chat262_protocol.h"
#include "common.h"
#include "database.h"
#include "user.h"

#include <cstdint>
#include <netinet/in.h>
#include <string>

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

    // Send the message `msg` to `client_fd`.
    // @return ok    - success
    // @return error - fatal error in sending the message.
    //                 `errno` is appropriately set.
    status send_msg(int client_fd, std::shared_ptr<chat262::message> msg) const;

    // Receive a message header from `client_fd` into `hdr`.
    // @return ok     - success
    // @return error  - fatal error in receiving the header.
    //                  `errno` is appropriately set.
    // @return closed - the client closed the connection before
    //                  the header was received
    status recv_hdr(int client_fd, chat262::message_header& hdr) const;

    // Receive a message body of length `body_len` from the `client_fd` into
    // `data`. `body_len` should be chosen depending on the header that was
    // received first.
    // @return ok    - success
    // @return error - fatal error in receiving the body.
    //                 `errno` is appropriately set.
    status recv_body(int client_fd,
                     uint32_t body_len,
                     std::vector<uint8_t>& data) const;

    status handle_registration(int client_fd,
                               const std::vector<uint8_t>& body_data);
    status handle_login(int client_fd, const std::vector<uint8_t>& body_data);
    status handle_list_accounts(int client_fd,
                                const std::vector<uint8_t>& body_data);

    // The users database
    database database_;

    // Listen socket file descriptor
    int server_fd_;

    // IP address in network byte order
    uint32_t n_ip_addr_;

    // IP address in string format
    std::string str_ip_addr_;
};

#endif
