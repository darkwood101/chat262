#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <chrono>
#include <cstdio>

class logger {
public:
    logger() = delete;

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
