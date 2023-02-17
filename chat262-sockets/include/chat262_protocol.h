#ifndef _CHAT262_PROTOCOL_
#define _CHAT262_PROTOCOL_

#include "chat.h"
#include "common.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace chat262 {

static constexpr uint16_t version = 1;
static constexpr uint16_t port = 61079;

// Message types. Types in the range [101, 199] are client requestes. Types in
// the range [201, 299] are server responses.
enum message_type : uint16_t {
    // Client requests
    msgtype_registration_request = 101,
    msgtype_login_request = 102,
    msgtype_logout_request = 103,
    msgtype_accounts_request = 104,
    msgtype_send_txt_request = 105,
    msgtype_recv_txt_request = 106,
    msgtype_correspondents_request = 107,
    msgtype_delete_request = 108,

    // Server responses
    msgtype_registration_response = 201,
    msgtype_login_response = 202,
    msgtype_logout_response = 203,
    msgtype_accounts_response = 204,
    msgtype_send_txt_response = 205,
    msgtype_recv_txt_response = 206,
    msgtype_correspondents_response = 207,
    msgtype_delete_response = 208,

    // Error server responses
    msgtype_wrong_version_response = 301,
    msgtype_invalid_type_response = 302
};

// Server response status codes
enum status_code : uint32_t {
    status_code_ok = 0,
    status_code_invalid_credentials = 1,
    status_code_user_exists = 2,
    status_code_user_noexist = 3,
    status_code_username_invalid = 4,
    status_code_password_invalid = 5,
    status_code_unauthorized = 6
};

// Look up the message type and returns a descriptive string
const char* message_type_lookup(uint16_t msg_type);

// Look up the server status code and returns a descriptive string
const char* status_code_lookup(uint32_t stat_code);

struct message_header {
    uint16_t version_;
    uint16_t type_;
    uint32_t body_len_;

    // Extract the header from `data` into `hdr`.
    // @return ok    - success
    // @return error - `data.size() != sizeof(message_header)`
    //                 This is the fault of the local implementation, never of
    //                 the remote party.
    static status deserialize(const std::vector<uint8_t>& data,
                              message_header& hdr);
};

struct message {
    message_header hdr_;
    uint8_t body_[];
};

struct registration_request {
    uint32_t user_len_;
    uint32_t pass_len_;
    uint8_t user_pass_[];

    // Form a complete registration request message from `username` and
    // `password`.
    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);

    // Extract the username and password from `data`.
    // `data` must contain the `registration_request` structure.
    // @return ok    - success
    // @return error - `data.size() != sizeof(registration_request) + user_len_
    //                 + pass_len_`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` did not match
    //                 what was provided in `user_len_` and `pass_len_`.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& username,
                              std::string& password);
};

struct registration_response {
    uint32_t stat_code;

    // Form a complete registration response message from `stat_code`.
    static std::shared_ptr<message> serialize(uint32_t stat_code);

    // Extract the status code from `data`.
    // `data` must contain the `registration_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size() != sizeof(registration_response)`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct login_request {
    uint32_t user_len_;
    uint32_t pass_len_;
    uint8_t user_pass_[];

    // Form a complete login request message from `username` and `password`.
    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);

    // Extract the username and password from `data`.
    // `data` must contain the `login_request` structure.
    // @return ok    - success
    // @return error - `data.size() != sizeof(login_request) + user_len_
    //                 + pass_len_`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` did not match
    //                 what was provided in `user_len_` and `pass_len_`.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& username,
                              std::string& password);
};

struct login_response {
    uint32_t stat_code;

    // Form a complete login response message from `stat_code`.
    static std::shared_ptr<message> serialize(uint32_t stat_code);

    // Extract the status code from `data`.
    // `data` must contain the `login_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size() != sizeof(login_response)`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct logout_request {
    // Form a complete logout request message. There is no body in this request.
    static std::shared_ptr<message> serialize();

