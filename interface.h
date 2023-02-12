#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "chat.h"
#include "common.h"

#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <termios.h>
#include <vector>

using user_choice = uint32_t;
inline constexpr user_choice ESCAPE = static_cast<user_choice>(-1);

enum class screen_type {
    login_registration,
    login,
    login_fail,
    registration,
    registration_success,
    registration_fail,
    main_menu,
    list_accounts,
    list_accounts_success,
    list_accounts_fail,
    open_chats,
    open_chats_success,
    open_chats_fail,
    new_chat,
    recv_txt,
    recv_txt_fail,
    send_txt,
    send_txt_fail,
    exit
};

class interface {
public:
    interface();
    ~interface();

    interface(const interface&) = delete;
    interface& operator=(const interface&) = delete;
    interface(interface&&) = delete;
    interface& operator=(interface&&) = delete;

    user_choice make_selection(const std::string& prefix,
                               const std::vector<std::string>& choices);
    void wait_anykey();

    user_choice login_registration();
    void login(std::string& username, std::string& password, bool& hit_escape);
    user_choice login_fail(uint32_t stat_code);
    void registration(std::string& username,
                      std::string& password,
                      bool& hit_escape);
    void registration_success();
    user_choice registration_fail(uint32_t stat_code);
    user_choice main_menu(const std::string& username);
    void list_accounts();
    void list_accounts_success(const std::vector<std::string>& usernames);
    user_choice list_accounts_fail(uint32_t stat_code);
    void open_chats();
    user_choice open_chats_success(const std::vector<std::string>& usernames);
    user_choice open_chats_fail(uint32_t stat_code);
    void new_chat(std::string& correspondent, bool& hit_escape);
    void recv_txt();
    void recv_txt_fail(uint32_t stat_code);
    void draw_send_txt(const std::string& me,
                       const std::string& correspondent,
                       const chat& c,
                       const std::string& partial_txt);
    void prompt_send_txt(std::string& partial_txt,
                         std::mutex& m,
                         bool& hit_escape);
    user_choice send_txt_fail(uint32_t stat_code) const;
    screen_type next_;

private:
    void clear_screen() const;

    template <typename T>
    T get_user_unsigned() const;

    std::string get_user_string(size_t min_len,
                                size_t max_len,
                                bool& hit_escape);
    void draw_choices(const std::string& prefix,
                      const std::vector<std::string>& choices,
                      const user_choice selection);

    std::mutex m_;
    termios old_t_;
    termios new_t_;
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
