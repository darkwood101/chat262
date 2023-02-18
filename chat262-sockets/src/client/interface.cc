#include "interface.h"

#include "chat262_protocol.h"
#include "client.h"
#include "common.h"

#include <cerrno>
#include <cstring>
#include <poll.h>
#include <thread>
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

status interface::start(const uint32_t n_ip_addr) {
    status s;
    s = client_.connect_server(n_ip_addr);
    if (s != status::ok) {
        return s;
    }

    // Begin with the initial screen
    next_ = screen_type::login_registration;
    while (true) {
        switch (next_) {
        case screen_type::login_registration: {
            user_choice choice = login_registration();
            switch (choice) {
            case 0:
                next_ = screen_type::login;
                break;
            case 1:
                next_ = screen_type::registration;
                break;
            case 2:
            case ESCAPE:
                next_ = screen_type::exit;
                break;
            default:
                break;
            }
            break;
        }

        case screen_type::login:
            login();
            if (hit_escape_) {
                next_ = screen_type::login_registration;
                break;
            }
            stat_code_ = client_.login(username_, password_);
            if (stat_code_ == chat262::status_code_ok) {
                next_ = screen_type::main_menu;
            } else {
                next_ = screen_type::login_fail;
            }
            break;

        case screen_type::login_fail: {
            user_choice choice = login_fail();
            switch (choice) {
            case 0:
                next_ = screen_type::login;
                break;
            case 1:
            case ESCAPE:
                next_ = screen_type::login_registration;
                break;
            default:
                break;
            }
            break;
        }

        case screen_type::registration:
            registration();
            if (hit_escape_) {
                next_ = screen_type::login_registration;
                break;
            }
            stat_code_ = client_.registration(username_, password_);
            if (stat_code_ == chat262::status_code_ok) {
                next_ = screen_type::registration_success;
            } else {
                next_ = screen_type::registration_fail;
            }
            break;

        case screen_type::registration_success:
            registration_success();
            stat_code_ = client_.login(username_, password_);
            if (stat_code_ == chat262::status_code_ok) {
                next_ = screen_type::main_menu;
            } else {
                next_ = screen_type::login_fail;
            }
            break;

        case screen_type::registration_fail: {
            user_choice choice = registration_fail();
            switch (choice) {
            case 0:
                next_ = screen_type::registration;
                break;
            case 1:
            case ESCAPE:
                next_ = screen_type::login_registration;
                break;
            default:
                break;
            }
            break;
        }

        case screen_type::main_menu: {
            user_choice choice = main_menu();
            switch (choice) {
            case 0:
                next_ = screen_type::open_chats;
                break;
            case 1:
                next_ = screen_type::search_accounts;
                break;
            case 2:
                next_ = screen_type::delete_account;
                break;
            case 3:
            case ESCAPE:
                stat_code_ = client_.logout();
                if (stat_code_ == chat262::status_code_ok) {
                    next_ = screen_type::login_registration;
                }
                // TODO: handle logout fail
            default:
                break;
            }
            break;
        }

        case screen_type::open_chats:
            open_chats();
            stat_code_ = client_.recv_correspondents(all_correspondents_);
            if (stat_code_ == chat262::status_code_ok) {
                next_ = screen_type::open_chats_success;
            } else {
                next_ = screen_type::open_chats_fail;
            }
            break;

        case screen_type::open_chats_success: {
            user_choice choice = open_chats_success();
            if (choice == 0) {
                next_ = screen_type::new_chat;
            } else if (choice == 1 || choice == ESCAPE) {
                next_ = screen_type::main_menu;
            } else {
                correspondent_ = all_correspondents_[choice - 2];
                next_ = screen_type::recv_txt;
            }
            break;
        }

        case screen_type::open_chats_fail:
            open_chats_fail();
            next_ = screen_type::main_menu;
            break;

        case screen_type::new_chat:
            new_chat();
            if (hit_escape_) {
                next_ = screen_type::open_chats;
                break;
            }
            next_ = screen_type::recv_txt;
            break;

        case screen_type::recv_txt:
            recv_txt();
            stat_code_ = client_.recv_txt(correspondent_, curr_chat_);
            if (stat_code_ == chat262::status_code_ok) {
                next_ = screen_type::send_txt;
            } else {
                next_ = screen_type::recv_txt_fail;
            }
            break;

        case screen_type::recv_txt_fail:
            recv_txt_fail();
            next_ = screen_type::open_chats;
            break;

        case screen_type::send_txt: {
            // Draw previous messages
            partial_txt_.clear();
            draw_send_txt();

            // Start a background listener thread to periodically ask for new texts
            listener_should_exit_ = false;
            std::thread listener(&interface::background_listener, this);

            // Get the text from the user
            get_user_txt();

            // Tear down the background listener
            std::unique_lock<std::mutex> lock(mutex_);
            listener_should_exit_ = true;
            listener_cv_.notify_all();
            lock.unlock();
            listener.join();

            if (hit_escape_) {
                next_ = screen_type::open_chats;
                break;
            }
            // Actually send the text
            stat_code_ = client_.send_txt(correspondent_, partial_txt_);
            if (stat_code_ != chat262::status_code_ok) {
                next_ = screen_type::send_txt_fail;
                break;
            }
            // Receive updated texts
            stat_code_ = client_.recv_txt(correspondent_, curr_chat_);
            if (stat_code_ != chat262::status_code_ok) {
                next_ = screen_type::recv_txt_fail;
            }
            break;
        }
        case screen_type::send_txt_fail:
            send_txt_fail();
            next_ = screen_type::open_chats;
            break;

        case screen_type::search_accounts:
            search_accounts();
            if (hit_escape_) {
                next_ = screen_type::main_menu;
                break;
            }
            stat_code_ = client_.list_accounts(pattern_, matched_usernames_);
            if (stat_code_ == chat262::status_code_ok) {
                next_ = screen_type::search_accounts_success;
            } else {
                next_ = screen_type::search_accounts_fail;
            }
            break;

        case screen_type::search_accounts_success:
            search_accounts_success();
            next_ = screen_type::search_accounts;
            break;

        case screen_type::search_accounts_fail:
            search_accounts_fail();
            next_ = screen_type::main_menu;
            break;

        case screen_type::delete_account: {
            user_choice choice = delete_account();
            switch (choice) {
            case 0:
            case ESCAPE:
                next_ = screen_type::main_menu;
                break;
            case 1:
                stat_code_ = client_.delete_account();
                if (stat_code_ == chat262::status_code_ok) {
                    next_ = screen_type::delete_account_success;
                } else {
                    next_ = screen_type::delete_account_fail;
                }
                break;
            default:
                break;
            }
            break;
        }

        case screen_type::delete_account_success:
            delete_account_success();
            next_ = screen_type::login_registration;
            break;

        case screen_type::delete_account_fail:
            delete_account_fail();
            next_ = screen_type::main_menu;
            break;

        case screen_type::exit:
            clear_screen();
            return status::ok;
        }
    }
}

