#include "chat262_protocol.h"
#include "client.h"
#include "server.h"

#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

void client::test_registration() {
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

    auto send_request = [&](const char* username, const char* password) {
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
        chat262::registration_response::deserialize(data, stat_code);
        return stat_code;
    };

    // Registering a user should work
    assert(send_request("testuser", "password") == 0);
    // Registering the same user should fail with status code 2
    assert(send_request("testuser", "otherpassword") == 2);
    // Registering another user should work
    assert(send_request("otheruser", "password") == 0);
    // Registering user with asterisk in username should fail with status code 4
    assert(send_request("A2zpsuE*HbVs", "cQ7Kdtov394x") == 4);
    // Registering user with whitespace in username should fail with status code
    // 4
    assert(send_request("hXGE C3pE pA", "HWLNWEdk9Hed") == 4);
    // Whitespace in password should work
    assert(send_request("JoP4fqkvVpBQ", "                ") == 0);
    // Asterisks in passwords should work
    assert(send_request("Ea8jjQa2hzom", "*******") == 0);

    // Username too short
    assert(send_request("", "Xj3wMvbJ9cfaKs2zk7L66ENZVT") == 4);
    assert(send_request("aaa", "-X83!C7MmRHyrNAPsehKXx") == 4);
    // Username of borderline length
    assert(send_request("1234", "*raR*rF8KbRGhTa") == 0);
    // Username too long
    assert(send_request("dXGACsiGPzJucVdzAKDd8qkGbZs2AkLMDn679kmyhpEjpviKsn",
                        "j43znyi97oHpnk73eVi9") == 4);
    assert(send_request("k4iVyqCsz7rK8sGHGc93hXH4f8ut6Xpza7gnmLVAo",
                        "aVLcLiUBVKBmzRTrTGDsVU3Dz6") == 4);
    // Username of borderline length
    assert(send_request("AiBya377TNiVGymD8QLqDrpzdPnhgXJLtQi9LhZ7",
                        "ZhaJsKXeALg4sJMgHhcen3J") == 0);

    // Password too short
    assert(send_request("9NxTyLzR4b3Czfdoa", "psw") == 5);
    assert(send_request("3PMgbTmj", "") == 5);
    // Password of borderline length
    assert(send_request("3_BKn.4C", "pswd") == 0);
    // Password too long
    assert(send_request("2x2-J@E2",
                        "Z4hv4my8LZxvPc2crbBFPmqKTPNYBATNhVwvpsMMGuUy6AzkdXvksn"
                        "nki86ui3dDX62mm8") == 5);
    assert(
        send_request(
            "tgC8qbEkRUn",
            "NazQ9zTACCycs6a3xPBRTYArd3cTgNd9Few6DkjpfqKxKeKzZy7ZQPJwKExJK") ==
        5);
    // Password of borderline length
    assert(
        send_request(
            "J-_WwFLJeJ3",
            "9a*4mHEsrL9VKRsn6VrqRXFEoHH*7Mdupp4koZc*LXgCuTx7oVE_R!bRaiPc") ==
        0);

    // If both username and password invalid, username takes precedence
    assert(send_request("", "") == 4);
    assert(send_request("use", "psw") == 4);
    assert(send_request("us",
                        "KwEpZ-gTg.FpTCm4kuVwfx-rRoZno@_WP@kUu@xAq!3TcznKQ4cyr*"
                        "QAJkX6X6Lamw") == 4);
    assert(send_request("uvEE4RbgnCJBEdKWbkanAoXEqitNozfutxw2338jjD6bGHv2no2",
                        "_!n3ifG2TfMftXjxvL*EPi7dDnc97KVKiqqCm2hP@wDj!zAT@"
                        "aA47eCHHuCgibXJhL") == 4);
    assert(send_request("mk8qhBZoABLReQJUixACWE7xE3nqRxtXp76jgpgNK6eD3g7C2jY",
                        "ok") == 4);
}

int main() {
    client c;
    c.test_registration();
}
