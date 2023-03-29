#ifndef _SERVER_H_
#define _SERVER_H_

#include "chat262_protocol.h"
#include "common.h"
#include "database.h"

#include <cstdint>
#include <netinet/in.h>
#include <string>

class server {
public:
    server();
    ~server();

    // Prevent copy/move
    server(const server&) = delete;
    server(server&&) = delete;
    server& operator=(const server&) = delete;
    server& operator=(server&&) = delete;

    // Run the server with command-line arguments.
    // This function never returns if the server starts successfully.
    // @return ok      - "-h" was supplied as a command-line argument
    // @return error   - Invalid command-line arguments.
    // @param[in] argc - Number of command-line arguments
    // @param[in] argv - List of command-line arguments
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

    // Send the message `msg` to the `client_fd`.
    // @return ok         - The message was successfully sent
    // @return send_error - The send failed. This is possibly due to a closed
    //                      connection.
    status send_msg(int client_fd, std::shared_ptr<chat262::message> msg) const;

    // Receive a message header from `client_fd` into `hdr`.
    // @return ok                - The header was successfully read.
    // @return receive_error     - The read failed.
    // @return closed_connection - The client closed the connection.
    status recv_hdr(int client_fd, chat262::message_header& hdr) const;

    // Receive a message body of length `body_len` from `client_fd` into `data`.
    // `body_len` should be chosen depending on the header that was received
    // first.
    // @return ok                - The body was successfully read.
    // @return receive_error     - The read failed.
    // @return closed_connection - The server closed the connection.
    status recv_body(int client_fd,
                     uint32_t body_len,
                     std::vector<uint8_t>& data) const;

    // Handle a registration request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_registration(int client_fd,
                               const std::vector<uint8_t>& body_data);

    // Handle a login request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_login(int client_fd, const std::vector<uint8_t>& body_data);

    // Handle a logout request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_logout(int client_fd, const std::vector<uint8_t>& body_data);

    // Handle a search accounts request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_list_accounts(int client_fd,
                                const std::vector<uint8_t>& body_data);

    // Handle a send text request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_send_txt(int client_fd,
                           const std::vector<uint8_t>& body_data);

    // Handle a receive text request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_recv_txt(int client_fd,
                           const std::vector<uint8_t>& body_data);

    // Handle a retrieve correspondents request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_correspondents(int client_fd,
                                 const std::vector<uint8_t>& body_data);

    // Handle a delete account request and respond to the client.
    // @return ok           - The request was successfully parsed, and the
    //                        response was successfully sent.
    // @return body_error   - The client sent an improperly formed request
    //                        body.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    // @param[in] body_data - The bytes making up the request body.
    status handle_delete(int client_fd, const std::vector<uint8_t>& body_data);

    // Send a wrong version response to the client.
    // @return ok           - The response was successfully sent.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    status handle_wrong_version(int client_fd);

    // Send an invalid type response to the client.
    // @return ok           - The response was successfully sent.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    status handle_invalid_type(int client_fd);

    // Send an invalid body response to the client.
    // @return ok           - The response was successfully sent.
    // @return send_error   - There was an error in sending the response.
    // @param[in] client_fd - The socket descriptor for the client connection.
    status handle_invalid_body(int client_fd);

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