void interface::background_listener() {
    using std::chrono::high_resolution_clock;
    using std::chrono::seconds;
    std::unique_lock<std::mutex> lock(mutex_);
    while (!listener_should_exit_) {
        auto end_time = high_resolution_clock::now() + seconds(2);
        auto cv_status = listener_cv_.wait_until(lock, end_time);
        if (cv_status != std::cv_status::timeout) {
            continue;
        }
        lock.unlock();
        size_t prev_size = curr_chat_.texts_.size();
        client_.recv_txt(correspondent_, curr_chat_);
        lock.lock();
        if (prev_size == curr_chat_.texts_.size()) {
            continue;
        }
        draw_send_txt();
    }
}

interface::keypress interface::read_keypress() const {
    pollfd fd;
    memset(&fd, 0, sizeof(pollfd));
    fd.fd = STDIN_FILENO;
    fd.events |= POLLIN;

    keypress k;

    // Wait until there is something available. This should always return 1.
    if (poll(&fd, 1, -1) == 0) {
        k.type_ = keypress::type::unknown;
        return k;
    }

    char c;
    ssize_t s = read(STDIN_FILENO, &c, 1);
    // This shouldn't happen
    if (s != 1) {
        k.type_ = keypress::type::unknown;
        return k;
    }

    // Backspace
    if (c == new_t_.c_cc[VERASE]) {
        k.type_ = keypress::type::backspace;
        return k;
    }
    // Enter
    if (c == '\n') {
        k.type_ = keypress::type::enter;
        return k;
    }
    // Normal character
    if (c >= 32 && c <= 126) {
        k.type_ = keypress::type::regular;
        k.c_ = c;
        return k;
    }
    // A special character. Special sequences begin with 27 (ESC).
    if (c == 27) {
        // If there are no more characters available, it's ESC
        if (poll(&fd, 1, 0) == 0) {
            k.type_ = keypress::type::escape;
            return k;
        }
        char cc[2];
        s = read(STDIN_FILENO, cc, 2);
        // If we don't read exactly 2, it's not arrows
        if (s != 2) {
            k.type_ = keypress::type::unknown;
            return k;
        }
        // If the sequence is ESC-[-A, it's up arrow
        if (cc[0] == '[' && cc[1] == 'A') {
            k.type_ = keypress::type::uparrow;
            return k;
        }
        // If the sequence is ESC-[-B, it's down arrow
        if (cc[0] == '[' && cc[1] == 'B') {
            k.type_ = keypress::type::downarrow;
            return k;
        }
    }
    // This shouldn't happen
    k.type_ = keypress::type::unknown;
    return k;
}

