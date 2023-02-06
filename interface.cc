#include "interface.h"

#include "chat262_protocol.h"
#include "common.h"

#include <cstring>

user_choice interface::login_registration() const {
    clear_screen();
    std::cout << "\n*** Welcome to Chat262 ***\n"
                 "\n"
                 "[1] Login\n"
                 "[2] Register\n"
                 "[0] Exit\n\n";
    return get_user_unsigned<uint32_t>();
}

void interface::login(std::string& username, std::string& password) const {
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
                 "Logging in...\n";
}

user_choice interface::login_fail(uint32_t stat_code) const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code);
    std::cout << "\n*** Chat262 login ***\n"
                 "\n"
                 "Login failed: ";
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

void interface::registration(std::string& username,
                             std::string& password) const {
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
                 "Registering...\n";
}

user_choice interface::registration_success() const {
    clear_screen();
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Registration successful! You can now use your username and "
                 "password to log in.\n"
                 "\n"
                 "[1] Main menu\n\n";
    return get_user_unsigned<uint32_t>();
}

user_choice interface::registration_fail(uint32_t stat_code) const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code);
    std::cout << "\n*** Chat262 registration ***\n"
                 "\n"
                 "Registration failed: ";
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

user_choice interface::main_menu() const {
    clear_screen();
    std::cout << "\n*** Chat262 main menu ***\n"
                 "\n"
                 "[1] Chats (coming soon)\n"
                 "[2] List all accounts\n"
                 "[0] Log out\n\n";
    return get_user_unsigned<uint32_t>();
}

user_choice interface::list_accounts_success(
    const std::vector<std::string>& usernames) const {
    clear_screen();
    std::cout << "\n*** Chat262 accounts ***\n"
                 "\n"
                 "The following usernames are registered with Chat262:\n\n";
    for (size_t i = 0; i != usernames.size(); ++i) {
        std::cout << " " << i + 1 << ".\t" << usernames[i] << "\n";
    }
    std::cout << "\n"
                 "[1] Main menu\n\n";
    return get_user_unsigned<uint32_t>();
}

user_choice interface::list_accounts_fail(uint32_t stat_code) const {
    clear_screen();
    const char* description = chat262::status_code_lookup(stat_code);
    std::cout << "\n*** Chat262 accounts ***\n"
                 "\n"
                 "Failed to get accounts: ";
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

std::string interface::get_user_string(size_t min_len, size_t max_len) const {
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
