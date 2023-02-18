#include "chat262_protocol.h"
#include "client.h"
#include "server.h"

#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

// We will abuse the implementation detail that returned usernames are ordered
// alphabetically. This is NOT guaranteed by the Chat 262 protocol.
const char* test_usernames[] = {"1234",
                                "3_BKn.4C",
                                "Ea8jjQa2hzom",
                                "JoP4fqkvVpBQ",
                                "otheruser",
                                "testuser"};

void client::test_search_accounts() {
    const char* localhost = "127.0.0.1";
    char const* argv[] = {"./server", localhost};
    std::thread thread([&]() {
        server s;
        s.run(2, argv);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    thread.detach();

    inet_pton(AF_INET, localhost, &n_ip_addr_);
    str_ip_addr_ = argv[1];
    assert(connect_server() == status::ok);

    std::shared_ptr<chat262::message> msg;
    chat262::message_header hdr;
    std::vector<uint8_t> data;
    uint32_t stat_code;
    std::vector<std::string> usernames;

    auto send_reg_request = [&](const char* username, const char* password) {
        msg = chat262::registration_request::serialize(username, password);
        assert(msg->hdr_.version_ == 1);
        assert(msg->hdr_.type_ == 101);
        assert(msg->hdr_.body_len_ == 8 + strlen(username) + strlen(password));
        send_msg(msg);
        recv_hdr(hdr);
        assert(hdr.version_ == 1);
        assert(hdr.type_ == 201);
        assert(hdr.body_len_ == 4);
        recv_body(hdr.body_len_, data);
        assert(chat262::registration_response::deserialize(data, stat_code) ==
               status::ok);
        return stat_code;
    };

    assert(send_reg_request("testuser", "password") == 0);
    assert(send_reg_request("otheruser", "password") == 0);
    assert(send_reg_request("JoP4fqkvVpBQ", "                ") == 0);
    assert(send_reg_request("Ea8jjQa2hzom", "*******") == 0);
    assert(send_reg_request("1234", "*raR*rF8KbRGhTa") == 0);
    assert(send_reg_request("3_BKn.4C", "pswd") == 0);

    auto send_login_request = [&](const char* username, const char* password) {
        msg = chat262::login_request::serialize(username, password);
        assert(msg->hdr_.version_ == 1);
        assert(msg->hdr_.type_ == 102);
        assert(msg->hdr_.body_len_ == 8 + strlen(username) + strlen(password));
        send_msg(msg);
        recv_hdr(hdr);
        assert(hdr.version_ == 1);
        assert(hdr.type_ == 202);
        assert(hdr.body_len_ == 4);
        recv_body(hdr.body_len_, data);
        assert(chat262::login_response::deserialize(data, stat_code) ==
               status::ok);
        return stat_code;
    };

    auto send_logout_request = [&]() {
        msg = chat262::logout_request::serialize();
        assert(msg->hdr_.version_ == 1);
        assert(msg->hdr_.type_ == 103);
        assert(msg->hdr_.body_len_ == 0);
        send_msg(msg);
        recv_hdr(hdr);
        assert(hdr.version_ == 1);
        assert(hdr.type_ == 203);
        assert(hdr.body_len_ == 4);
        recv_body(hdr.body_len_, data);
        assert(chat262::logout_response::deserialize(data, stat_code) ==
               status::ok);
        return stat_code;
    };

    auto send_accounts_request = [&](const char* pattern,
                                     size_t expected_match_size) {
        msg = chat262::accounts_request::serialize(pattern);
        assert(msg->hdr_.version_ == 1);
        assert(msg->hdr_.type_ == 104);
        assert(msg->hdr_.body_len_ == 4 + strlen(pattern));
        send_msg(msg);
        recv_hdr(hdr);
        assert(hdr.version_ == 1);
        assert(hdr.type_ == 204);
        assert(hdr.body_len_ == expected_match_size);
        recv_body(hdr.body_len_, data);
        assert(chat262::accounts_response::deserialize(data,
                                                       stat_code,
                                                       usernames) ==
               status::ok);
        return stat_code;
    };

    // Should return all usernames
    assert(send_login_request("testuser", "password") == 0);
    assert(send_accounts_request("*", 85) == 0);
    assert(usernames.size() == 6);
    for (int i = 0; i != 6; ++i) {
        assert(strcmp(usernames[i].c_str(), test_usernames[i]) == 0);
    }
    assert(send_logout_request() == 0);

    // Unauthorized if not logged in
    assert(send_accounts_request("*", 4) == 6);
    assert(usernames.size() == 6);

    // OK if no match, but returns no usernames
    assert(send_login_request("1234", "*raR*rF8KbRGhTa") == 0);
    assert(send_accounts_request("**completelynonexistent", 8) == 0);
    assert(usernames.size() == 0);

    // Should return matching usernames
    assert(send_accounts_request("*u*", 33) == 0);
    assert(usernames.size() == 2);
    assert(strcmp(usernames[0].c_str(), "otheruser") == 0);
    assert(strcmp(usernames[1].c_str(), "testuser") == 0);

    // After registration, should work
    assert(send_reg_request("iamcompletelynonexistent", "password") == 0);
    assert(send_accounts_request("**completelynonexistent", 36) == 0);
    assert(usernames.size() == 1);
    assert(strcmp(usernames[0].c_str(), "iamcompletelynonexistent") == 0);

    // Almost full match
    assert(send_accounts_request("Ea8jjQa2hzo", 8) == 0);
    assert(usernames.size() == 0);
}

int main() {
    client c;
    c.test_search_accounts();
}
