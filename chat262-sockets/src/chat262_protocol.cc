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
        case msgtype_send_txt_request:
            return "Send text request";
        case msgtype_send_txt_response:
            return "Send text response";
        case msgtype_recv_txt_request:
            return "Receive text request";
        case msgtype_recv_txt_response:
            return "Receive text response";
        case msgtype_correspondents_request:
            return "Correspondents request";
        case msgtype_correspondents_response:
            return "Correspondents response";
        case msgtype_delete_request:
            return "Delete account request";
        case msgtype_delete_response:
            return "Delete account response";
        case msgtype_wrong_version_response:
            return "Wrong version response";
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
        case status_code_user_noexist:
            return "User does not exist";
        case status_code_username_invalid:
            return "Username is invalid";
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

std::shared_ptr<message> accounts_request::serialize(
    const std::string& pattern) {
    uint32_t body_len = sizeof(uint32_t) + pattern.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_accounts_request);
    msg->hdr_.body_len_ = e_htole32(static_cast<uint32_t>(body_len));

    uint32_t pattern_len_le =
        e_htole32(static_cast<uint32_t>(pattern.length()));
    memcpy(msg->body_, &pattern_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 4, pattern.c_str(), pattern.length());

    return msg;
}

status accounts_request::deserialize(const std::vector<uint8_t>& data,
                                     std::string& pattern) {
    if (data.size() < sizeof(uint32_t)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t pattern_len_le;
    memcpy(&pattern_len_le, msg_body, sizeof(uint32_t));
    uint32_t pattern_len = e_le32toh(pattern_len_le);

    if (sizeof(uint32_t) + pattern_len != data.size()) {
        return status::error;
    }

    pattern.assign(msg_body + 4, msg_body + 4 + pattern_len);
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

std::shared_ptr<message> send_txt_request::serialize(
    const std::string& recipient,
    const std::string& txt) {
    uint32_t body_len =
        sizeof(chat262::send_txt_request) + recipient.length() + txt.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_send_txt_request);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t recipient_len_le =
        e_htole32(static_cast<uint32_t>(recipient.length()));
    uint32_t txt_len_le = e_htole32(static_cast<uint32_t>(txt.length()));
    memcpy(msg->body_, &recipient_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 4, &txt_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 8, recipient.c_str(), recipient.length());
    memcpy(msg->body_ + 8 + recipient.length(), txt.c_str(), txt.length());
    return msg;
}

status send_txt_request::deserialize(const std::vector<uint8_t>& data,
                                     std::string& recipient,
                                     std::string& txt) {
    if (data.size() < sizeof(send_txt_request)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t recipient_len_le;
    memcpy(&recipient_len_le, msg_body, sizeof(uint32_t));
    uint32_t recipient_len = e_le32toh(recipient_len_le);

    uint32_t txt_len_le;
    memcpy(&txt_len_le, msg_body + 4, sizeof(uint32_t));
    uint32_t txt_len = e_le32toh(txt_len_le);

    // Cannot proceed if there is a mismatch of size
    if (2 * sizeof(uint32_t) + recipient_len + txt_len != data.size()) {
        return status::error;
    }

    recipient.assign(msg_body + 8, msg_body + 8 + recipient_len);
    txt.assign(msg_body + 8 + recipient_len,
               msg_body + 8 + recipient_len + txt_len);
    return status::ok;
}

std::shared_ptr<message> send_txt_response::serialize(uint32_t stat_code) {
    uint32_t body_len = sizeof(send_txt_response);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_send_txt_response);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t stat_code_le = e_htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));
    return msg;
}

status send_txt_response::deserialize(const std::vector<uint8_t>& data,
                                      uint32_t& stat_code) {
    // We know the size upfront
    if (data.size() != sizeof(send_txt_response)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t stat_code_le;
    memcpy(&stat_code_le, msg_body, sizeof(uint32_t));
    stat_code = e_le32toh(stat_code_le);
    return status::ok;
}

std::shared_ptr<message> recv_txt_request::serialize(
    const std::string& sender) {
    uint32_t body_len = sizeof(chat262::recv_txt_request) + sender.length();
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_recv_txt_request);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t sender_len_le = e_htole32(static_cast<uint32_t>(sender.length()));
    memcpy(msg->body_, &sender_len_le, sizeof(uint32_t));
    memcpy(msg->body_ + 4, sender.c_str(), sender.length());
    return msg;
}

