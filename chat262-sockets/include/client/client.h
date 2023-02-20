#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "chat.h"
#include "chat262_protocol.h"
#include "common.h"

#include <string>

class client {
public:
    // Connect to a Chat 262 server.
    // @return ok           - The client successfully connected to the server.
    // @return error        - There was an error. A debug output is printed to
    //                        stderr.
    // @param[in] n_ip_addr - The IP address on which the server is listening.
    status connect_server(const uint32_t n_ip_addr);

    // Send a login request to the server and read the response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[in]  username      - Username for the login request.
    // @param[in]  password      - Password for the login request.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    status login(const std::string& username,
                 const std::string& password,
                 uint32_t& stat_code);

    // Send a registration request to the server and read the response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed. The status code from the server is
    //                             stored in `stat_code`.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[in]  username      - Username for the registration request.
    // @param[in]  password      - Password for the registration request.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    status registration(const std::string& username,
                        const std::string& password,
                        uint32_t& stat_code);

    // Send a logout request to the server and read the response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    status logout(uint32_t& stat_code);

    // Send a search accounts request to the server, with matching pattern
    // `pattern`.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed. The status code from the server is
    //                             stored in `stat_code` and the list of matched
    //                             usernames is stored in `usernames`.
    //                             `usernames` is ignored if the status code is
    //                             not OK.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[in] pattern        - The matching pattern for searching accounts.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    // @param[out] usernames     - Stores the matched usernames. This parameter
    //                             is ignored unless the return value is
    //                             `status::ok` and `stat_code` is OK (0).
    status list_accounts(const std::string& pattern,
                         uint32_t& stat_code,
                         std::vector<std::string>& usernames);

    // Send a send text request to the server and read the response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[in] recipient      - The username of the text recipient.
    // @param[in] txt            - The text to send to the recipient.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    status send_txt(const std::string& recipient,
                    const std::string& txt,
                    uint32_t& stat_code);

    // Send a receive texts request to the server and read the response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[in] sender         - The username of the user to retrieve the
    //                             texts from.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    // @param[out] c             - Stores the retrieved texts. This parameter
    //                             is ignored unless the return value is
    //                             `status::ok` and `stat_code` is OK (0).
    status recv_txt(const std::string& sender, uint32_t& stat_code, chat& c);

    // Send a receive correspondents request to the server and read the
    // response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    // @param[out] correspondents- Stores the list of correspondents. This
    //                             parameter is ignored unless the return value
    //                             is `status::ok` and `stat_code` is OK (0).
    status recv_correspondents(uint32_t& stat_code,
                               std::vector<std::string>& correspondents);

    // Send a delete account request to the server and read the response.
    // @return ok                - The request was successfully sent, and the
    //                             response was successfully received and
    //                             parsed.
    // @return send_error        - There was an error in sending the request.
    // @return receive_error     - There was an error in receiving the header or
    //                             the body.
    // @return closed_connection - The server closed the connection.
    // @return body_error        - The server sent an improperly formed response
    //                             body.
    // @param[out] stat_code     - Stores the status code received from the
    //                             server. This parameter is ignored unless the
    //                             return value is `status::ok`.
    status delete_account(uint32_t& stat_code);

private:
    // Send the message `msg` to the server.
    // @return ok         - The message was successfully sent
    // @return send_error - The send failed. This is possibly due to a closed
    //                      connection.
    status send_msg(std::shared_ptr<chat262::message> msg) const;

    // Receive a message header from the server into `hdr`.
    // @return ok                - The header was successfully read.
    // @return receive_error     - The read failed.
    // @return closed_connection - The server closed the connection.
    status recv_hdr(chat262::message_header& hdr) const;

    // Receive a message body of length `body_len` from the server into `data`.
    // `body_len` should be chosen depending on the header that was received
    // first.
    // @return ok                - The body was successfully read.
    // @return receive_error     - The read failed.
    // @return closed_connection - The server closed the connection.
    status recv_body(uint32_t body_len, std::vector<uint8_t>& data) const;

    // Check that `hdr` matches the Chat 262 Protocol format of a message
    // specified by message type `expected`.
    // @return ok           - The header is correctly formed.
    // @return header_error - There is an error in the header, which is
    //                        either a version mismatch or a type mismatch.
    status validate_hdr(const chat262::message_header& hdr,
                        const chat262::message_type& expected) const;

    // Connected socket file descriptor
    int server_fd_;

    // IP address in network byte order
    uint32_t n_ip_addr_;

    // IP address in string format
    std::string str_ip_addr_;
};

#endif
