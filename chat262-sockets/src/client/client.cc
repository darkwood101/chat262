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

status client::login(const std::string& username,
                     const std::string& password,
                     uint32_t& stat_code) {
    auto msg = chat262::login_request::serialize(username, password);
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_login_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::login_response::deserialize(body, stat_code);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::registration(const std::string& username,
                            const std::string& password,
                            uint32_t& stat_code) {
    auto msg = chat262::registration_request::serialize(username, password);
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_registration_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::registration_response::deserialize(body, stat_code);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::logout(uint32_t& stat_code) {
    auto msg = chat262::logout_request::serialize();
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_logout_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::logout_response::deserialize(body, stat_code);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::list_accounts(const std::string& pattern,
                             uint32_t& stat_code,
                             std::vector<std::string>& usernames) {
    auto msg = chat262::accounts_request::serialize(pattern);
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_accounts_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::accounts_response::deserialize(body, stat_code, usernames);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::send_txt(const std::string& recipient,
                        const std::string& txt,
                        uint32_t& stat_code) {
    auto msg = chat262::send_txt_request::serialize(recipient, txt);
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_send_txt_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::send_txt_response::deserialize(body, stat_code);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::recv_txt(const std::string& sender,
                        uint32_t& stat_code,
                        chat& c) {
    auto msg = chat262::recv_txt_request::serialize(sender);
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_recv_txt_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::recv_txt_response::deserialize(body, stat_code, c);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::recv_correspondents(uint32_t& stat_code,
                                   std::vector<std::string>& correspondents) {
    auto msg = chat262::correspondents_request::serialize();
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_correspondents_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::correspondents_response::deserialize(body,
                                                      stat_code,
                                                      correspondents);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::delete_account(uint32_t& stat_code) {
    auto msg = chat262::delete_request::serialize();
    status s = send_msg(msg);
    if (s != status::ok) {
        return s;
    }

    chat262::message_header msg_hdr;
    s = recv_hdr(msg_hdr);
    if (s != status::ok) {
        return s;
    }
    s = validate_hdr(msg_hdr, chat262::msgtype_delete_response);
    if (s != status::ok) {
        return s;
    }

    std::vector<uint8_t> body;
    s = recv_body(msg_hdr.body_len_, body);
    if (s != status::ok) {
        return s;
    }

    s = chat262::delete_response::deserialize(body, stat_code);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status client::send_msg(std::shared_ptr<chat262::message> msg) const {
    size_t total_sent = 0;
    ssize_t sent = 0;
    size_t total_len = sizeof(chat262::message_header) + msg->hdr_.body_len_;
    while (total_sent != total_len) {
        sent = write(server_fd_, msg.get(), total_len);
        if (sent < 0) {
            return status::send_error;
        }
        total_sent += sent;
    }
    return status::ok;
}

status client::recv_hdr(chat262::message_header& hdr) const {
    std::vector<uint8_t> hdr_data;
    hdr_data.resize(sizeof(chat262::message_header));
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != sizeof(chat262::message_header)) {
        readed =
            read(server_fd_, hdr_data.data(), sizeof(chat262::message_header));
        if (readed < 0) {
            return status::receive_error;
        } else if (readed == 0) {
            return status::closed_connection;
        }
        total_read += readed;
    }
    // This should always succeed
    chat262::message_header::deserialize(hdr_data, hdr);
    return status::ok;
}

status client::recv_body(uint32_t body_len, std::vector<uint8_t>& data) const {
    data.resize(body_len);
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != body_len) {
        readed = read(server_fd_, data.data(), body_len);
        if (readed < 0) {
            return status::receive_error;
        } else if (readed == 0) {
            return status::closed_connection;
        }
        total_read += readed;
    }
    return status::ok;
}

status client::validate_hdr(const chat262::message_header& hdr,
                            const chat262::message_type& expected) const {
    if (hdr.version_ != chat262::version || hdr.type_ != expected) {
        return status::header_error;
    }
    return status::ok;
}
