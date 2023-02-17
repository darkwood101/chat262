#ifndef _COMMON_H_
#define _COMMON_H_

#include <climits>
#include <cstdlib>
#include <limits>
#include <string>

enum class status { ok, error };

bool wildcard_match(const std::string& pattern, const std::string& target);

template <typename T>
status str_to_unsigned(char const* str, T& res) {
    char* endptr;
    errno = 0;
    unsigned long long num = strtoull(str, &endptr, 10);
    if ((errno == ERANGE && num == ULLONG_MAX) || endptr == str ||
        *endptr != '\0' || num > std::numeric_limits<T>::max()) {
        return status::error;
    }
    res = static_cast<T>(num);
    return status::ok;
}

template <typename T>
status str_to_unsigned(std::string const& str, T& res) {
    return str_to_unsigned<T>(str.c_str(), res);
}

#endif
