#ifndef _CHAT262_PROTOCOL_
#define _CHAT262_PROTOCOL_

#include "common.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace chat262 {

static constexpr uint16_t version = 0x1;
static constexpr uint16_t port = 45171;

// Message types. Types in the range [101, 199] are client requestes. Types in
// the range [201, 299] are server responses.
enum message_type : uint16_t {
    // Client requests
    msgtype_registration_request = 101,
    msgtype_login_request = 102,
    msgtype_logout_request = 103,
    msgtype_accounts_request = 104,

    // Server responses
    msgtype_registration_response = 201,
    msgtype_login_response = 202,
    msgtype_logout_response = 203,
    msgtype_accounts_response = 204
};

// Server response status codes
enum status_code : uint32_t {
    status_code_ok = 0x0,
    status_code_invalid_credentials = 0x1,
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

struct accounts_request {
    // Form a complete accounts request message. There is no body in this
    // request.
    static std::shared_ptr<message> serialize();

    // Do nothing because there is no body in the request. `data` must be an
    // empty vector.
    // @return ok    - success
    // @return error - `data.size() != 0`
    //                 This is the fault of the local implementation, never of
    //                 the remote party.
    static status deserialize(const std::vector<uint8_t>& data);
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

struct logout_body {};

static_assert(sizeof(message_header) == 8);
static_assert(sizeof(message) == 8);
static_assert(offsetof(message, body_) == 8);


};  // namespace chat262

#endif
