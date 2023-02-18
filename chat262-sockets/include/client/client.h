#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "chat.h"
#include "chat262_protocol.h"
#include "common.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>

class client {
public:
    status connect_server(const uint32_t n_ip_addr);

    uint32_t login(const std::string& username, const std::string& password);
    uint32_t registration(const std::string& username,
                          const std::string& password);
    uint32_t logout();
    uint32_t list_accounts(const std::string& pattern,
                           std::vector<std::string>& usernames);
    uint32_t send_txt(const std::string& recipient, const std::string& txt);
    uint32_t recv_txt(const std::string& sender, chat& c);
    uint32_t recv_correspondents(std::vector<std::string>& correspondents);
    uint32_t delete_account();

#ifdef TESTING
    void test_registration();
    void test_login();
    void test_logout();
    void test_search_accounts();
    void test_send_txt();
#endif

#ifndef TESTING
private:
#else
public:
#endif
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

    void validate_hdr(const chat262::message_header& hdr,
                      const chat262::message_type& expected);

    // Connected socket file descriptor
    int server_fd_;
    // IP address in network byte order
    uint32_t n_ip_addr_;
    // IP address in string format
    std::string str_ip_addr_;
};

#endif