void interface::wait_anykey() const {
    read_keypress();
}

user_choice interface::make_selection(
    const std::string& prefix,
    const std::vector<std::string>& choices) const {
    user_choice selection = 0;
    bool ought_to_redraw = true;
    while (true) {
        if (ought_to_redraw) {
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
            std::cout << "\nUse Up and Down arrows to navigate. Press ENTER to "
                         "confirm. Press ESC to go back."
                      << std::flush;
            ought_to_redraw = false;
        }

        keypress k = read_keypress();
        switch (k.type_) {
        case keypress::type::escape:
            return ESCAPE;
        case keypress::type::uparrow:
            // Wrap around if needed
            if (selection == 0) {
                selection = choices.size() - 1;
            } else {
                selection = (selection - 1) % choices.size();
            }
            ought_to_redraw = true;
            break;
        case keypress::type::downarrow:
            selection = (selection + 1) % choices.size();
            ought_to_redraw = true;
            break;
        case keypress::type::enter:
            std::cout << "\n" << std::flush;
            return selection;
        case keypress::type::regular:
            if (k.c_ == 'w' || k.c_ == 'W') {
                if (selection == 0) {
                    selection = choices.size() - 1;
                } else {
                    selection = (selection - 1) % choices.size();
                }
                ought_to_redraw = true;
            } else if (k.c_ == 's' || k.c_ == 'S') {
                selection = (selection + 1) % choices.size();
                ought_to_redraw = true;
            }
            break;
        default:
            break;
        }
    }
}

std::string interface::get_user_string(size_t min_len, size_t max_len) {
    std::string input;
    std::cout << "Chat262> " << std::flush;
    while (true) {
        keypress k = read_keypress();
        switch (k.type_) {
        // Erase from the buffer if backspace
        case keypress::type::backspace:
            if (input.length() > 0) {
                std::cout << "\b \b" << std::flush;
                input.pop_back();
            }
            break;
        // Check length if enter
        case keypress::type::enter:
            std::cout << "\n" << std::flush;
            if (input.length() >= min_len && input.length() <= max_len) {
                hit_escape_ = false;
                return input;
            }
            std::cout << "Input must be between " << min_len << " and "
                      << max_len << " characters\n";
            input.clear();
            std::cout << "Chat262> " << std::flush;
            break;
        // Push into buffer if regular
        case keypress::type::regular:
            std::cout << k.c_ << std::flush;
            input.push_back(k.c_);
            break;
        // Return immediately if escape was hit
        case keypress::type::escape:
            hit_escape_ = true;
            return input;
        // Ignore anything else
        default:
            break;
        }
    }
}

