#include "database.h"

status database::login(const std::string& username,
                       const std::string& password) {
    const std::lock_guard<std::mutex> lock(mutex_);

    // Check if the thread is already logged in
    if (threads_.find(std::this_thread::get_id()) != threads_.end()) {
        return status::error;
    }

    // Check if the user exists
    auto it = users_.find(username);
    if (it == users_.end()) {
        return status::error;
    }

    // Check if the password is correct
    user& u = (*it).second;
    if (password != u.password_) {
        return status::error;
    }
    // This thread is now dedicated to this user and the user is logged in
    threads_.insert({std::this_thread::get_id(), username});

    return status::ok;
}

status database::registration(const std::string& username,
                              const std::string& password) {
    const std::lock_guard<std::mutex> lock(mutex_);

    // Check if the username already exists or has existed before
    if (users_.find(username) != users_.end() ||
        historical_users_.find(username) != historical_users_.end()) {
        return status::error;
    }

    user u;
    u.username_ = username;
    u.password_ = password;
    users_.insert({username, u});
    historical_users_.insert(username);

    return status::ok;
}

status database::logout() {
    const std::lock_guard<std::mutex> lock(mutex_);

    // Check if the thread is already logged out
    auto thread_it = threads_.find(std::this_thread::get_id());
    if (thread_it == threads_.end()) {
        return status::error;
    }

    threads_.erase(thread_it);
    return status::ok;
}

bool database::is_logged_in() {
    const std::lock_guard<std::mutex> lock(mutex_);

    return threads_.find(std::this_thread::get_id()) != threads_.end();
}

std::vector<std::string> database::get_usernames(const std::string& pattern) {
    const std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> usernames;
    for (const auto& it : users_) {
        if (wildcard_match(pattern, it.first)) {
            usernames.push_back(it.first);
        }
    }
    return usernames;
}

status database::send_txt(const std::string& recipient_username,
                          const std::string& txt) {
    const std::lock_guard<std::mutex> lock(mutex_);

    auto thread_it = threads_.find(std::this_thread::get_id());
    if (thread_it == threads_.end()) {
        return status::error;
    }
    user& sender = users_.at((*thread_it).second);

    auto recipient_it = users_.find(recipient_username);
    if (recipient_it == users_.end()) {
        return status::error;
    }
    user& recipient = (*recipient_it).second;

    if (sender.chats_.find(recipient_username) == sender.chats_.end()) {
        sender.chats_.insert({recipient_username, {}});
    }
    if (recipient.chats_.find(sender.username_) == recipient.chats_.end()) {
        recipient.chats_.insert({sender.username_, {}});
    }

    text sender_txt;
    sender_txt.sender_ = text::sender_you;
    sender_txt.content_ = txt;
    sender.chats_.at(recipient_username).texts_.push_back(sender_txt);

    text recipient_txt;
    recipient_txt.sender_ = text::sender_other;
    recipient_txt.content_ = txt;
    recipient.chats_.at(sender.username_).texts_.push_back(recipient_txt);

    return status::ok;
}

status database::recv_txt(const std::string& sender_username, chat& c) {
    const std::lock_guard<std::mutex> lock(mutex_);

    auto thread_it = threads_.find(std::this_thread::get_id());
    if (thread_it == threads_.end()) {
        return status::error;
    }
    const user& recipient = users_.at((*thread_it).second);

    if (users_.find(sender_username) == users_.end()) {
        return status::error;
    }

    if (recipient.chats_.find(sender_username) == recipient.chats_.end()) {
        c.texts_.clear();
    } else {
        c = recipient.chats_.at(sender_username);
    }

    return status::ok;
}

status database::get_correspondents(std::vector<std::string>& usernames) {
    const std::lock_guard<std::mutex> lock(mutex_);

    auto thread_it = threads_.find(std::this_thread::get_id());
    if (thread_it == threads_.end()) {
        return status::error;
    }
    const user& this_user = users_.at((*thread_it).second);

    usernames.clear();
    for (const auto& chat_it : this_user.chats_) {
        usernames.push_back(chat_it.first);
    }
    return status::ok;
}

status database::delete_user() {
    const std::lock_guard<std::mutex> lock(mutex_);

    // Check if the thread is already logged out
    auto thread_it = threads_.find(std::this_thread::get_id());
    if (thread_it == threads_.end()) {
        return status::error;
    }

    const std::string& username = (*thread_it).second;
    const user& u = users_.at(username);

    // For every correspondent, delete their chat with the current user
    for (auto& chat_it : u.chats_) {
        const std::string& correspondent_username = chat_it.first;
        user& correspondent = users_.at(correspondent_username);
        correspondent.chats_.erase(username);
    }
    // Delete the current user
    users_.erase(username);
    // Log out the thread
    threads_.erase(thread_it);
    return status::ok;
}

bool database::wildcard_match(const std::string& pattern,
                              const std::string& target) {
    size_t target_idx = 0;
    size_t pattern_idx = 0;
    size_t last_star = static_cast<size_t>(-1);
    size_t saved_target = 0;
    while (target_idx < target.length()) {
        // If an explicit match, we are certain we can move forward with both
        // pattern and target.
        if (pattern_idx < pattern.length() &&
            target[target_idx] == pattern[pattern_idx]) {
            ++target_idx;
            ++pattern_idx;
        }
        // If a star, let's assume that the star consumes no characters. If
        // we're wrong, we'll go back. So save the star and the corresponding
        // target index.
        else if (pattern_idx < pattern.length() &&
                 pattern[pattern_idx] == '*') {
            last_star = pattern_idx;
            ++pattern_idx;
            saved_target = target_idx;
        }
        // If not a star, we backtrack to the previous star. Assume the star
        // consumes the saved target character, and just move forward. We now
        // save a new target character.
        else if (last_star != static_cast<size_t>(-1)) {
            pattern_idx = last_star + 1;
            target_idx = saved_target + 1;
            ++saved_target;
        }
        // If no star to backtrack to, then no match.
        else {
            return false;
        }
    }
    // Ignore any stars at the end of the pattern
    while (pattern_idx < pattern.length() && pattern[pattern_idx] == '*') {
        ++pattern_idx;
    }
    // If we finished all characters in the pattern, we have a match. Otherwise,
    // not.
    return pattern_idx == pattern.length();
}
