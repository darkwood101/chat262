#include "chat262_protocol.h"

#include "endianness.h"

#include <cstdlib>
#include <cstring>

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
        case msgtype_accounts_request:
            return "List accounts request";
        case msgtype_accounts_response:
            return "List accounts response";
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
        case status_code_user_exists:
            return "Username already exists";
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
    hdr.version_ = e_le16toh(hdr.version_);
    hdr.type_ = e_le16toh(hdr.type_);
    hdr.body_len_ = e_le32toh(hdr.body_len_);
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
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_registration_request);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t user_len_le = e_htole32(static_cast<uint32_t>(username.length()));
    uint32_t pass_len_le = e_htole32(static_cast<uint32_t>(password.length()));
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
    uint32_t user_len = e_le32toh(user_len_le);

    uint32_t pass_len_le;
    memcpy(&pass_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t pass_len = e_le32toh(pass_len_le);

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
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_registration_response);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t stat_code_le = e_htole32(stat_code);
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
    stat_code = e_le32toh(stat_code_le);
    return status::ok;
}

std::shared_ptr<message> login_request::serialize(const std::string& username,
                                                  const std::string& password) {
    uint32_t body_len =
        sizeof(chat262::login_request) + username.length() + password.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_login_request);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t user_len_le = e_htole32(static_cast<uint32_t>(username.length()));
    uint32_t pass_len_le = e_htole32(static_cast<uint32_t>(password.length()));
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
    uint32_t user_len = e_le32toh(user_len_le);

    uint32_t pass_len_le;
    memcpy(&pass_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t pass_len = e_le32toh(pass_len_le);

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
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_login_response);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t stat_code_le = e_htole32(stat_code);
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
    stat_code = e_le32toh(stat_code_le);
    return status::ok;
}

std::shared_ptr<message> logout_request::serialize() {
    std::shared_ptr<message> msg(
        static_cast<message*>(malloc(sizeof(message_header))),
        free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_logout_request);
    msg->hdr_.body_len_ = e_htole32(static_cast<uint32_t>(0));
    return msg;
}

status logout_request::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() != 0) {
        return status::error;
    }
    return status::ok;
}

std::shared_ptr<message> logout_response::serialize(uint32_t stat_code) {
    uint32_t body_len = sizeof(logout_response);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_logout_response);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t stat_code_le = e_htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));
    return msg;
}

status logout_response::deserialize(const std::vector<uint8_t>& data,
                                   uint32_t& stat_code) {
    // We know the size upfront
    if (data.size() != sizeof(logout_response)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t stat_code_le;
    memcpy(&stat_code_le, msg_body, sizeof(uint32_t));
    stat_code = e_le32toh(stat_code_le);
    return status::ok;
}

std::shared_ptr<message> accounts_request::serialize() {
    std::shared_ptr<message> msg(
        static_cast<message*>(malloc(sizeof(message_header))),
        free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_accounts_request);
    msg->hdr_.body_len_ = e_htole32(static_cast<uint32_t>(0));
    return msg;
}

status accounts_request::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() != 0) {
        return status::error;
    }
    return status::ok;
}

std::shared_ptr<message> accounts_response::serialize(
    uint32_t stat_code,
    const std::vector<std::string>& usernames) {
    uint32_t body_len = sizeof(uint32_t);
    // Add usernames to body length only if status code is ok
    if (stat_code == status_code_ok) {
        body_len += sizeof(uint32_t) + usernames.size() * sizeof(uint32_t);
        for (const std::string& u : usernames) {
            body_len += u.length();
        }
    }
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_accounts_response);
    msg->hdr_.body_len_ = e_htole32(body_len);

    // The status code is always serialized
    uint32_t stat_code_le = e_htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));

    if (stat_code == status_code_ok) {
        // Copy the number of acconts
        uint32_t num_accounts_le =
            e_htole32(static_cast<uint32_t>(usernames.size()));
        memcpy(msg->body_ + 4, &num_accounts_le, sizeof(uint32_t));

        // Points to the next username length to copy
        uint8_t* u_lengths_ptr = msg->body_ + 8;
        // Points to the next username to copy
        uint8_t* u_ptr = u_lengths_ptr + usernames.size() * sizeof(uint32_t);
        for (const std::string& u : usernames) {
            uint32_t u_length = static_cast<uint32_t>(u.length());
            uint32_t u_length_le = e_htole32(u_length);
            // Copy the username length
            memcpy(u_lengths_ptr, &u_length_le, sizeof(uint32_t));
            u_lengths_ptr += sizeof(uint32_t);
            // Copy the username
            memcpy(u_ptr, u.c_str(), u_length);
            u_ptr += u_length;
        }
    }
    return msg;
}

status accounts_response::deserialize(const std::vector<uint8_t>& data,
                                      uint32_t& stat_code,
                                      std::vector<std::string>& usernames) {
    // Make sure we can read the status code
    if (data.size() < sizeof(uint32_t)) {
        return status::error;
    }

    const uint8_t* msg_body = data.data();
    // Copy the status code
    uint32_t stat_code_le;
    memcpy(&stat_code_le, msg_body, sizeof(uint32_t));
    msg_body += sizeof(uint32_t);
    uint32_t stat_code_h = e_le32toh(stat_code_le);
    // If the status code is not OK, we're done
    if (stat_code_h != status_code_ok) {
        stat_code = stat_code_h;
        return status::ok;
    }

    // Make sure we can read the number of accounts
    if (data.size() < 2 * sizeof(uint32_t)) {
        return status::error;
    }

    // Copy the number of accounts
    uint32_t num_accounts_le;
    memcpy(&num_accounts_le, msg_body, sizeof(uint32_t));
    msg_body += sizeof(uint32_t);
    uint32_t num_accounts_h = e_le32toh(num_accounts_le);

    // Make sure we can read the username lengths
    if (data.size() <
        2 * sizeof(uint32_t) + num_accounts_h * sizeof(uint32_t)) {
        return status::error;
    }

    // Store all username lengths and compute total username length
    std::vector<uint32_t> u_lens;
    u_lens.resize(num_accounts_h);
    uint32_t total_u_len = 0;
    for (uint32_t i = 0; i != num_accounts_h;
         ++i, msg_body += sizeof(uint32_t)) {
        const uint8_t* u_len_ptr = msg_body;
        uint32_t u_len_le;
        memcpy(&u_len_le, u_len_ptr, sizeof(uint32_t));
        u_lens[i] = e_le32toh(u_len_le);
        total_u_len += u_lens[i];
    }

    // Make sure we can read all usernames
    if (data.size() < 2 * sizeof(uint32_t) + num_accounts_h * sizeof(uint32_t) +
                          total_u_len) {
        return status::error;
    }

    // Copy all usernames
    usernames.resize(num_accounts_h);
    for (uint32_t i = 0; i != num_accounts_h; ++i) {
        const uint8_t* u_ptr = msg_body;
        usernames[i].assign(u_ptr, u_ptr + u_lens[i]);
        msg_body += u_lens[i];
    }
    stat_code = stat_code_h;

    return status::ok;
}

}  // namespace chat262
