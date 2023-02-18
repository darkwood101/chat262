#include "client.h"

#include "chat262_protocol.h"

#include <arpa/inet.h>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <limits>
#include <mutex>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

status client::connect_server(const uint32_t n_ip_addr) {
    n_ip_addr_ = n_ip_addr;
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Could not create a socket: " << strerror(errno) << "\n";
        return status::error;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(chat262::port);
    server_addr.sin_addr.s_addr = n_ip_addr_;

    if (connect(server_fd_,
                (const sockaddr*) &server_addr,
                sizeof(server_addr)) < 0) {
        std::cerr << "Could not connect to server: " << strerror(errno) << "\n";
        return status::error;
    }

    std::cout << "Successfully connected to server on " << str_ip_addr_ << ":"
              << chat262::port << "\n";
    return status::ok;
}

uint32_t client::login(const std::string& username,
                       const std::string& password) {
    auto msg = chat262::login_request::serialize(username, password);
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_login_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::login_response::deserialize(body, stat_code) != status::ok) {
        throw std::runtime_error(
            "Login response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::registration(const std::string& username,
                              const std::string& password) {
    auto msg = chat262::registration_request::serialize(username, password);
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_registration_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::registration_response::deserialize(body, stat_code) !=
        status::ok) {
        throw std::runtime_error(
            "Registration response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::logout() {
    auto msg = chat262::logout_request::serialize();
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_logout_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::logout_response::deserialize(body, stat_code) != status::ok) {
        throw std::runtime_error(
            "Logout response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::list_accounts(const std::string& pattern,
                               std::vector<std::string>& usernames) {
    auto msg = chat262::accounts_request::serialize(pattern);
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_accounts_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::accounts_response::deserialize(body, stat_code, usernames) !=
        status::ok) {
        throw std::runtime_error(
            "Accounts response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::send_txt(const std::string& recipient,
                          const std::string& txt) {
    auto msg = chat262::send_txt_request::serialize(recipient, txt);
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_send_txt_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::send_txt_response::deserialize(body, stat_code) !=
        status::ok) {
        throw std::runtime_error(
            "Send text response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::recv_txt(const std::string& sender, chat& c) {
    auto msg = chat262::recv_txt_request::serialize(sender);
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_recv_txt_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::recv_txt_response::deserialize(body, stat_code, c) !=
        status::ok) {
        throw std::runtime_error(
            "Receive text response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::recv_correspondents(std::vector<std::string>& correspondents) {
    auto msg = chat262::correspondents_request::serialize();
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_correspondents_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::correspondents_response::deserialize(body,
                                                      stat_code,
                                                      correspondents) !=
        status::ok) {
        throw std::runtime_error(
            "Receive text response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::delete_account() {
    auto msg = chat262::delete_request::serialize();
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    validate_hdr(msg_hdr, chat262::msgtype_delete_response);

    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::delete_response::deserialize(body, stat_code) != status::ok) {
        throw std::runtime_error(
            "Logout response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

void client::send_msg(std::shared_ptr<chat262::message> msg) const {
    size_t total_sent = 0;
    ssize_t sent = 0;
    size_t total_len = sizeof(chat262::message_header) + msg->hdr_.body_len_;
    while (total_sent != total_len) {
        sent = write(server_fd_, msg.get(), total_len);
        if (sent < 0) {
            throw std::runtime_error(std::string("Cannot send the message: ") +
                                     std::string(strerror(errno)));
        }
        total_sent += sent;
    }
}

void client::recv_hdr(chat262::message_header& hdr) const {
    std::vector<uint8_t> hdr_data;
    hdr_data.resize(sizeof(chat262::message_header));
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != sizeof(chat262::message_header)) {
        readed =
            read(server_fd_, hdr_data.data(), sizeof(chat262::message_header));
        if (readed < 0) {
            throw std::runtime_error(std::string("Cannot send the message: ") +
                                     std::string(strerror(errno)) +
                                     std::string("\n"));
        } else if (readed == 0) {
            throw std::runtime_error(
                "Cannot send the message: Server closed the connection\n");
        }
        total_read += readed;
    }
    // This should always succeed
    chat262::message_header::deserialize(hdr_data, hdr);
}

void client::recv_body(uint32_t body_len, std::vector<uint8_t>& data) const {
    data.resize(body_len);
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != body_len) {
        readed = read(server_fd_, data.data(), body_len);
        if (readed < 0) {
            throw std::runtime_error(std::string("Cannot send the message: ") +
                                     std::string(strerror(errno)) +
                                     std::string("\n"));
        } else if (readed == 0) {
            throw std::runtime_error(
                "Cannot send the message: Server closed the connection\n");
        }
        total_read += readed;
    }
}

void client::validate_hdr(const chat262::message_header& hdr,
                          const chat262::message_type& expected) {
    if (hdr.version_ != chat262::version) {
        throw std::runtime_error(std::string("Unsupported protocol version ") +
                                 std::to_string(hdr.version_) +
                                 std::string("\n"));
    } else if (hdr.type_ != expected) {
        throw std::runtime_error(
            std::string("Wrong message type, expected ") +
            std::string(chat262::message_type_lookup(expected)) +
            std::string(" (") + std::to_string(expected) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(hdr.type_)) +
            std::string(" (") + std::to_string(hdr.type_) + std::string(")\n"));
    }
}
