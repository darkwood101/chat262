#include "chat262_protocol.h"
#include "client.h"
#include "server.h"

#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

void client::test_logout() {
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
        chat262::login_response::deserialize(data, stat_code);
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
        chat262::logout_response::deserialize(data, stat_code);
        return stat_code;
    };

    assert(send_login_request("testuser", "password") == 0);
    assert(send_logout_request() == 0);

    // Unauthorized if not logged in
    assert(send_logout_request() == 6);

    // Unauthorized logout if invalid credentials
    assert(send_login_request("testuser", "wrongpassword") == 1);
    assert(send_logout_request() == 6);

    // OK if two login requests
    assert(send_login_request("JoP4fqkvVpBQ", "                ") == 0);
    assert(send_login_request("3_BKn.4C", "pswd") == 0);
    assert(send_logout_request() == 0);

    // OK if logged in on second attempt
    assert(send_login_request("3_BKn.4", "pswd") == 1);
    assert(send_login_request("Ea8jjQa2hzom", "*******") == 0);
    assert(send_logout_request() == 0);

    // Not OK if second login invalid
    assert(send_login_request("3_BKn.4C", "pswd") == 0);
    assert(send_login_request("3_BKn.4C", "ooops") == 1);
    assert(send_logout_request() == 6);
}

int main() {
    client c;
    c.test_logout();
}
