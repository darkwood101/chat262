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

status client::run(int argc, char const* const* argv) {
    cmdline_args args;
    try {
        args = parse_args(argc, argv);
    } catch (std::exception& e) {
        std::cerr << e.what() << "\n";
        usage(argv[0]);
        return status::error;
    }

    if (args.help_) {
        usage(argv[0]);
        return status::ok;
    }

    n_ip_addr_ = args.n_ip_addr_;
    str_ip_addr_ = argv[1];

    status s = connect_server();
    if (s != status::ok) {
        return s;
    }

    start_ui();

    return status::ok;
}

client::cmdline_args client::parse_args(const int argc,
                                        char const* const* argv) const {
    if (argc != 2 && argc != 3) {
        throw std::invalid_argument("Wrong number of arguments");
    }

    cmdline_args args;
    // Look for "-h"
    if (strcmp(argv[1], "-h") == 0 ||
        (argc == 3 && strcmp(argv[2], "-h") == 0)) {
        args.help_ = true;
        return args;
    } else {
        args.help_ = false;
    }
    // Parse the IP address
    if (inet_pton(AF_INET, argv[1], &(args.n_ip_addr_)) != 1) {
        throw std::invalid_argument("Invalid IP address");
    }
    return args;
}

void client::usage(char const* prog) const {
    std::cerr << "usage: " << prog
              << " [-h] <ip address>\n"
                 "\n"
                 "Start the Chat262 client and connect to a Chat262 server on "
                 "IP address\n"
                 "<ip address>. The address should be in the xxx.xxx.xxx.xxx "
                 "format.\n"
                 "\n"
                 "Options:\n"
                 "\t-h\t\t Display this message and exit.\n";
}