status recv_txt_request::deserialize(const std::vector<uint8_t>& data,
                                     std::string& sender) {
    if (data.size() < sizeof(recv_txt_request)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t sender_len_le;
    memcpy(&sender_len_le, msg_body, sizeof(uint32_t));
    uint32_t sender_len = e_le32toh(sender_len_le);

    // Cannot proceed if there is a mismatch of size
    if (sizeof(uint32_t) + sender_len != data.size()) {
        return status::error;
    }

    sender.assign(msg_body + 4, msg_body + 4 + sender_len);
    return status::ok;
}

std::shared_ptr<message> recv_txt_response::serialize(uint32_t stat_code,
                                                      const chat& c) {
    uint32_t body_len = sizeof(uint32_t);
    if (stat_code == status_code_ok) {
        body_len += sizeof(uint32_t) +
                    c.texts_.size() * (sizeof(uint8_t) + sizeof(uint32_t));
        for (const text& txt : c.texts_) {
            body_len += txt.content_.length();
        }
    }
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_recv_txt_response);
    msg->hdr_.body_len_ = e_htole32(body_len);

    // The status code is always serialized
    uint32_t stat_code_le = e_htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));

    // The rest is serialized only if status code is OK
    if (stat_code == status_code_ok) {
        // Copy the number of texts
        uint32_t num_accounts_le =
            e_htole32(static_cast<uint32_t>(c.texts_.size()));
        memcpy(msg->body_ + 4, &num_accounts_le, sizeof(uint32_t));

        // Points to the next sender identifier to copy
        uint8_t* sender_ptr = msg->body_ + 8;
        // Points to the next text length to copy
        uint8_t* txt_lens_ptr = sender_ptr + c.texts_.size() * sizeof(uint8_t);
        // Points to the next text to copy
        uint8_t* txt_ptr = txt_lens_ptr + c.texts_.size() * sizeof(uint32_t);
        for (const text& txt : c.texts_) {
            memcpy(sender_ptr, &(txt.sender_), sizeof(uint8_t));
            sender_ptr += sizeof(uint8_t);

            uint32_t txt_len = static_cast<uint32_t>(txt.content_.length());
            uint32_t txt_len_le = e_htole32(txt_len);

            memcpy(txt_lens_ptr, &txt_len_le, sizeof(uint32_t));
            txt_lens_ptr += sizeof(uint32_t);

            memcpy(txt_ptr, txt.content_.c_str(), txt_len);
            txt_ptr += txt_len;
        }
    }
    return msg;
}