void interface::clear_screen() const {
    int s = system("clear");
    (void) s;
}

user_choice interface::login_registration() {
    return make_selection("\n*** Welcome to Chat262 ***\n"
                          "\n",
                          {"Login", "Register", "Exit"});
}

void interface::login() {
    clear_screen();
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Please enter your username. Press ESC to cancel.\n\n";
    username_ = get_user_string(4, 40);
    if (hit_escape_) {
        return;
    }
    clear_screen();
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Username: "
              << username_
              << "\n\n"
                 "Please enter your password. Press ESC to cancel.\n\n";
    password_ = get_user_string(4, 60);
    if (hit_escape_) {
        return;
    }
    clear_screen();
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Username: "
              << username_
              << "\n"
                 "Password: "
              << password_
              << "\n\n"
                 "Logging in..."
              << std::flush;
}

user_choice interface::login_fail() const {
    std::string prefix = "\n*** Chat262 login ***\n"
                         "\n"
                         "Login failed: ";
    const char* description = chat262::status_code_lookup(stat_code_);
    if (strcmp(description, "Unknown") == 0) {
        prefix.append(description);
        prefix.append(" (status code " + std::to_string(stat_code_) + ")\n");
    } else {
        prefix.append(description);
    }
    prefix.append("\n"
                  "\n"
                  "Try again?\n"
                  "\n");
    return make_selection(prefix, {"Yes", "No"});
}

void interface::registration() {
    clear_screen();
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Please enter a username (between 4 and 40 characters). Must "
                 "not contain '*' or whitespace.\nPress ESC to cancel.\n\n";
    do {
        username_ = get_user_string(4, 40);
        if (username_.find_first_of("* ") == std::string::npos) {
            break;
        }
        std::cout << "Input must not contain '*' or whitespace\n";
    } while (true);
    if (hit_escape_) {
        return;
    }
    clear_screen();
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Username: "
              << username_
              << "\n\n"
                 "Please enter your password (between 4 and 60 characters). "
                 "Press ESC to cancel.\n\n";
    password_ = get_user_string(4, 60);
    if (hit_escape_) {
        return;
    }
    clear_screen();
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Username: "
              << username_
              << "\n"
                 "Password: "
              << password_
              << "\n\n"
                 "Registering..."
              << std::flush;
}

void interface::registration_success() const {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Registration successful! Logging you in..."
              << std::flush;
}

user_choice interface::registration_fail() const {
    std::string prefix = "\n*** Chat262 login ***\n"
                         "\n"
                         "Registration failed: ";
    const char* description = chat262::status_code_lookup(stat_code_);
    if (strcmp(description, "Unknown") == 0) {
        prefix.append(description);
        prefix.append(" (status code " + std::to_string(stat_code_) + ")\n");
    } else {
        prefix.append(description);
    }
    prefix.append("\n"
                  "\n"
                  "Try again?\n"
                  "\n");
    return make_selection(prefix, {"Yes", "No"});
}

user_choice interface::main_menu() const {
    std::string prefix = "\n*** Chat 262 main menu ***\n"
                         "\n"
                         "Welcome, ";
    prefix.append(username_);
    prefix.append("\n\n");
    return make_selection(
        prefix,
        {"Chats", "Search users", "Delete account", "Log out"});
}

void interface::search_accounts() {
    clear_screen();
    std::cout << "\n*** Chat 262 user search ***\n"
                 "\n"
                 "Please enter the pattern to search for (* will match any "
                 "number of any characters).\n"
                 "To list all users, search for '*'.\n"
                 "Press ESC to cancel.\n\n";
    pattern_ = get_user_string(1, 40);
    clear_screen();
    std::cout << "\n*** Chat 262 user search ***\n"
                 "\n"
                 "Retrieving the list of users..."
              << std::flush;
}

