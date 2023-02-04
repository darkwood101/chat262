#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <chrono>
#include <cstdio>

class logger {
public:
    logger() = default;
    logger(const logger&) = delete;
    logger(logger&&) = delete;
    logger& operator=(const logger&) = delete;
    logger& operator=(logger&&) = delete;

    template <typename... args>
    static void log_out(args... a);

    template <typename... args>
    static void log_err(args... a);

private:
    static void add_prefix(FILE* out);
};

template <typename... args>
void logger::log_out(args... a) {
    add_prefix(stdout);
    fprintf(stdout, a...);
}

template <typename... args>
void logger::log_err(args... a) {
    add_prefix(stderr);
    fprintf(stderr, a...);
}

#endif
