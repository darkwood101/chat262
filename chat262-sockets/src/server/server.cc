#include "server.h"

#include "chat262_protocol.h"
#include "logger.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cinttypes>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

server::server() : server_fd_(-1), n_ip_addr_(0) {
}

server::~server() {
    // If the socket descriptor was open, close it
    if (server_fd_ != -1) {
        close(server_fd_);
    }
}

status server::run(int argc, char const* const* argv) {
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

    status s = start_listening();
    if (s != status::ok) {
        return s;
    }

    start_accepting();

    return status::ok;
}

server::cmdline_args server::parse_args(const int argc,
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

void server::usage(char const* prog) const {
    std::cerr << "usage: " << prog
              << " [-h] <ip address>\n"
                 "\n"
                 "Start the Chat262 server on IP address <ip address>.\n"
                 "The address should be in the xxx.xxx.xxx.xxx format.\n"
                 "\n"
                 "Options:\n"
                 "\t-h\t\t Display this message and exit.\n";
}

status server::start_listening() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        logger::log_err("Could not create socket: %s\n", strerror(errno));
        return status::error;
    }

    // Allow reuse of this address immediately. Otherwise, we might have to
    // wait.
    static constexpr int enable_addr_reuse = 1;
    if (setsockopt(server_fd_,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &enable_addr_reuse,
                   sizeof(enable_addr_reuse)) < 0) {
        logger::log_err("Could not enable address reuse: %s\n",
                        strerror(errno));
        return status::error;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(chat262::port);
    server_addr.sin_addr.s_addr = n_ip_addr_;
    if (bind(server_fd_, (const sockaddr*) &server_addr, sizeof(server_addr)) <
        0) {
        logger::log_err("Could not bind the socket: %s\n", strerror(errno));
        return status::error;
    }

    if (listen(server_fd_, 1) < 0) {
        logger::log_err("Could not listen on the socket: %s\n",
                        strerror(errno));
        return status::error;
    }
    logger::log_out("Listening on %s:%" PRIu16 "\n",
                    str_ip_addr_.c_str(),
                    chat262::port);
    return status::ok;
}

void server::start_accepting() {
    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_addr_len = sizeof(client_addr);

    while (true) {
        int client_fd =
            accept(server_fd_, (sockaddr*) &client_addr, &client_addr_len);
        std::thread t(&server::handle_client, this, client_fd, client_addr);
        t.detach();
    }
}

void server::handle_client(int client_fd, sockaddr_in client_addr) {
    // Make sure the connection was properly accepted
    if (client_fd < 0) {
        logger::log_err("Could not accept: %s\n", strerror(errno));
        return;
    }
    char client_ip[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET,
                   &client_addr.sin_addr,
                   client_ip,
                   sizeof(client_ip))) {
        logger::log_err("%s", "Could not read client's IP\n");
        return;
    }
    logger::log_out("Accepted connection from %s\n", client_ip);

    while (true) {
        chat262::message_header msg_hdr;
        status s = recv_hdr(client_fd, msg_hdr);
        if (s == status::error) {
            break;
        }

        logger::log_out("Received header: version %" PRIu16 ", type %" PRIu16
                        " (%s), body len %" PRIu32 "\n",
                        msg_hdr.version_,
                        msg_hdr.type_,
                        chat262::message_type_lookup(msg_hdr.type_),
                        msg_hdr.body_len_);

        if (msg_hdr.version_ != chat262::version) {
            logger::log_err("Unsupported protocol version %" PRIu16 "\n",
                            msg_hdr.version_);
            auto msg =
                chat262::wrong_version_response::serialize(chat262::version);
            send_msg(client_fd, msg);
            break;
        }

        std::vector<uint8_t> body;
        s = recv_body(client_fd, msg_hdr.body_len_, body);
        if (s == status::error) {
            break;
        }

        logger::log_out("%s", "Received the body\n");

        switch (msg_hdr.type_) {
            case chat262::msgtype_registration_request:
                handle_registration(client_fd, body);
                break;
            case chat262::msgtype_login_request:
                handle_login(client_fd, body);
                break;
            case chat262::msgtype_logout_request:
                handle_logout(client_fd, body);
                break;
            case chat262::msgtype_accounts_request:
                handle_list_accounts(client_fd, body);
                break;
            case chat262::msgtype_send_txt_request:
                handle_send_txt(client_fd, body);
                break;
            case chat262::msgtype_recv_txt_request:
                handle_recv_txt(client_fd, body);
                break;
            case chat262::msgtype_correspondents_request:
                handle_correspondents(client_fd, body);
                break;
            case chat262::msgtype_delete_request:
                handle_delete(client_fd, body);
                break;
            default:
                logger::log_err("Unknown message type %" PRIu16 "\n",
                                msg_hdr.type_);
                handle_invalid_type(client_fd);
                break;
        }
    }
    database_.logout();
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    logger::log_out("Terminated connection from %s\n", client_ip);
}

