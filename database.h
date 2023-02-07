#ifndef _DATABASE_H_
#define _DATABASE_H_

#include "common.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class database {
public:
    // Attempts to log in with `username` and `password`..
    // @return ok      - `username` and `password` match an existing user.
    //                   This user becomes logged in and the executing thread
    //                   is dedicated to the user.
    // @return error   - `username` doesn't exist or `password` is
    //                   incorrect.
    // @return error   - The executing thread is already logged in.
    //                   The caller can check if this is the case by calling
    //                   `is_logged_in` before calling `login`.
    status login(const std::string& username, const std::string& password);

    // Attempts to register a user with `username` and `password`.
    // @return ok      - A user is successfully created.
    // @return error   - `username` already exists.
    status registration(const std::string& username,
                        const std::string& password);

    // Logs out the current thread's user.
    // @return ok    - The user is successfully logged out.
    // @return error - This thread does not have an associated user (already
    //                 logged out).
    status logout();

    // Checks if the current thread has an associated user.
    bool is_logged_in();

    // Returns a vector of all usernames.
    std::vector<std::string> get_usernames();

private:
    struct user {
        std::string username_;
        std::string password_;
    };

    // Protects everything. We don't care about the performance, so we go for
    // coarse-grained strategy.
    std::mutex mutex_;

    // Map from usernames to users
    std::map<std::string, user> users_;

    // After a user logs in, each thread is in charge of exactly one user.
    // `threads_` maps unique thread IDs to usernames, so that the server
    // doesn't have to keep track of usernames. If the thread ID is in the map,
    // the user is logged in.
    std::unordered_map<std::thread::id, std::string> threads_;
};

#endif
