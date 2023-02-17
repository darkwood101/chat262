#ifndef _COMMON_H_
#define _COMMON_H_

#include <climits>
#include <cstdlib>
#include <limits>
#include <string>

enum class status { ok, error };

bool wildcard_match(const std::string& pattern, const std::string& target);

#endif
