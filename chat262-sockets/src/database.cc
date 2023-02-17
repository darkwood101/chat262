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

    // Check if the username already exists
    if (users_.find(username) != users_.end()) {
        return status::error;
    }

    user u;
    u.username_ = username;
    u.password_ = password;
    users_.insert({username, u});

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
