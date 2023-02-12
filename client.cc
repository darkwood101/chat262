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

void client::start_ui() {
    uint32_t stat_code = 0;
    interface_.next_ = screen_type::login_registration;
    chat curr_chat;
    std::string me;
    std::string correspondent;
    std::vector<std::string> all_usernames;

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
                        interface_.next_ = screen_type::exit;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::login: {
                std::string password;
                interface_.login(me, password);
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
                        interface_.next_ = screen_type::login_registration;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::registration: {
                std::string username;
                std::string password;
                interface_.registration(username, password);
                stat_code = registration(username, password);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::registration_success;
                } else {
                    interface_.next_ = screen_type::registration_fail;
                }
            } break;

            case screen_type::registration_success: {
                interface_.registration_success();
                interface_.next_ = screen_type::login_registration;
            } break;

            case screen_type::registration_fail: {
                user_choice choice = interface_.registration_fail(stat_code);
                switch (choice) {
                    case 0:
                        interface_.next_ = screen_type::registration;
                        break;
                    case 1:
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
                        interface_.next_ = screen_type::open_chat;
                        break;
                    case 1:
                        interface_.next_ = screen_type::list_accounts;
                        break;
                    case 2:
                        stat_code = logout();
                        if (stat_code == chat262::status_code_ok) {
                            interface_.next_ = screen_type::login_registration;
                        }
                        // TODO: handle logout fail
                    default:
                        break;
                }
            } break;

            case screen_type::open_chat: {
                interface_.open_chat(correspondent);
                interface_.next_ = screen_type::send_txt;
            } break;

            case screen_type::open_chat_fail: {
                user_choice choice = interface_.open_chat_fail(stat_code);
                switch (choice) {
                    case 1:
                        interface_.next_ = screen_type::open_chat;
                        break;
                    case 2:
                        interface_.next_ = screen_type::main_menu;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::send_txt: {
                stat_code = recv_txt(correspondent, curr_chat);

                if (stat_code != chat262::status_code_ok) {
                    interface_.next_ = screen_type::open_chat_fail;
                    break;
                }

                std::mutex m;
                std::condition_variable cv;
                bool should_exit = false;
                using std::chrono::high_resolution_clock;
                using std::chrono::seconds;
                std::string partial_txt;
                int ss = system("clear");
                (void) ss;
                std::cout << "\n*** Chat262 ***\n"
                             "\n"
                             "================================================="
                             "============"
                             "===================\n";
                for (const text& t : curr_chat.texts_) {
                    if (t.sender_ == text::sender_you) {
                        std::cout << me << ": " << t.content_ << "\n\n";
                    } else {
                        std::cout << correspondent << ": " << t.content_
                                  << "\n\n";
                    }
                }
                std::cout << "================================================="
                             "============"
                             "===================\n\n";
                std::cout << "Chat262> " << partial_txt << std::flush;

                std::thread listener([&]() {
                    std::unique_lock<std::mutex> lock(m);
                    while (!should_exit) {
                        auto end_time =
                            high_resolution_clock::now() + seconds(2);
                        auto cv_status = cv.wait_until(lock, end_time);
                        if (cv_status == std::cv_status::timeout) {
                            std::string tl_correspondent = correspondent;
                            chat tl_curr_chat;
                            lock.unlock();
                            recv_txt(tl_correspondent, tl_curr_chat);
                            if (tl_curr_chat.texts_.size() ==
                                curr_chat.texts_.size()) {
                                lock.lock();
                                continue;
                            }
                            lock.lock();
                            curr_chat = tl_curr_chat;
                            int s = system("clear");
                            (void) s;
                            std::cout << "\n*** Chat262 ***\n"
                                         "\n"
                                         "====================================="
                                         "========================"
                                         "===================\n";
                            for (const text& t : curr_chat.texts_) {
                                if (t.sender_ == text::sender_you) {
                                    std::cout << me << ": " << t.content_
                                              << "\n\n";
                                } else {
                                    std::cout << correspondent << ": "
                                              << t.content_ << "\n\n";
                                }
                            }
                            std::cout << "====================================="
                                         "========================"
                                         "===================\n\n";
                            std::cout << "Chat262> " << partial_txt
                                      << std::flush;
                        }
                    }
                });
                termios old_t;
                tcgetattr(STDIN_FILENO, &old_t);
                termios new_t = old_t;
                new_t.c_lflag &= ~ICANON;
                new_t.c_lflag &= ~ECHO;
                new_t.c_cc[VMIN] = 1;
                new_t.c_cc[VTIME] = 0;
                tcsetattr(0, TCSANOW, &old_t);
                pollfd fd;
                memset(&fd, 0, sizeof(pollfd));
                fd.fd = STDIN_FILENO;
                fd.events |= POLLIN;
                char c = 0;
                while (true) {
                    if (poll(&fd, 1, -1) != 0) {
                        std::unique_lock<std::mutex> lock(m);
                        ssize_t s = read(STDIN_FILENO, &c, 1);
                        (void) s;
                        if (c == old_t.c_cc[VERASE]) {
                            s = write(STDOUT_FILENO, "\b \b", 3);
                            (void) s;
                            partial_txt.pop_back();
                        } else if (c == '\n') {
                            break;
                        } else {
                            s = write(STDOUT_FILENO, &c, 1);
                            (void) s;
                            partial_txt.push_back(c);
                        }
                    }
                }
                std::unique_lock<std::mutex> lock(m);
                should_exit = true;
                cv.notify_all();
                lock.unlock();
                listener.join();

                stat_code = send_txt(correspondent, partial_txt);
                partial_txt.clear();
                if (stat_code != chat262::status_code_ok) {
                    interface_.next_ = screen_type::send_txt_fail;
                    break;
                }
            } break;

            case screen_type::send_txt_fail: {
                user_choice choice = interface_.send_txt_fail(stat_code);
                switch (choice) {
                    case 1:
                        interface_.next_ = screen_type::open_chat;
                        break;
                    case 2:
                        interface_.next_ = screen_type::main_menu;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::list_accounts: {
                interface_.list_accounts();
                stat_code = list_accounts(all_usernames);
                if (stat_code == chat262::status_code_ok) {
                    interface_.next_ = screen_type::list_accounts_success;
                } else {
                    interface_.next_ = screen_type::list_accounts_fail;
                }
            } break;

            case screen_type::list_accounts_success: {
                interface_.list_accounts_success(all_usernames);
                interface_.next_ = screen_type::main_menu;
            } break;

            case screen_type::list_accounts_fail: {
                user_choice choice = interface_.list_accounts_fail(stat_code);
                switch (choice) {
                    case 0:
                        interface_.next_ = screen_type::list_accounts;
                        break;
                    case 1:
                        interface_.next_ = screen_type::main_menu;
                        break;
                    default:
                        break;
                }
            } break;

            case screen_type::exit: {
                return;
            }
        }
    }
}

uint32_t client::login(const std::string& username,
                       const std::string& password) {
    auto msg = chat262::login_request::serialize(username, password);
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    if (msg_hdr.version_ != chat262::version) {
        throw std::runtime_error(
            std::string("Login response: Unsupported protocol version ") +
            std::to_string(msg_hdr.version_) + std::string("\n"));
    } else if (msg_hdr.type_ != chat262::msgtype_login_response) {
        throw std::runtime_error(
            std::string("Login response: Wrong message type, expected ") +
            std::string(
                chat262::message_type_lookup(chat262::msgtype_login_response)) +
            std::string(" (") +
            std::to_string(chat262::msgtype_login_response) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(msg_hdr.type_)) +
            std::string(" (") + std::to_string(msg_hdr.type_) +
            std::string(")\n"));
    }
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
    if (msg_hdr.version_ != chat262::version) {
        throw std::runtime_error(
            std::string(
                "Registration response: Unsupported protocol version ") +
            std::to_string(msg_hdr.version_) + std::string("\n"));
    } else if (msg_hdr.type_ != chat262::msgtype_registration_response) {
        throw std::runtime_error(
            std::string(
                "Registration response: Wrong message type, expected ") +
            std::string(chat262::message_type_lookup(
                chat262::msgtype_registration_response)) +
            std::string(" (") +
            std::to_string(chat262::msgtype_registration_response) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(msg_hdr.type_)) +
            std::string(" (") + std::to_string(msg_hdr.type_) +
            std::string(")\n"));
    }
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
    if (msg_hdr.version_ != chat262::version) {
        throw std::runtime_error(
            std::string("Logout response: Unsupported protocol version ") +
            std::to_string(msg_hdr.version_) + std::string("\n"));
    } else if (msg_hdr.type_ != chat262::msgtype_logout_response) {
        throw std::runtime_error(
            std::string("Logout response: Wrong message type, expected ") +
            std::string(chat262::message_type_lookup(
                chat262::msgtype_logout_response)) +
            std::string(" (") +
            std::to_string(chat262::msgtype_logout_response) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(msg_hdr.type_)) +
            std::string(" (") + std::to_string(msg_hdr.type_) +
            std::string(")\n"));
    }
    std::vector<uint8_t> body;
    recv_body(msg_hdr.body_len_, body);

    uint32_t stat_code;
    if (chat262::logout_response::deserialize(body, stat_code) != status::ok) {
        throw std::runtime_error(
            "Logout response: Unable to deserialize the message body\n");
    }
    return stat_code;
}

