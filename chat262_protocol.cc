#include "chat262_protocol.h"

#include <cstdlib>
#include <cstring>
#include <endian.h>

namespace chat262 {

const char* message_type_lookup(uint16_t msg_type) {
    switch (msg_type) {
        case msgtype_registration_request:
            return "Registration request";
        case msgtype_registration_response:
            return "Registration response";
        case msgtype_login_request:
            return "Login request";
        case msgtype_login_response:
            return "Login response";
        case msgtype_logout_request:
            return "Logout request";
        case msgtype_logout_response:
            return "Logout response";
        default:
            return "Unknown";
    }
}

const char* status_code_lookup(uint32_t stat_code) {
    switch (stat_code) {
        case status_code_ok:
            return "OK";
        case status_code_invalid_credentials:
            return "Invalid username and/or password";
        default:
            return "Unknown";
    }
}

status message_header::deserialize(const std::vector<uint8_t>& data,
                                   message_header& hdr) {
    if (data.size() != sizeof(message_header)) {
        return status::error;
    }
    memcpy(&hdr, data.data(), sizeof(message_header));
    hdr.version_ = le16toh(hdr.version_);
    hdr.type_ = le16toh(hdr.type_);
    hdr.body_len_ = le32toh(hdr.body_len_);
    return status::ok;
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

status registration_request::deserialize(const std::vector<uint8_t>& data,
                                         std::string& username,
                                         std::string& password) {
    // Ensure we can at least read username and password length
    if (data.size() < sizeof(registration_request)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t user_len_le;
    memcpy(&user_len_le, msg_body, sizeof(uint32_t));
    uint32_t user_len = le32toh(user_len_le);

    uint32_t pass_len_le;
    memcpy(&pass_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t pass_len = le32toh(pass_len_le);

    // Cannot proceed if there is a mismatch of size
    if (2 * sizeof(uint32_t) + user_len + pass_len != data.size()) {
        return status::error;
    }

    username.assign(msg_body + 8, msg_body + 8 + user_len);
    password.assign(msg_body + 8 + user_len,
                    msg_body + 8 + user_len + pass_len);
    return status::ok;
}

std::shared_ptr<message> registration_response::serialize(uint32_t stat_code) {
    uint32_t body_len = sizeof(registration_response);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = htole16(version);
    msg->hdr_.type_ = htole16(msgtype_registration_response);
    msg->hdr_.body_len_ = htole32(body_len);
    uint32_t stat_code_le = htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));
    return msg;
}

status registration_response::deserialize(const std::vector<uint8_t>& data,
                                          uint32_t& stat_code) {
    // We know the size upfront
    if (data.size() != sizeof(registration_response)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t stat_code_le;
    memcpy(&stat_code_le, msg_body, sizeof(uint32_t));
    uint32_t stat_code_he = le32toh(stat_code_le);
    stat_code = le32toh(stat_code_le);
    return status::ok;
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

status login_request::deserialize(const std::vector<uint8_t>& data,
                                  std::string& username,
                                  std::string& password) {
    // Ensure we can at least read username and password length
    if (data.size() < sizeof(login_request)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t user_len_le;
    memcpy(&user_len_le, msg_body, sizeof(uint32_t));
    uint32_t user_len = le32toh(user_len_le);

    uint32_t pass_len_le;
    memcpy(&pass_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t pass_len = le32toh(pass_len_le);

    // Cannot proceed if there is a mismatch of size
    if (2 * sizeof(uint32_t) + user_len + pass_len != data.size()) {
        return status::error;
    }

    username.assign(msg_body + 8, msg_body + 8 + user_len);
    password.assign(msg_body + 8 + user_len,
                    msg_body + 8 + user_len + pass_len);
    return status::ok;
}

std::shared_ptr<message> login_response::serialize(uint32_t stat_code) {
    uint32_t body_len = sizeof(login_response);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = htole16(version);
    msg->hdr_.type_ = htole16(msgtype_login_response);
    msg->hdr_.body_len_ = htole32(body_len);
    uint32_t stat_code_le = htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));
    return msg;
}

status login_response::deserialize(const std::vector<uint8_t>& data,
                                   uint32_t& stat_code) {
    // We know the size upfront
    if (data.size() != sizeof(login_response)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t stat_code_le;
    memcpy(&stat_code_le, msg_body, sizeof(uint32_t));
    uint32_t stat_code_he = le32toh(stat_code_le);
    stat_code = le32toh(stat_code_le);
    return status::ok;
}

}  // namespace chat262
