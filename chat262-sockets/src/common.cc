#include "common.h"

bool wildcard_match(const std::string& pattern, const std::string& target) {
    size_t target_idx = 0;
    size_t pattern_idx = 0;
    size_t last_star = static_cast<size_t>(-1);
    size_t saved_target = 0;
    while (target_idx < target.size() && pattern_idx < pattern.size()) {
        // If an explicit match, we are certain we can move forward with both
        // pattern and target.
        if (target[target_idx] == pattern[pattern_idx]) {
            ++target_idx;
            ++pattern_idx;
        }
        // If a star, let's assume that the star consumes no characters. If
        // we're wrong, we'll go back. So save the star and the corresponding
        // target index.
        else if (pattern[pattern_idx] == '*') {
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
