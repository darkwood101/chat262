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

std::vector<std::string> database::get_usernames() {
    const std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> usernames;
    for (const auto& it : users_) {
        usernames.push_back(it.first);
    }
    return usernames;
}