status recv_txt_response::deserialize(const std::vector<uint8_t>& data,
                                      uint32_t& stat_code,
                                      chat& c) {
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

    // Make sure we can read the number of texts
    if (data.size() < 2 * sizeof(uint32_t)) {
        return status::error;
    }

    // Copy the number of texts
    uint32_t num_txts_le;
    memcpy(&num_txts_le, msg_body, sizeof(uint32_t));
    msg_body += sizeof(uint32_t);
    uint32_t num_txts_h = e_le32toh(num_txts_le);

    // Make sure we can read senders and text lengths
    if (data.size() < 2 * sizeof(uint32_t) +
                          num_txts_h * (sizeof(uint8_t) + sizeof(uint32_t))) {
        return status::error;
    }

    // Store all senders and text lengths, and compute total text length
    std::vector<uint8_t> senders;
    std::vector<uint32_t> txt_lens;
    senders.resize(num_txts_h);
    txt_lens.resize(num_txts_h);
    uint32_t total_txt_len = 0;
    const uint8_t* sender_ptr = msg_body;
    const uint8_t* txt_lens_ptr = msg_body + num_txts_h * sizeof(uint8_t);
    for (uint32_t i = 0; i != num_txts_h;
         ++i, sender_ptr += sizeof(uint8_t), txt_lens_ptr += sizeof(uint32_t)) {
        memcpy(&(senders[i]), sender_ptr, sizeof(uint8_t));
        uint32_t txt_len_le;
        memcpy(&txt_len_le, txt_lens_ptr, sizeof(uint32_t));
        txt_lens[i] = e_le32toh(txt_len_le);
        total_txt_len += txt_lens[i];
    }
    msg_body += num_txts_h * (sizeof(uint8_t) + sizeof(uint32_t));

    // Make sure we can read all texts
    if (data.size() < 2 * sizeof(uint32_t) +
                          num_txts_h * (sizeof(uint8_t) + sizeof(uint32_t)) +
                          total_txt_len) {
        return status::error;
    }

    // Copy all senders and texts
    c.texts_.resize(num_txts_h);
    for (uint32_t i = 0; i != num_txts_h; ++i) {
        c.texts_[i].sender_ = senders[i];
        c.texts_[i].content_.assign(msg_body, msg_body + txt_lens[i]);
        msg_body += txt_lens[i];
    }
    stat_code = stat_code_h;

    return status::ok;
}

std::shared_ptr<message> correspondents_request::serialize() {
    std::shared_ptr<message> msg(
        static_cast<message*>(malloc(sizeof(message_header))),
        free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_correspondents_request);
    msg->hdr_.body_len_ = e_htole32(static_cast<uint32_t>(0));
    return msg;
}

status correspondents_request::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() != 0) {
        return status::error;
    }
    return status::ok;
}

std::shared_ptr<message> correspondents_response::serialize(
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
    msg->hdr_.type_ = e_htole16(msgtype_correspondents_response);
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

status correspondents_response::deserialize(
    const std::vector<uint8_t>& data,
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

std::shared_ptr<message> delete_request::serialize() {
    std::shared_ptr<message> msg(
        static_cast<message*>(malloc(sizeof(message_header))),
        free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_delete_request);
    msg->hdr_.body_len_ = e_htole32(static_cast<uint32_t>(0));
    return msg;
}

status delete_request::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() != 0) {
        return status::error;
    }
    return status::ok;
}

std::shared_ptr<message> delete_response::serialize(uint32_t stat_code) {
    uint32_t body_len = sizeof(uint32_t);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_delete_response);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t stat_code_le = e_htole32(stat_code);
    memcpy(msg->body_, &stat_code_le, sizeof(uint32_t));
    return msg;
}

status delete_response::deserialize(const std::vector<uint8_t>& data,
                                    uint32_t& stat_code) {
    if (data.size() != sizeof(delete_response)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t stat_code_le;
    memcpy(&stat_code_le, msg_body, sizeof(uint32_t));
    stat_code = e_le32toh(stat_code_le);
    return status::ok;
}

std::shared_ptr<message> wrong_version_response::serialize(
    uint32_t correct_version) {
    uint32_t body_len = sizeof(uint32_t);
    size_t total_len = sizeof(message_header) + body_len;
    std::shared_ptr<message> msg(static_cast<message*>(malloc(total_len)),
                                 free);
    msg->hdr_.version_ = e_htole16(version);
    msg->hdr_.type_ = e_htole16(msgtype_wrong_version_response);
    msg->hdr_.body_len_ = e_htole32(body_len);
    uint32_t version_le = e_htole32(correct_version);
    memcpy(msg->body_, &version_le, sizeof(uint32_t));
    return msg;
}

status wrong_version_response::deserialize(const std::vector<uint8_t>& data,
                                           uint32_t& correct_version) {
    if (data.size() != sizeof(uint32_t)) {
        return status::error;
    }
    const uint8_t* msg_body = data.data();

    uint32_t version_le;
    memcpy(&version_le, msg_body, sizeof(uint32_t));
    correct_version = e_le32toh(version_le);
    return status::ok;
}

}  // namespace chat262
