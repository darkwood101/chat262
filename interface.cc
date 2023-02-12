#include "interface.h"

#include "chat262_protocol.h"
#include "common.h"

#include <cerrno>
#include <cstring>
#include <poll.h>
#include <unistd.h>

interface::interface() {
    // Save old terminal state
    if (tcgetattr(STDIN_FILENO, &old_t_) < 0) {
        throw std::runtime_error(std::string("Cannot save terminal state: ") +
                                 std::string(strerror(errno)));
    }
    new_t_ = old_t_;
    // Turn on non-canonical mode and disable character echo
    new_t_.c_lflag &= ~(ICANON | ECHO);
    new_t_.c_cc[VMIN] = 1;
    new_t_.c_cc[VTIME] = 0;
    // Set new terminal state
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_t_) < 0) {
        throw std::runtime_error(std::string("Cannot set terminal state: ") +
                                 std::string(strerror(errno)));
    }
    // Per man pages: "Note that tcsetattr() returns success if any of the
    // requested  changes  could  be  successfull carried  out.   Therefore,
    // when making multiple changes it may be necessary to follow this call with
    // a further call to tcgetattr() to check that all changes have been
    // performed successfully."
    termios check_t;
    if (tcgetattr(STDIN_FILENO, &check_t) < 0) {
        throw std::runtime_error(std::string("Cannot save terminal state: ") +
                                 std::string(strerror(errno)));
    }
    if (check_t.c_lflag != new_t_.c_lflag ||
        check_t.c_cc[VMIN] != new_t_.c_cc[VMIN] ||
        check_t.c_cc[VTIME] != new_t_.c_cc[VTIME]) {
        throw std::runtime_error(std::string("Cannot set terminal state: ") +
                                 std::string(strerror(errno)));
    }
}

interface::~interface() {
    // Restore terminal state
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_t_);
}

user_choice interface::make_selection(const std::string& prefix,
                                      const std::vector<std::string>& choices) {
    pollfd fd;
    memset(&fd, 0, sizeof(pollfd));
    fd.fd = STDIN_FILENO;
    fd.events |= POLLIN;
    user_choice selection = 0;
    char c = 0;
    draw_choices(prefix, choices, selection);
    while (true) {
        if (poll(&fd, 1, -1) != 0) {
            std::unique_lock<std::mutex> lock(m_);
            ssize_t s = read(STDIN_FILENO, &c, 1);
            (void) s;
            switch (c) {
                case 'w':
                case 'W':
                    if (selection == 0) {
                        selection = choices.size() - 1;
                    } else {
                        selection = (selection - 1) % choices.size();
                    }
                    draw_choices(prefix, choices, selection);
                    break;
                case 's':
                case 'S':
                    selection = (selection + 1) % choices.size();
                    draw_choices(prefix, choices, selection);
                    break;
                case '\n':
                    return selection;
            }
        }
    }
}

void interface::wait_anykey() {
    pollfd fd;
    memset(&fd, 0, sizeof(pollfd));
    fd.fd = STDIN_FILENO;
    fd.events |= POLLIN;
    char c = 0;
    while (true) {
        if (poll(&fd, 1, -1) != 0) {
            std::unique_lock<std::mutex> lock(m_);
            ssize_t s = read(STDIN_FILENO, &c, 1);
            (void) s;
            return;
        }
    }
}

user_choice interface::login_registration() {
    return make_selection("\n*** Welcome to Chat262 ***\n"
                          "\n",
                          {"Login", "Register", "Exit"});
}

void interface::login(std::string& username, std::string& password) {
    clear_screen();
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Please enter your username.\n\n";
    username = get_user_string(4, 40);
    clear_screen();
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Username: "
              << username
              << "\n\n"
                 "Please enter your password.\n\n";
    password = get_user_string(4, 60);
    clear_screen();
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Username: "
              << username
              << "\n"
                 "Password: "
              << password
              << "\n\n"
                 "Logging in..."
              << std::flush;
}

