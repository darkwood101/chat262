#include "client.h"

#include "chat262_protocol.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <endian.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
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

std::string client::get_user_string(size_t min_len, size_t max_len) {
    std::string line;
    while (true) {
        std::cout << "Chat262> " << std::flush;
        std::getline(std::cin, line);
        if (std::cin.eof()) {
            std::cout << "\n";
            std::cin.clear();
            clearerr(stdin);
        } else if (line.length() < min_len || line.length() > max_len) {
            std::cout << "Input must be between " << min_len << " and "
                      << max_len << " characters\n";
        } else {
            return line;
        }
    }
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
        // TODO: check for errors
        total_sent += sent;
    }
}

void client::start_ui() {
    clear_screen();
    std::cout << "\n*** Welcome to Chat262 ***\n"
                 "\n"
                 "[1] Login\n"
                 "[2] Register\n"
                 "[0] Exit\n\n";
    uint32_t choice = get_user_unsigned<uint32_t>();
    switch (choice) {
        case 0:
            return;
        case 1:
            login();
            break;
        case 2:
            registration();
            break;
    }
}

void client::login() {
    clear_screen();
    std::cout << "*** Chat262 login ***\n"
                 "\n"
                 "Please enter your username.\n\n";
    std::string username = get_user_string(4, 40);
    clear_screen();
    std::cout << "*** Chat262 login ***\n"
                 "\n"
                 "Username: "
              << username
              << "\n\n"
                 "Please enter your password.\n\n";
    std::string password = get_user_string(4, 60);
    clear_screen();
    std::cout << "*** Chat262 login ***\n"
                 "\n"
                 "Username: "
              << username
              << "\n"
                 "Password: "
              << password
              << "\n\n"
                 "Logging in...\n";

    auto msg = chat262::login_body::serialize(username, password);
    send_msg(msg);
}

void client::registration() {
    clear_screen();
    std::cout << "*** Chat262 registration ***\n"
                 "\n"
                 "Please enter a username (between 4 and 40 characters).\n\n";
    std::string username = get_user_string(4, 40);
    clear_screen();
    std::cout
        << "*** Chat262 registration ***\n"
           "\n"
           "Username: "
        << username
        << "\n\n"
           "Please enter your password (between 4 and 60 characters).\n\n";
    std::string password = get_user_string(4, 60);
    clear_screen();
    std::cout << "*** Chat262 registration ***\n"
                 "\n"
                 "Username: "
              << username
              << "\n"
                 "Password: "
              << password
              << "\n\n"
                 "Registering...\n";

    auto msg = chat262::registration_body::serialize(username, password);
    send_msg(msg);
}

int main(int argc, char** argv) {
    client c;
    if (c.run(argc, argv) == status::ok) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