    // Do nothing because there is no body in the request. `data` must be an
    // empty vector.
    // @return ok    - success
    // @return error - `data.size() != 0`
    //                 This is the fault of the local implementation, never of
    //                 the remote party.
    static status deserialize(const std::vector<uint8_t>& data);
};

struct logout_response {
    uint32_t stat_code;

    // Form a complete logout response message from `stat_code`.
    static std::shared_ptr<message> serialize(uint32_t stat_code);

    // Extract the status code from `data`.
    // `data` must contain the `logout_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size() != sizeof(logout_response)`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct accounts_request {
    uint32_t pattern_len_;
    uint8_t pattern_[];

    // Form a complete accounts request message from `pattern`.
    static std::shared_ptr<message> serialize(const std::string& pattern);

    // Extract the matching pattern from `data`. `data` must contain the
    // `accounts_request` structure.
    // @return ok    - success
    // @return error - `data.size() != 0`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& pattern);
};

struct accounts_response {
    uint32_t stat_code_;
    // uint32_t num_accounts_;
    // uint32_t username_lens_[];
    // uint8_t usernames_[];

    // Form a complete accounts response from `stat_code` and `usernames`.
    // If `stat_code` is `status_code_ok`, then the message is properly formed.
    // If `stat_code` is anything else, then `usernames` is ignored and no
    // actual usernames are serialized into the message; the message contains
    // only the status code.
    static std::shared_ptr<message> serialize(
        uint32_t stat_code,
        const std::vector<std::string>& usernames);

    // Extract the status code and the usernames from `data`.
    // If `stat_code` is `status_code_ok`, then the data is properly extracted.
    // If `stat_code` is anything else, then `usernames` is ignored.
    // The caller must first check `stat_code` before accessing `usernames`.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` does not reflect the contents of
    //                 `accounts_response`
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code,
                              std::vector<std::string>& usernames);
};

struct send_txt_request {
    uint32_t user_len_;
    uint32_t msg_len_;
    uint8_t user_txt_[];

    static std::shared_ptr<message> serialize(const std::string& recipient,
                                              const std::string& txt);
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& recipient,
                              std::string& txt);
};

struct send_txt_response {
    uint32_t stat_code_;

    static std::shared_ptr<message> serialize(uint32_t stat_code);
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct recv_txt_request {
    uint32_t user_len_;
    uint8_t user_[];

    static std::shared_ptr<message> serialize(const std::string& sender);
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& sender);
};

struct recv_txt_response {
    uint32_t stat_code_;
    // uint32_t num_txts_;
    // uin8_t senders_[num_txts_];
    // uint32_t txt_lens_[num_txts_];
    // uin8_t txts_[num_txts_];

    static std::shared_ptr<message> serialize(uint32_t stat_code,
                                              const chat& c);
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code,
                              chat& c);
};

struct correspondents_request {
    static std::shared_ptr<message> serialize();
    static status deserialize(const std::vector<uint8_t>& data);
};

struct correspondents_response {
    uint32_t stat_code;
    // uint32_t num_accounts_;
    // uint32_t username_lens_[];
    // uint8_t usernames_[];

    static std::shared_ptr<message> serialize(
        uint32_t stat_code,
        const std::vector<std::string>& usernames);
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code,
                              std::vector<std::string>& usernames);
};

struct delete_request {
    static std::shared_ptr<message> serialize();
    static status deserialize(const std::vector<uint8_t>& data);
};

struct delete_response {
    uint32_t stat_code;

    static std::shared_ptr<message> serialize(uint32_t stat_code);
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct wrong_version_response {
    uint32_t correct_version_;

    static std::shared_ptr<message> serialize(uint32_t correct_version);
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& correct_version);
};

struct invalid_type_response {
    static std::shared_ptr<message> serialize();
    static status deserialize(const std::vector<uint8_t>& data);
};

struct logout_body {};

static_assert(sizeof(message_header) == 8);
static_assert(sizeof(message) == 8);
static_assert(offsetof(message, body_) == 8);


};  // namespace chat262

#endif
