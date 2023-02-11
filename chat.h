#ifndef _CHAT_H_
#define _CHAT_H_

#include <string>
#include <vector>

struct text {
    static constexpr uint8_t sender_you = 0;
    static constexpr uint8_t sender_other = 1;

    uint8_t sender_;
    std::string content_;
};

struct chat {
    std::vector<text> texts_;
};


#endif