user_choice interface::login_fail(uint32_t stat_code) {
    std::string prefix = "\n*** Chat262 login ***\n"
                         "\n"
                         "Login failed: ";
    const char* description = chat262::status_code_lookup(stat_code);
    if (strcmp(description, "Unknown") == 0) {
        prefix.append(description);
        prefix.append(" (status code " + std::to_string(stat_code) + ")\n");
    } else {
        prefix.append(description);
    }
    prefix.append("\n"
                  "\n"
                  "Try again?\n"
                  "\n");
    return make_selection(prefix, {"Yes", "No"});
}

void interface::registration(std::string& username, std::string& password) {
    clear_screen();
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Please enter a username (between 4 and 40 characters).\n\n";
    username = get_user_string(4, 40);
    clear_screen();
    std::cout
        << "\n*** Chat262 registration ***\n"
           "\n"
           "Username: "
        << username
        << "\n\n"
           "Please enter your password (between 4 and 60 characters).\n\n";
    password = get_user_string(4, 60);
    clear_screen();
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Username: "
              << username
              << "\n"
                 "Password: "
              << password
              << "\n\n"
                 "Registering..."
              << std::flush;
}

void interface::registration_success() {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Registration successful! You can now use your username and "
                 "password to log in.\n"
                 "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

user_choice interface::registration_fail(uint32_t stat_code) {
    std::string prefix = "\n*** Chat262 login ***\n"
                         "\n"
                         "Registration failed: ";
    const char* description = chat262::status_code_lookup(stat_code);
    if (strcmp(description, "Unknown") == 0) {
        prefix.append(description);
        prefix.append(" (status code " + std::to_string(stat_code) + ")\n");
    } else {
        prefix.append(description);
    }
    prefix.append("\n"
                  "\n"
                  "Try again?\n"
                  "\n");
    return make_selection(prefix, {"Yes", "No"});
}

user_choice interface::main_menu(const std::string& username) {
    std::string prefix = "\n*** Chat 262 main menu ***\n"
                         "\n"
                         "Welcome, ";
    prefix.append(username);
    prefix.append("\n\n");
    return make_selection(prefix, {"Chats", "List all accounts", "Log out"});
}

void interface::list_accounts() {
    std::cout << "\n*** Chat 262 accounts ***\n"
                 "\n"
                 "Retrieving the list of accounts..."
              << std::flush;
}

void interface::list_accounts_success(
    const std::vector<std::string>& usernames) {
    clear_screen();
    std::cout << "\n*** Chat262 accounts ***\n"
                 "\n"
                 "The following usernames are registered with Chat262:\n\n";
    for (size_t i = 0; i != usernames.size(); ++i) {
        std::cout << " " << i + 1 << ".\t" << usernames[i] << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

user_choice interface::list_accounts_fail(uint32_t stat_code) {
    std::string prefix = "\n*** Chat262 login ***\n"
                         "\n"
                         "Retrieving accounts failed: ";
    const char* description = chat262::status_code_lookup(stat_code);
    if (strcmp(description, "Unknown") == 0) {
        prefix.append(description);
        prefix.append(" (status code " + std::to_string(stat_code) + ")\n");
    } else {
        prefix.append(description);
    }
    prefix.append("\n"
                  "\n"
                  "Try again?\n"
                  "\n");
    return make_selection(prefix, {"Yes", "No"});
}

void interface::open_chats() {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Retrieving your chats..."
              << std::flush;
}

user_choice interface::open_chats_success(
    const std::vector<std::string>& usernames) {
    std::vector<std::string> usernames_copy = usernames;
    usernames_copy.insert(usernames_copy.begin(), 1, "Start a new chat\n");
    return make_selection("\n*** Chat 262 ***\n"
                          "\n"
                          "Select a chat.\n\n",
                          usernames_copy);
}

user_choice interface::open_chats_fail(uint32_t stat_code) {
    std::string prefix = "\n*** Chat262 ***\n"
                         "\n"
                         "Retrieving chats failed: ";
    const char* description = chat262::status_code_lookup(stat_code);
    if (strcmp(description, "Unknown") == 0) {
        prefix.append(description);
        prefix.append(" (status code " + std::to_string(stat_code) + ")\n");
    } else {
        prefix.append(description);
    }
    prefix.append("\n"
                  "\n"
                  "Try again?\n"
                  "\n");
    return make_selection(prefix, {"Yes", "No"});
}

void interface::new_chat(std::string& correspondent) {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Enter the username of the person you wish to chat with.\n\n";
    correspondent = get_user_string(4, 40);
}

void interface::recv_txt() {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Opening the requested chat..."
              << std::flush;
}

void interface::recv_txt_fail(uint32_t stat_code) {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code);
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Failed to retrieve the chat: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

void interface::draw_send_txt(const std::string& me,
                              const std::string& correspondent,
                              const chat& c,
                              const std::string& partial_txt) {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "============================================================="
                 "===================\n";

    for (const text& t : c.texts_) {
        if (t.sender_ == text::sender_you) {
            std::cout << me << " (you): " << t.content_ << "\n\n";
        } else {
            std::cout << correspondent << ": " << t.content_ << "\n\n";
        }
    }
    std::cout << "============================================================="
                 "===================\n\n";
    std::cout << "Chat262> " << partial_txt << std::flush;
}

void interface::prompt_send_txt(std::string& partial_txt, std::mutex& m) {
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
            if (c == new_t_.c_cc[VERASE]) {
                if (partial_txt.length() > 0) {
                    std::cout << "\b \b" << std::flush;
                    partial_txt.pop_back();
                }
            } else if (c == '\n') {
                std::cout << "\n" << std::flush;
                break;
            } else if (c >= 32 && c <= 126) {
                std::cout << c << std::flush;
                partial_txt.push_back(c);
            }
        }
    }
}

user_choice interface::send_txt_fail(uint32_t stat_code) const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code);
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Failed to send the text: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Try again?\n"
                 "\n"
                 "[1] Yes\n"
                 "[2] No\n\n";
    return get_user_unsigned<uint32_t>();
}

