#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "chat.h"
#include "client.h"
#include "common.h"

#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <termios.h>
#include <vector>

using user_choice = uint32_t;
inline constexpr user_choice ESCAPE = static_cast<user_choice>(-1);

class interface {
public:
    // Initializes terminal to non-canonical mode
    interface();
    // Resets the terminal back to canonical mode
    ~interface();

    status start(const uint32_t n_ip_addr);

    // Because of terminal state, we don't want copying or moving
    interface(const interface&) = delete;
    interface& operator=(const interface&) = delete;
    interface(interface&&) = delete;
    interface& operator=(interface&&) = delete;

private:
    // A structure describing a keypress.
    // If `type_` is `regular`, then `c_` stores the ASCII code of the
    // character. Otherwise, `c_` is invalid.
    struct keypress {
        char c_;
        enum class type : uint8_t {
            regular,
            escape,
            enter,
            backspace,
            uparrow,
            downarrow,
            unknown
        } type_;
    };
    // Read a keypress from the user. Will block until there is a keypress
    // available.
    keypress read_keypress() const;

    // Wait until any key is pressed
    void wait_anykey() const;

    // Get a selection from the user. The selection is an integer between 0 and
    // `choices.size() - 1`, inclusive. Will print `prefix` before printing the
    // choices.
    user_choice make_selection(const std::string& prefix,
                               const std::vector<std::string>& choices) const;

    // Get a string from the user between `min_len` and `max_len`, inclusive.
    // The user will keep getting asked to reinput if the length doesn't work.
    // If a user hits ESC, partial input will be returned and `hit_escape_` is
    // set to `true`. If a user enters a valid string, `hit_escape_` is set to
    // `false`.
    std::string get_user_string(size_t min_len, size_t max_len);

    void clear_screen() const;

    // Get a choice from the starting application menu
    user_choice login_registration();

    // Fill `username_` and `password_` from the user, subject to registration
    // length restrictions. `hit_escape_` indicates if ESC was pressed. If
    // `true`, `username_` and `password_` may be partially filled.
    void login();

    // Display a failed login screen, reading the reason from `stat_code_`.
    // Ask the user if they want to try again.
    user_choice login_fail() const;

    // Fill `username_` and `password_` from the user, subject to registration
    // length restrictions. `hit_escape_` indicates if ESC was pressed. If
    // `true`, `username_` and `password_` may be partially filled.
    void registration();

    // Display brief registration success screen, while logging the user in
    void registration_success() const;

    // Display a failed login screen, reading the reason from `stat_code_`.
    // Ask the user if they want to try again.
    user_choice registration_fail() const;

    // Get a choice from the main menu
    user_choice main_menu() const;

    // Fill `pattern_` from the user.
    // `hit_escape_` indicates if ESC was pressed. If `true`, `pattern_` may be
    // partially filled.
    void search_accounts();

    // Display the matched usernames from search.
    // Wait until the user presses anything.
    void search_accounts_success() const;

    // Display a failed search accounts screen, reading the reason from
    // `stat_code_`. Wait until the user presses anything.
    void search_accounts_fail() const;

    // Display brief opening chats screen, while retrieving the chats
    void open_chats() const;

    // Get a choice from the chats menu
    user_choice open_chats_success() const;

    // Display a failed open chats screen, reading the reason from `stat_code_`.
    // Wait until the user presses anything.
    void open_chats_fail() const;

    // Fill `correspondent_` from the user.
    // `hit_escape_` indicates if ESC was pressed. If `true`, `pattern_` may be
    // partially filled.
    void new_chat();

    // Display brief receiving texts screen, while receiving the texts
    void recv_txt() const;

    // Display a failed receiving texts screen, reading the reason from
    // `stat_code_`. Wait until the user presses anything.
    void recv_txt_fail() const;

    // Draw `curr_chat_` and the partially typed text from `partial_txt_`.
    void draw_send_txt() const;

    // Get a text from the user, and put it char-by-char into `partial_txt_`.
    // Standard output is protected by `mutex_`, so that output from the main
    // thread calling `get_user_txt()` and the listener thread calling
    // `draw_send_txt()` is not mixed.
    void get_user_txt();

    // Display a failed send text screen, reading the reason from `stat_code_`.
    // Wait until the user presses anything.
    void send_txt_fail() const;

    // Confirm account deletion from the user
    user_choice delete_account() const;

    // Display a successful account deletion screen.
    // Wait until the user presses anything.
    void delete_account_success() const;

    // Display a failed account deletion screen, reading the reason from
    // `stat_code_`. Wait until the user presses anything.
    void delete_account_fail() const;

    // Type of screen to be shown next in the interface
    enum class screen_type {
        login_registration,
        login,
        login_fail,
        registration,
        registration_success,
        registration_fail,
        main_menu,
        search_accounts,
        search_accounts_success,
        search_accounts_fail,
        open_chats,
        open_chats_success,
        open_chats_fail,
        new_chat,
        recv_txt,
        recv_txt_fail,
        send_txt,
        send_txt_fail,
        delete_account,
        delete_account_success,
        delete_account_fail,
        exit
    } next_;

    // The state of the terminal after setting it to non-canonical mode
    termios new_t_;

    // The state of the terminal before setting it to non-canonical mode
    termios old_t_;

    // The client connected to the interface
    client client_;

    // Status code returned by the previous client operation
    uint32_t stat_code_;

    // Chat retrieved by the previous `recv_txt` client operation
    chat curr_chat_;

    // The username for `register` and `login` client operations
    std::string username_;

    // The password for `register` and `login` client operations
    std::string password_;

    // The matching pattern for the `search_accounts` client operation
    std::string pattern_;

    // The list of usernames returned by the `search_accounts` client operation
    std::vector<std::string> matched_usernames_;

    // The list of correspondent usernames returned by the `retrieve
    // correspondents` client operation
    std::vector<std::string> all_correspondents_;

    // The username of the correspondent for the `recv_txt` and `send_txt`
    // client operations
    std::string correspondent_;

    // Partially typed in text
    std::string partial_txt_;

    // Elements for synchronizing the listener thread and the main interface
    // thread
    std::mutex mutex_;
    std::condition_variable listener_cv_;
    bool listener_should_exit_;

    // Function for the listener thread.
    // This thread sends periodic receive texts request, and updates the send
    // text screen if any new texts arrive.
    void background_listener();

    // True if the user pressed escape
    bool hit_escape_;
};

#endif