status client::connect_server() {
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

void client::start_ui() {
    uint32_t stat_code = 0;
    interface_.next_ = screen_type::login_registration;
    chat curr_chat;
    std::string me;
    std::string password;
    std::string pattern;
    std::string correspondent;
    std::vector<std::string> matched_usernames;
    std::vector<std::string> all_correspondents;
    bool hit_escape;

    while (true) {
        switch (interface_.next_) {
            case screen_type::login_registration: {
                user_choice choice = interface_.login_registration();
                switch (choice) {
                    case 0:
                        interface_.next_ = screen_type::login;
                        break;
                    case 1:
                        interface_.next_ = screen_type::registration;
                        break;
                    case 2:
                    case ESCAPE:
                        interface_.next_ = screen_type::exit;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::login: {
                interface_.login(me, password, hit_escape);
                if (hit_escape) {
                    interface_.next_ = screen_type::login_registration;
                    break;
                }
                stat_code = login(me, password);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::main_menu;
                } else {
                    interface_.next_ = screen_type::login_fail;
                }
            } break;

            case screen_type::login_fail: {
                user_choice choice = interface_.login_fail(stat_code);
                switch (choice) {
                    case 0:
                        interface_.next_ = screen_type::login;
                        break;
                    case 1:
                    case ESCAPE:
                        interface_.next_ = screen_type::login_registration;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::registration: {
                interface_.registration(me, password, hit_escape);
                if (hit_escape) {
                    interface_.next_ = screen_type::login_registration;
                    break;
                }
                stat_code = registration(me, password);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::registration_success;
                } else {
                    interface_.next_ = screen_type::registration_fail;
                }
            } break;

            case screen_type::registration_success: {
                interface_.registration_success();
                stat_code = login(me, password);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::main_menu;
                } else {
                    interface_.next_ = screen_type::login_fail;
                }
            } break;

            case screen_type::registration_fail: {
                user_choice choice = interface_.registration_fail(stat_code);
                switch (choice) {
                    case 0:
                        interface_.next_ = screen_type::registration;
                        break;
                    case 1:
                    case ESCAPE:
                        interface_.next_ = screen_type::login_registration;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::main_menu: {
                user_choice choice = interface_.main_menu(me);
                switch (choice) {
                    case 0:
                        interface_.next_ = screen_type::open_chats;
                        break;
                    case 1:
                        interface_.next_ = screen_type::list_accounts;
                        break;
                    case 2:
                        interface_.next_ = screen_type::delete_account;
                        break;
                    case 3:
                    case ESCAPE:
                        stat_code = logout();
                        if (stat_code == chat262::status_code_ok) {
                            interface_.next_ = screen_type::login_registration;
                        }
                        // TODO: handle logout fail
                    default:
                        break;
                }
            } break;

            case screen_type::open_chats: {
                interface_.open_chats();
                stat_code = recv_correspondents(all_correspondents);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::open_chats_success;
                } else {
                    interface_.next_ = screen_type::open_chats_fail;
                }
            } break;

            case screen_type::open_chats_success: {
                user_choice choice =
                    interface_.open_chats_success(all_correspondents);
                if (choice == 0) {
                    interface_.next_ = screen_type::new_chat;
                } else if (choice == 1 || choice == ESCAPE) {
                    interface_.next_ = screen_type::main_menu;
                } else {
                    correspondent = all_correspondents[choice - 2];
                    interface_.next_ = screen_type::recv_txt;
                }
            } break;

            case screen_type::open_chats_fail: {
                interface_.open_chats_fail(stat_code);
                interface_.next_ = screen_type::main_menu;
            } break;

            case screen_type::new_chat: {
                interface_.new_chat(correspondent, hit_escape);
                if (hit_escape) {
                    interface_.next_ = screen_type::open_chats;
                    break;
                }
                interface_.next_ = screen_type::recv_txt;
            } break;

            case screen_type::recv_txt: {
                interface_.recv_txt();
                stat_code = recv_txt(correspondent, curr_chat);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::send_txt;
                } else {
                    interface_.next_ = screen_type::recv_txt_fail;
                }
            } break;

            case screen_type::recv_txt_fail: {
                interface_.recv_txt_fail(stat_code);
                interface_.next_ = screen_type::open_chats;
            } break;

            case screen_type::send_txt: {
                std::string partial_txt;

                interface_.draw_send_txt(me,
                                         correspondent,
                                         curr_chat,
                                         partial_txt);
                listener_should_exit_ = false;

                std::thread listener(&client::background_listener,
                                     this,
                                     std::ref(me),
                                     std::ref(correspondent),
                                     std::ref(curr_chat),
                                     std::ref(partial_txt));

                interface_.prompt_send_txt(partial_txt,
                                           listener_m_,
                                           hit_escape);

                std::unique_lock<std::mutex> lock(listener_m_);
                listener_should_exit_ = true;
                listener_cv_.notify_all();
                lock.unlock();

                listener.join();

                if (hit_escape) {
                    interface_.next_ = screen_type::open_chats;
                    break;
                }
                stat_code = send_txt(correspondent, partial_txt);
                partial_txt.clear();
                if (stat_code != chat262::status_code_ok) {
                    interface_.next_ = screen_type::send_txt_fail;
                    break;
                }

                stat_code = recv_txt(correspondent, curr_chat);
                if (stat_code != chat262::status_code_ok) {
                    interface_.next_ = screen_type::recv_txt_fail;
                }
            } break;

            case screen_type::send_txt_fail: {
                interface_.send_txt_fail(stat_code);
                interface_.next_ = screen_type::open_chats;
            } break;

            case screen_type::list_accounts: {
                interface_.list_accounts(pattern, hit_escape);
                if (hit_escape) {
                    interface_.next_ = screen_type::main_menu;
                    break;
                }
                stat_code = list_accounts(pattern, matched_usernames);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::list_accounts_success;
                } else {
                    interface_.next_ = screen_type::list_accounts_fail;
                }
            } break;

            case screen_type::list_accounts_success: {
                interface_.list_accounts_success(pattern, matched_usernames);
                interface_.next_ = screen_type::list_accounts;
            } break;

            case screen_type::list_accounts_fail: {
                interface_.list_accounts_fail(stat_code);
                interface_.next_ = screen_type::main_menu;
            } break;

            case screen_type::delete_account: {
                user_choice choice = interface_.delete_account();
                switch (choice) {
                    case 0:
                    case ESCAPE:
                        interface_.next_ = screen_type::main_menu;
                        break;
                    case 1:
                        stat_code = delete_account();
                        if (stat_code == chat262::status_code_ok) {
                            interface_.next_ =
                                screen_type::delete_account_success;
                        } else {
                            interface_.next_ = screen_type::delete_account_fail;
                        }
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::delete_account_success: {
                interface_.delete_account_success();
                interface_.next_ = screen_type::login_registration;
            } break;

            case screen_type::delete_account_fail: {
                interface_.delete_account_fail(stat_code);
                interface_.next_ = screen_type::main_menu;
            } break;

            case screen_type::exit: {
                return;
            }
        }
    }
}

void client::background_listener(const std::string& me,
                                 const std::string& correspondent,
                                 chat& curr_chat,
                                 const std::string& partial_txt) {
    using std::chrono::high_resolution_clock;
    using std::chrono::seconds;
    std::unique_lock<std::mutex> lock(listener_m_);
    while (!listener_should_exit_) {
        auto end_time = high_resolution_clock::now() + seconds(2);
        auto cv_status = listener_cv_.wait_until(lock, end_time);
        if (cv_status != std::cv_status::timeout) {
            continue;
        }
        lock.unlock();
        size_t prev_size = curr_chat.texts_.size();
        recv_txt(correspondent, curr_chat);
        lock.lock();
        if (prev_size == curr_chat.texts_.size()) {
            continue;
        }
        interface_.draw_send_txt(me, correspondent, curr_chat, partial_txt);
    }
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