void interface::search_accounts_success() const {
    clear_screen();
    std::cout << "\n*** Chat262 user search ***\n"
                 "\n";

    if (matched_usernames_.size() > 0) {
        std::cout << "The following usernames match the pattern \"" << pattern_
                  << "\":\n\n";
        for (size_t i = 0; i != matched_usernames_.size(); ++i) {
            std::cout << " " << i + 1 << ".\t" << matched_usernames_[i] << "\n";
        }
    } else {
        std::cout << "No usernames match the pattern \"" << pattern_ << "\".\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

void interface::search_accounts_fail() const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code_);
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Retrieving users failed: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code_ << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

void interface::open_chats() const {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Retrieving your chats..."
              << std::flush;
}

user_choice interface::open_chats_success() const {
    std::vector<std::string> usernames_copy = all_correspondents_;
    if (usernames_copy.size() > 0) {
        usernames_copy.insert(usernames_copy.begin(),
                              1,
                              "Go back\n\nYour chats:\n");
    } else {
        usernames_copy.insert(
            usernames_copy.begin(),
            1,
            "Go back\n\nYou have no chats. Start a new one!\n");
    }
    usernames_copy.insert(usernames_copy.begin(), 1, "Start a new chat");
    return make_selection("\n*** Chat 262 ***\n\n", usernames_copy);
}

void interface::open_chats_fail() const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code_);
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Retrieving chats failed: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code_ << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

void interface::new_chat() {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Enter the username of the person you wish to chat with.\n\n";
    correspondent_ = get_user_string(4, 40);
}

void interface::recv_txt() const {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Opening the requested chat..."
              << std::flush;
}

void interface::recv_txt_fail() const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code_);
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Failed to retrieve the chat: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code_ << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

void interface::draw_send_txt() const {
    clear_screen();
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Your chat with "
              << correspondent_
              << "\n\n"
                 "============================================================="
                 "===================\n";
    if (curr_chat_.texts_.size() == 0) {
        std::cout << "This chat is empty. Start chatting now!\n";
    }

    for (const text& t : curr_chat_.texts_) {
        if (t.sender_ == text::sender_you) {
            std::cout << username_ << " (you): " << t.content_ << "\n\n";
        } else {
            std::cout << correspondent_ << ": " << t.content_ << "\n\n";
        }
    }
    std::cout << "============================================================="
                 "===================\n\n";
    std::cout << "Type your message and press ENTER when you're done. Press "
                 "ESC to cancel.\n\n";
    std::cout << "Chat262> " << partial_txt_ << std::flush;
}

void interface::get_user_txt() {
    while (true) {
        keypress k = read_keypress();
        // Synchronize output with the listener thread
        std::unique_lock<std::mutex> lock(mutex_);
        switch (k.type_) {
        case keypress::type::backspace:
            if (partial_txt_.length() > 0) {
                std::cout << "\b \b" << std::flush;
                partial_txt_.pop_back();
            }
            break;
        case keypress::type::enter:
            std::cout << "\n" << std::flush;
            hit_escape_ = false;
            return;
        case keypress::type::regular:
            std::cout << k.c_ << std::flush;
            break;
        case keypress::type::escape:
            hit_escape_ = true;
            return;
        default:
            break;
        }
    }
}

void interface::send_txt_fail() const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code_);
    std::cout << "\n*** Chat262 ***\n"
                 "\n"
                 "Failed to send the text: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code_ << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}

user_choice interface::delete_account() const {
    return make_selection("\n*** Chat 262 account deletion ***\n"
                          "\n"
                          "WARNING: You are about to delete your account. This "
                          "cannot be undone. Are you sure?\n"
                          "\n",
                          {"No", "Yes"});
}

void interface::delete_account_success() const {
    clear_screen();
    std::cout << "\n*** Chat 262 account deletion ***\n"
                 "\n"
                 "Account successfully deleted.\n"
                 "\n"
                 "Press any key to continue..."
              << std::flush;
    wait_anykey();
}

void interface::delete_account_fail() const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code_);
    std::cout << "\n*** Chat 262 account deletion ***\n"
                 "\n"
                 "Failed to delete the account: ";
    if (strcmp(description, "Unknown") == 0) {
        std::cout << description << " (status code " << stat_code_ << ")\n";
    } else {
        std::cout << description << "\n";
    }
    std::cout << "\n"
                 "Press any key to go back..."
              << std::flush;
    wait_anykey();
}