status server::send_msg(int client_fd,
                        std::shared_ptr<chat262::message> msg) const {
    size_t total_sent = 0;
    ssize_t sent = 0;
    size_t total_len = sizeof(chat262::message_header) + msg->hdr_.body_len_;
    while (total_sent != total_len) {
        sent = write(client_fd, msg.get(), total_len);
        if (sent < 0) {
            logger::log_err("Unable to send the message: %s\n",
                            strerror(errno));
            return status::error;
        }
        total_sent += sent;
    }
    return status::ok;
}

status server::recv_hdr(int client_fd, chat262::message_header& hdr) const {
    std::vector<uint8_t> hdr_data;
    hdr_data.resize(sizeof(chat262::message_header));
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != sizeof(chat262::message_header)) {
        readed =
            read(client_fd, hdr_data.data(), sizeof(chat262::message_header));
        if (readed < 0) {
            logger::log_err("Failed to receive the header: %s\n",
                            strerror(errno));
            return status::error;
        } else if (readed == 0) {
            logger::log_err(
                "%s",
                "Failed to receive the header: Client closed the connection\n");
            return status::error;
        }
        total_read += readed;
    }
    // This should never happen
    status s = chat262::message_header::deserialize(hdr_data, hdr);
    if (s != status::ok) {
        return s;
    }
    return status::ok;
}

status server::recv_body(int client_fd,
                         uint32_t body_len,
                         std::vector<uint8_t>& data) const {
    data.resize(body_len);
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != body_len) {
        readed = read(client_fd, data.data(), body_len);
        if (readed < 0) {
            logger::log_err("Failed to receive the body: %s\n",
                            strerror(errno));
            return status::error;
        } else if (readed == 0) {
            logger::log_err(
                "%s",
                "Failed to receive the body: Client closed the connection\n");
            return status::error;
        }
        total_read += readed;
    }
    return status::ok;
}

status server::handle_registration(int client_fd,
                                   const std::vector<uint8_t>& body_data) {
    std::string username;
    std::string password;
    status s = chat262::registration_request::deserialize(body_data,
                                                          username,
                                                          password);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    std::shared_ptr<chat262::message> msg;
    if (username.find_first_of("* ") != std::string::npos) {
        logger::log_out("Username \"%s\" contains invalid characters\n",
                        username.c_str());
        msg = chat262::registration_response::serialize(
            chat262::status_code_username_invalid);
        return send_msg(client_fd, msg);
    }

    s = database_.registration(username, password);
    if (s == status::ok) {
        logger::log_out(
            "Registered user with username \"%s\" and password \"%s\"\n",
            username.c_str(),
            password.c_str());
        msg =
            chat262::registration_response::serialize(chat262::status_code_ok);
    } else {
        logger::log_out("Username \"%s\" already exists\n", username.c_str());
        msg = chat262::registration_response::serialize(
            chat262::status_code_user_exists);
    }

    return send_msg(client_fd, msg);
}

status server::handle_login(int client_fd,
                            const std::vector<uint8_t>& body_data) {
    std::string username;
    std::string password;

    status s =
        chat262::login_request::deserialize(body_data, username, password);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out(
        "Login requested with username \"%s\" and password \"%s\"\n",
        username.c_str(),
        password.c_str());

    std::shared_ptr<chat262::message> msg;

    if (database_.is_logged_in()) {
        database_.logout();
    }

    s = database_.login(username, password);
    if (s == status::ok) {
        logger::log_out("%s", "Correct credentials\n");
        msg = chat262::login_response::serialize(chat262::status_code_ok);
    } else {
        logger::log_out("%s", "Invalid credentials\n");
        msg = chat262::login_response::serialize(
            chat262::status_code_invalid_credentials);
    }
    return send_msg(client_fd, msg);
}

status server::handle_logout(int client_fd,
                             const std::vector<uint8_t>& body_data) {
    status s = chat262::logout_request::deserialize(body_data);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out("%s", "Logout requested\n");

    std::shared_ptr<chat262::message> msg;

    if (!database_.is_logged_in()) {
        msg = chat262::logout_response::serialize(
            chat262::status_code_unauthorized);
        return send_msg(client_fd, msg);
    }

    database_.logout();

    msg = chat262::logout_response::serialize(chat262::status_code_ok);
    return send_msg(client_fd, msg);
}