uint32_t client::list_accounts(std::vector<std::string>& usernames) {
    auto msg = chat262::accounts_request::serialize();
    send_msg(msg);

    chat262::message_header msg_hdr;
    recv_hdr(msg_hdr);
    if (msg_hdr.version_ != chat262::version) {
        throw std::runtime_error(
            std::string("Accounts response: Unsupported protocol version ") +
            std::to_string(msg_hdr.version_) + std::string("\n"));
    } else if (msg_hdr.type_ != chat262::msgtype_accounts_response) {
        throw std::runtime_error(
            std::string("Accounts response: Wrong message type, expected ") +
            std::string(chat262::message_type_lookup(
                chat262::msgtype_accounts_response)) +
            std::string(" (") +
            std::to_string(chat262::msgtype_accounts_response) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(msg_hdr.type_)) +
            std::string(" (") + std::to_string(msg_hdr.type_) +
            std::string(")\n"));
    }
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
    if (msg_hdr.version_ != chat262::version) {
        throw std::runtime_error(
            std::string("Send text response: Unsupported protocol version ") +
            std::to_string(msg_hdr.version_) + std::string("\n"));
    } else if (msg_hdr.type_ != chat262::msgtype_send_txt_response) {
        throw std::runtime_error(
            std::string("Send text response: Wrong message type, expected ") +
            std::string(chat262::message_type_lookup(
                chat262::msgtype_send_txt_response)) +
            std::string(" (") +
            std::to_string(chat262::msgtype_send_txt_response) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(msg_hdr.type_)) +
            std::string(" (") + std::to_string(msg_hdr.type_) +
            std::string(")\n"));
    }
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
    if (msg_hdr.version_ != chat262::version) {
        throw std::runtime_error(
            std::string(
                "Receive text response: Unsupported protocol version ") +
            std::to_string(msg_hdr.version_) + std::string("\n"));
    } else if (msg_hdr.type_ != chat262::msgtype_recv_txt_response) {
        throw std::runtime_error(
            std::string(
                "Receive text response: Wrong message type, expected ") +
            std::string(chat262::message_type_lookup(
                chat262::msgtype_recv_txt_response)) +
            std::string(" (") +
            std::to_string(chat262::msgtype_recv_txt_response) +
            std::string("), got ") +
            std::string(chat262::message_type_lookup(msg_hdr.type_)) +
            std::string(" (") + std::to_string(msg_hdr.type_) +
            std::string(")\n"));
    }
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

int main(int argc, char** argv) {
    client c;
    if (c.run(argc, argv) == status::ok) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
