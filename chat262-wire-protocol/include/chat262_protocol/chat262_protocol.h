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

// Message types.
// Types in the range [101, 199] are client requests.
// Types in the range [201, 299] are server responses.
// Types in the range [301, 399] are special server responses.
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

    // Special server responses
    msgtype_wrong_version_response = 301,
    msgtype_invalid_type_response = 302,
    msgtype_invalid_body_response = 303
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
const char* message_type_lookup(const uint16_t msg_type);

// Look up the server status code and returns a descriptive string
const char* status_code_lookup(const uint32_t stat_code);

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
    // Layout from the specification:
    //
    // uint32_t username_length;
    // uint32_t password_length;
    // uint8_t username[username_length];
    // uint8_t password[password_length]

    // Form a complete registration request message from `username` and
    // `password`.
    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);

    // Extract the username and password from `data` into `username` and
    // `password`. `data` must contain the `registration_request` structure.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& username,
                              std::string& password);
};

struct registration_response {
    // Layout from the specification:
    //
    // uint32_t status_code;

    // Form a complete registration response message from `stat_code`.
    static std::shared_ptr<message> serialize(const uint32_t stat_code);

    // Extract the status code from `data` into `stat_code`.
    // `data` must contain the `registration_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct login_request {
    // Layout from the specification:
    //
    // uint32_t username_length;
    // uint32_t password_length;
    // uint8_t username[username_length];
    // uint8_t password[password_length]

    // Form a complete login request message from `username` and `password`.
    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);

    // Extract the username and password from `data` into `username` and
    // `password`. `data` must contain the `login_request` structure.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& username,
                              std::string& password);
};

struct login_response {
    // Layout from the specification:
    //
    // uint32_t status_code;

    // Form a complete login response message from `stat_code`.
    static std::shared_ptr<message> serialize(const uint32_t stat_code);

    // Extract the status code from `data` into `stat_code`.
    // `data` must contain the `login_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
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
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data);
};

struct logout_response {
    // Layout from the specification:
    //
    // uint32_t status_code;

    // Form a complete logout response message from `stat_code`.
    static std::shared_ptr<message> serialize(const uint32_t stat_code);

    // Extract the status code from `data` into `stat_code`.
    // `data` must contain the `logout_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct accounts_request {
    // Layout from the specification:
    //
    // uint32_t pattern_length;
    // uint8_t pattern[pattern_length];

    // Form a complete accounts request message from `pattern`.
    static std::shared_ptr<message> serialize(const std::string& pattern);

    // Extract the matching pattern from `data` into `pattern`. `data` must
    // contain the `accounts_request` structure.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& pattern);
};

struct accounts_response {
    // Layout from the specification:
    //
    // uint32_t status_code;
    //
    // // present only if `status_code` is OK
    // uint32_t num_accounts;
    //
    // // present only if `status_code` is OK
    // uint32_t username_lengths[num_accounts];
    //
    // // present only if `status_code` is OK
    // uint8_t usernames[num_accounts];

    // Form a complete accounts response from `stat_code` and `usernames`.
    // If `stat_code` is `status_code_ok`, then the message is properly formed.
    // If `stat_code` is anything else, then `usernames` is ignored and no
    // actual usernames are serialized into the message; the message contains
    // only the status code.
    static std::shared_ptr<message> serialize(
        const uint32_t stat_code,
        const std::vector<std::string>& usernames);

    // Extract the status code and the usernames from `data` into `stat_code`
    // and `usernames`. `data` must contain the `accounts_response` structure.
    // If `stat_code` is `status_code_ok`, then the data is properly extracted.
    // If `stat_code` is anything else, then `usernames` is ignored.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
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
    // Layout from the specification:
    //
    // uint32_t username_length;
    // uint32_t text_length;
    // uint8_t username[username_length];
    // uint8_t text[text_length];

    // Form a complete send text request message from `recipient` and `txt`.
    static std::shared_ptr<message> serialize(const std::string& recipient,
                                              const std::string& txt);

    // Extract the recipient and the text from `data` into `recipient` and
    // `txt`. `data` must contain the `send_txt_request` structure.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& recipient,
                              std::string& txt);
};

struct send_txt_response {
    // Layout from the specification:
    //
    // uint32_t status_code;

    // Form a complete send text response message from `stat_code`.
    static std::shared_ptr<message> serialize(const uint32_t stat_code);

    // Extract the status code from `data` into `stat_code`.
    // `data` must contain the `send_txt_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct recv_txt_request {
    // Layout from the specification:
    //
    // uint32_t username_length;
    // uint32_t username[username_length];

    // Form a complete receive text request from `username`.
    static std::shared_ptr<message> serialize(const std::string& username);