status server::handle_list_accounts(int client_fd,
                                    const std::vector<uint8_t>& body_data) {
    std::string pattern;
    status s = chat262::accounts_request::deserialize(body_data, pattern);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out("List accounts requested, pattern \"%s\"\n",
                    pattern.c_str());

    std::shared_ptr<chat262::message> msg;
    std::vector<std::string> usernames;

    if (!database_.is_logged_in()) {
        msg = chat262::accounts_response::serialize(
            chat262::status_code_unauthorized,
            usernames);
        return send_msg(client_fd, msg);
    }

    usernames = database_.get_usernames(pattern);
    msg = chat262::accounts_response::serialize(chat262::status_code_ok,
                                                usernames);
    return send_msg(client_fd, msg);
}

status server::handle_send_txt(int client_fd,
                               const std::vector<uint8_t>& body_data) {
    std::string recipient;
    std::string txt;
    status s =
        chat262::send_txt_request::deserialize(body_data, recipient, txt);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out("Send text requested to user \"%s\"\n", recipient.c_str());

    std::shared_ptr<chat262::message> msg;

    if (!database_.is_logged_in()) {
        msg = chat262::send_txt_response::serialize(
            chat262::status_code_unauthorized);
        return send_msg(client_fd, msg);
    }

    s = database_.send_txt(recipient, txt);
    if (s == status::ok) {
        logger::log_out("Sent text to \"%s\"\n", recipient.c_str());
        msg = chat262::send_txt_response::serialize(chat262::status_code_ok);
    } else {
        logger::log_out("User \"%s\" does not exist\n", recipient.c_str());
        msg = chat262::send_txt_response::serialize(
            chat262::status_code_user_noexist);
    }
    return send_msg(client_fd, msg);
}

status server::handle_recv_txt(int client_fd,
                               const std::vector<uint8_t>& body_data) {
    std::string sender;
    status s = chat262::recv_txt_request::deserialize(body_data, sender);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out("Receive text requested from user \"%s\"\n",
                    sender.c_str());

    std::shared_ptr<chat262::message> msg;
    chat c;

    if (!database_.is_logged_in()) {
        msg = chat262::recv_txt_response::serialize(
            chat262::status_code_unauthorized,
            c);
        return send_msg(client_fd, msg);
    }

    s = database_.recv_txt(sender, c);
    if (s == status::ok) {
        logger::log_out("Sending texts from \"%s\"\n", sender.c_str());
        msg = chat262::recv_txt_response::serialize(chat262::status_code_ok, c);
    } else {
        logger::log_out("User \"%s\" does not exist\n", sender.c_str());
        msg = chat262::recv_txt_response::serialize(
            chat262::status_code_user_noexist,
            c);
    }
    return send_msg(client_fd, msg);
}

status server::handle_correspondents(int client_fd,
                                     const std::vector<uint8_t>& body_data) {
    status s = chat262::correspondents_request::deserialize(body_data);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out("%s", "Retrieve correspondents requested\n");

    std::shared_ptr<chat262::message> msg;
    std::vector<std::string> correspondents;

    if (!database_.is_logged_in()) {
        msg = chat262::correspondents_response::serialize(
            chat262::status_code_unauthorized,
            correspondents);
        return send_msg(client_fd, msg);
    }

    database_.get_correspondents(correspondents);
    msg = chat262::correspondents_response::serialize(chat262::status_code_ok,
                                                      correspondents);
    return send_msg(client_fd, msg);
}

status server::handle_delete(int client_fd,
                             const std::vector<uint8_t>& body_data) {
    status s = chat262::delete_request::deserialize(body_data);
    if (s != status::ok) {
        logger::log_err("%s", "Unable to deserialize request body\n");
        return s;
    }

    logger::log_out("%s", "Delete account requested\n");

    std::shared_ptr<chat262::message> msg;

    if (!database_.is_logged_in()) {
        msg = chat262::delete_response::serialize(
            chat262::status_code_unauthorized);
        return send_msg(client_fd, msg);
    }

    database_.delete_user();
    msg = chat262::delete_response::serialize(chat262::status_code_ok);
    return send_msg(client_fd, msg);
}

status server::handle_invalid_type(int client_fd) {
    auto msg = chat262::invalid_type_response::serialize();
    return send_msg(client_fd, msg);
}
