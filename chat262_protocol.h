#ifndef _CHAT262_PROTOCOL_
#define _CHAT262_PROTOCOL_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace chat262 {

static constexpr uint16_t version = 0x1;
static constexpr uint16_t port = 45171;

enum message_type : uint16_t { registration = 0x0, login = 0x1, logout = 0x2 };

struct message_header {
    uint16_t version_;
    uint16_t type_;
    uint32_t body_len_;

    static message_header deserialize(const std::vector<uint8_t>& hdr_data);
};

struct message {
    message_header hdr_;
    uint8_t body_[];
};

struct registration_body {
    uint32_t user_len_;
    uint32_t pass_len_;
    uint8_t user_pass_[];

    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);
    static void deserialize(const std::vector<uint8_t>& body,
                            std::string& username,
                            std::string& password);
};

struct login_body {
    uint32_t user_len_;
    uint32_t pass_len_;
    uint8_t user_pass_[];

    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);
    static void deserialize(const std::vector<uint8_t>& body,
                            std::string& username,
                            std::string& password);
};

struct logout_body {};

static_assert(sizeof(message_header) == 8);
static_assert(sizeof(message) == 8);
static_assert(offsetof(message, body_) == 8);


};  // namespace chat262

#endif
