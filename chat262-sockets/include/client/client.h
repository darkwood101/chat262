#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "chat.h"
#include "chat262_protocol.h"
#include "common.h"

#include <string>

class client {
public:
    status connect_server(const uint32_t n_ip_addr);

    status login(const std::string& username,
                 const std::string& password,
                 uint32_t& stat_code);
    status registration(const std::string& username,
                        const std::string& password,
                        uint32_t& stat_code);
    status logout(uint32_t& stat_code);
    status list_accounts(const std::string& pattern,
                         uint32_t& stat_code,
                         std::vector<std::string>& usernames);
    status send_txt(const std::string& recipient,
                    const std::string& txt,
                    uint32_t& stat_code);
    status recv_txt(const std::string& sender, uint32_t& stat_code, chat& c);
    status recv_correspondents(uint32_t& stat_code,
                               std::vector<std::string>& correspondents);
    status delete_account(uint32_t& stat_code);

private:
    // Send the message `msg` to the server.
    // Throws `std::runtime_error` if the message cannot be sent (fatal).
    status send_msg(std::shared_ptr<chat262::message> msg) const;

    // Receive a message header from the server into `hdr`.
    // Throws `std::runtime_error` if the header cannot be received (fatal).
    status recv_hdr(chat262::message_header& hdr) const;

    // Receive a message body of length `body_len` from the server into `data`.
    // `body_len` should be chosen depending on the header that was received
    // first.
    // Throws `std::runtime_error` if the body cannot be received (fatal).
    status recv_body(uint32_t body_len, std::vector<uint8_t>& data) const;

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
