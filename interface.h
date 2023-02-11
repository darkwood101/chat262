#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "chat.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using user_choice = uint32_t;

enum class screen_type {
    login_registration,
    login,
    login_fail,
    registration,
    registration_success,
    registration_fail,
    main_menu,
    list_accounts,
    open_chat,
    open_chat_fail,
    send_txt,
    send_txt_fail,
    exit
};

class interface {
public:
    user_choice login_registration() const;
    void login(std::string& username, std::string& password) const;
    user_choice login_fail(uint32_t stat_code) const;
    void registration(std::string& username, std::string& password) const;
    user_choice registration_success() const;
    user_choice registration_fail(uint32_t stat_code) const;
    user_choice main_menu() const;
    user_choice list_accounts_success(
        const std::vector<std::string>& usernames) const;
    user_choice list_accounts_fail(uint32_t stat_code) const;
    void open_chat(std::string& username) const;
    user_choice open_chat_fail(uint32_t stat_code) const;
    void send_txt(const std::string& me,
                  const std::string& correspondent,
                  const chat& c,
                  std::string& txt) const;
    user_choice send_txt_fail(uint32_t stat_code) const;
    screen_type next_;

private:
    void clear_screen() const;

    template <typename T>
    T get_user_unsigned() const;

    std::string get_user_string(size_t min_len, size_t max_len) const;
};

template <typename T>
T interface::get_user_unsigned() const {
    std::string line;
    T num;
    while (true) {
        std::cout << "Chat262> " << std::flush;
        std::getline(std::cin, line);
        if (std::cin.eof()) {
            std::cout << "\n";
            std::cin.clear();
            clearerr(stdin);
        }
        if (str_to_unsigned<T>(line, num) == status::ok) {
            break;
        }
    }
    return num;
}

#endif