    // Extract the sender and the text from `data` into `sender`. `data` must
    // contain the `recv_txt_request` structure.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& sender);
};

struct recv_txt_response {
    // Layout from the specification
    //
    // uint32_t status_code;
    //
    // // present only if `status_code` is OK
    // uint32_t num_txts;
    //
    // // present only if `status_code` is OK
    // uin8_t senders_indicators[num_txts];
    //
    // // present only if `status_code` is OK
    // uint32_t txt_lengths[num_txts];
    //
    // // present only if `status_code` is OK
    // uint8_t txt_0[txt_lengths[0]];
    // uint8_t txt_1[txt_lengths[1]];
    // // ...
    // uint8_t txt_last[txt_lengths[num_txts - 1]];

    // Form a complete receive text response from `stat_code` and `c`.
    static std::shared_ptr<message> serialize(const uint32_t stat_code,
                                              const chat& c);

    // Extract the status code and the received texts from `data` into
    // `stat_code` and `c`. `data` must contain the `recv_txt_response`
    // structure. If `stat_code` is `status_code_ok`, then the data is properly
    // extracted. If `stat_code` is anything else, then `c` is ignored.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code,
                              chat& c);
};

struct correspondents_request {
    // Form a complete correspondents request message. There is no body in this
    // request.
    static std::shared_ptr<message> serialize();

    // Do nothing because there is no body in the request. `data` must be an
    // empty vector.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data);
};

struct correspondents_response {
    // Layout from the specification
    //
    // uint32_t status_code;
    //
    // // present only if `status_code` is OK
    // uint32_t num_accounts;
    //
    // // present only if `status_code` is OK
    // uin8_t username_lengths[num_accounts];
    //
    // // present only if `status_code` is OK
    // uint8_t username_0[username_lengths[0]];
    // uint8_t username_1[username_lengths[1]];
    // // ...
    // uint8_t username_last[username_lengths[num_accounts - 1]];

    // Form a complete correspondents response message from `stat_code` and
    // `usernames`.
    static std::shared_ptr<message> serialize(
        const uint32_t stat_code,
        const std::vector<std::string>& usernames);

    // Extract the status code and the correspondents from `data` into
    // `stat_code` and `usernames`. `data` must contain the
    // `correspondents_response` structure.
    // If `stat_code` is `status_code_ok`, then the data is properly extracted.
    // If `stat_code` is anything else, then `usernames` is ignored.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code,
                              std::vector<std::string>& usernames);
};

struct delete_request {
    // Form a complete delete request message. There is no body in this request.
    static std::shared_ptr<message> serialize();

    // Do nothing because there is no body in the request. `data` must be an
    // empty vector.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data);
};

struct delete_response {
    // Layout from the specification:
    //
    // uint32_t status_code;

    // Form a complete delete response message from `stat_code`.
    static std::shared_ptr<message> serialize(const uint32_t stat_code);

    // Extract the status code from `data` into `stat_code`.
    // `data` must contain the `delete_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint32_t& stat_code);
};

struct wrong_version_response {
    // Layout from the specification:
    //
    // uint16_t correct_version;

    // Form a complete wrong version response message from `correct_version`.
    static std::shared_ptr<message> serialize(const uint16_t correct_version);

    // Extract the status code from `data` into `correct_version`.
    // `data` must contain the `wrong_version_response` structure.
    // @return ok    - success. There is no guarantee that `stat_code` is a
    //                 valid member of the `status_code` enum, and local
    //                 implementation should do further error-checking.
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data,
                              uint16_t& correct_version);
};

struct invalid_type_response {
    // Form a complete invalid type response message. There is no body in this
    // request.
    static std::shared_ptr<message> serialize();

    // Do nothing because there is no body in the request. `data` must be an
    // empty vector.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data);
};

struct invalid_body_response {
    // Form a complete invalid body response message. There is no body in this
    // request.
    static std::shared_ptr<message> serialize();

    // Do nothing because there is no body in the request. `data` must be an
    // empty vector.
    // @return ok    - success
    // @return error - `data.size()` is of incorrect size.
    //                 This is potentially the fault of the local
    //                 implementation, if `data` was not resized to `body_len_`
    //                 advertised in the message header. It could also be the
    //                 fault of the remote party, if `body_len_` was incorrectly
    //                 advertised in the message header.
    static status deserialize(const std::vector<uint8_t>& data);
};

// Make sure the layout of `message` is as we expect it
static_assert(sizeof(message_header) == 8);
static_assert(sizeof(message) == 8);
static_assert(offsetof(message, body_) == 8);

};  // namespace chat262

#endif