void interface::clear_screen() const {
    int s = system("clear");
    (void) s;
}

std::string interface::get_user_string(size_t min_len, size_t max_len) {
    pollfd fd;
    memset(&fd, 0, sizeof(pollfd));
    fd.fd = STDIN_FILENO;
    fd.events |= POLLIN;
    std::string input;
    char c = 0;
    std::cout << "Chat262> " << std::flush;
    while (true) {
        if (poll(&fd, 1, -1) != 0) {
            std::unique_lock<std::mutex> lock(m_);
            ssize_t s = read(STDIN_FILENO, &c, 1);
            (void) s;
            if (c == new_t_.c_cc[VERASE]) {
                if (input.length() > 0) {
                    std::cout << "\b \b" << std::flush;
                    input.pop_back();
                }
            } else if (c == '\n') {
                std::cout << "\n" << std::flush;
                if (input.length() >= min_len && input.length() <= max_len) {
                    break;
                }
                std::cout << "Input must be between " << min_len << " and "
                          << max_len << " characters\n";
                input.clear();
                std::cout << "Chat262> " << std::flush;
            } else if (c >= 32 && c <= 126) {
                std::cout << c << std::flush;
                input.push_back(c);
            }
        }
    }
    return input;
}

void interface::draw_choices(const std::string& prefix,
                             const std::vector<std::string>& choices,
                             const user_choice selection) {
    clear_screen();
    std::cout << prefix << std::flush;
    for (uint32_t i = 0; i != choices.size(); ++i) {
        if (i == selection) {
            std::cout << "[X] ";
        } else {
            std::cout << "[ ] ";
        }
        std::cout << choices[i] << "\n";
    }
    std::cout << "\nUse W, A, and ENTER to navigate." << std::flush;
}
