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

std::shared_ptr<message> registration_request::serialize(
    const std::string& username,
    const std::string& password) {
    uint32_t body_len = sizeof(chat262::registration_request) +
                        username.length() + password.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = htole16(version);
    msg->hdr_.type_ = htole16(msgtype_registration_request);
    msg->hdr_.body_len_ = htole32(body_len);
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

void registration_request::deserialize(const std::vector<uint8_t>& data,
                                       std::string& username,
                                       std::string& password) {
    const uint8_t* msg_body = data.data();

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

std::shared_ptr<message> registration_response::serialize(uint32_t status) {
    uint32_t body_len = sizeof(registration_response);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = htole16(version);
    msg->hdr_.type_ = htole16(msgtype_registration_response);
    msg->hdr_.body_len_ = htole32(body_len);
    uint32_t status_le = htole32(status);
    memcpy(msg->body_, &status_le, sizeof(uint32_t));
    return msg;
}

void registration_response::deserialize(const std::vector<uint8_t>& data,
                                        uint32_t& status) {
    const uint8_t* msg_body = data.data();

    uint32_t status_le;
    memcpy(&status_le, msg_body, sizeof(uint32_t));
    status = le32toh(status_le);
}

std::shared_ptr<message> login_request::serialize(const std::string& username,
                                                  const std::string& password) {
    uint32_t body_len =
        sizeof(chat262::login_request) + username.length() + password.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = htole16(version);
    msg->hdr_.type_ = htole16(msgtype_login_request);
    msg->hdr_.body_len_ = htole32(body_len);
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

void login_request::deserialize(const std::vector<uint8_t>& data,
                                std::string& username,
                                std::string& password) {
    const uint8_t* msg_body = data.data();

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

std::shared_ptr<message> login_response::serialize(uint32_t status) {
    uint32_t body_len = sizeof(login_response);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = htole16(version);
    msg->hdr_.type_ = htole16(msgtype_login_response);
    msg->hdr_.body_len_ = htole32(body_len);
    uint32_t status_le = htole32(status);
    memcpy(msg->body_, &status_le, sizeof(uint32_t));
    return msg;
}

void login_response::deserialize(const std::vector<uint8_t>& data,
                                 uint32_t& status) {
    const uint8_t* msg_body = data.data();

    uint32_t status_le;
    memcpy(&status_le, msg_body, sizeof(uint32_t));
    status = le32toh(status_le);
}

}  // namespace chat262
