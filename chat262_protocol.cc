#include "chat262_protocol.h"

#include <cstdlib>
#include <cstring>
#include <endian.h>

namespace chat262 {

message_header message_header::deserialize(
    const std::vector<uint8_t>& hdr_data) {
    message_header hdr;
    memcpy(&hdr, hdr_data.data(), sizeof(message_header));
    hdr.version_ = le16toh(hdr.version_);
    hdr.type_ = le16toh(hdr.type_);
    hdr.body_len_ = le32toh(hdr.body_len_);
    return hdr;
}

std::shared_ptr<message> registration_body::serialize(
    const std::string& username,
    const std::string& password) {
    uint32_t body_len = sizeof(chat262::registration_body) + username.length() +
                        password.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = version;
    msg->hdr_.type_ = registration;
    msg->hdr_.body_len_ = body_len;
    uint32_t user_len_le = htole32(static_cast<uint32_t>(username.length()));
    uint32_t pass_len_le = htole32(static_cast<uint32_t>(password.length()));
    memcpy(msg->body_, &user_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 4, &pass_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 8, username.c_str(), username.length());
    memcpy(msg->body_ + 8 + username.length(),
           password.c_str(),
           password.length());
    return msg;
}

void registration_body::deserialize(const std::vector<uint8_t>& body,
                                    std::string& username,
                                    std::string& password) {
    const uint8_t* msg_body = body.data();

    uint32_t user_len_le;
    memcpy(&user_len_le, msg_body, sizeof(uint32_t));
    uint32_t user_len = le32toh(user_len_le);

    uint32_t pass_len_le;
    memcpy(&pass_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t pass_len = le32toh(pass_len_le);

    username.assign(msg_body + 8, msg_body + 8 + user_len);
    password.assign(msg_body + 8 + user_len,
                    msg_body + 8 + user_len + pass_len);
}

std::shared_ptr<message> login_body::serialize(const std::string& username,
                                               const std::string& password) {
    uint32_t body_len = sizeof(chat262::registration_body) + username.length() +
                        password.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = version;
    msg->hdr_.type_ = login;
    msg->hdr_.body_len_ = body_len;
    uint32_t user_len_le = htole32(static_cast<uint32_t>(username.length()));
    uint32_t pass_len_le = htole32(static_cast<uint32_t>(password.length()));
    memcpy(msg->body_, &user_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 4, &pass_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 8, username.c_str(), username.length());
    memcpy(msg->body_ + 8 + username.length(),
           password.c_str(),
           password.length());
    return msg;
}

void login_body::deserialize(const std::vector<uint8_t>& body,
                             std::string& username,
                             std::string& password) {
    const uint8_t* msg_body = body.data();

    uint32_t user_len_le;
    memcpy(&user_len_le, msg_body, sizeof(uint32_t));
    uint32_t user_len = le32toh(user_len_le);

    uint32_t pass_len_le;
    memcpy(&pass_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t pass_len = le32toh(pass_len_le);

    username.assign(msg_body + 8, msg_body + 8 + user_len);
    password.assign(msg_body + 8 + user_len,
                    msg_body + 8 + user_len + pass_len);
}

}  // namespace chat262
