#include "chat262_protocol.h"
#include "client.h"
#include "server.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

constexpr uint32_t n_ip_addr = 0x0100007F;

static void spawn_server() {
    const char* localhost = "127.0.0.1";
    char const* argv[] = {"./server", localhost};
    std::thread thread([&]() {
        server s;
        s.run(2, argv);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    thread.detach();
}

int main() {
    spawn_server();

    client c;
    assert(c.connect_server(n_ip_addr) == status::ok);

    uint32_t stat_code;
    // Registering a user should work
    assert(c.registration("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    // Registering the same user should fail with status code 2
    assert(c.registration("testuser", "otherpassword", stat_code) == status::ok);
    assert(stat_code == 2);
    // Registering another user should work
    assert(c.registration("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);

    // Registering user with asterisk in username should fail with status code 4
    assert(c.registration("A2zpsuE*HbVs", "cQ7Kdtov394x", stat_code) == status::ok);
    assert(stat_code == 4);
    // Registering user with whitespace in username should fail with status code 4
    assert(c.registration("hXGE C3pE pA", "HWLNWEdk9Hed", stat_code) == status::ok);
    assert(stat_code == 4);
    // Whitespace in password should work
    assert(c.registration("JoP4fqkvVpBQ", "                ", stat_code) == status::ok);
    assert(stat_code == 0);
    // Asterisks in passwords should work
    assert(c.registration("Ea8jjQa2hzom", "*******", stat_code) == status::ok);
    assert(stat_code == 0);

    // Username too short
    assert(c.registration("", "Xj3wMvbJ9cfaKs2zk7L66ENZVT", stat_code) == status::ok);
    assert(stat_code == 4);
    assert(c.registration("aaa", "-X83!C7MmRHyrNAPsehKXx", stat_code) == status::ok);
    assert(stat_code == 4);
    // Username of borderline length
    assert(c.registration("1234", "*raR*rF8KbRGhTa", stat_code) == status::ok);
    assert(stat_code == 0);
    // Username too long
    assert(c.registration("dXGACsiGPzJucVdzAKDd8qkGbZs2AkLMDn679kmyhpEjpviKsn",
                        "j43znyi97oHpnk73eVi9", stat_code) == status::ok);
    assert(stat_code == 4);
    assert(c.registration("k4iVyqCsz7rK8sGHGc93hXH4f8ut6Xpza7gnmLVAo",
                        "aVLcLiUBVKBmzRTrTGDsVU3Dz6", stat_code) == status::ok);
    assert(stat_code == 4);
    // Username of borderline length
    assert(c.registration("AiBya377TNiVGymD8QLqDrpzdPnhgXJLtQi9LhZ7",
                        "ZhaJsKXeALg4sJMgHhcen3J", stat_code) == status::ok);
    assert(stat_code == 0);

    // Password too short
    assert(c.registration("9NxTyLzR4b3Czfdoa", "psw", stat_code) == status::ok);
    assert(stat_code == 5);
    assert(c.registration("3PMgbTmj", "", stat_code) == status::ok);
    assert(stat_code == 5);
    // Password of borderline length
    assert(c.registration("3_BKn.4C", "pswd", stat_code) == status::ok);
    assert(stat_code == 0);
    // Password too long
    assert(c.registration("2x2-J@E2",
                        "Z4hv4my8LZxvPc2crbBFPmqKTPNYBATNhVwvpsMMGuUy6AzkdXvksn"
                        "nki86ui3dDX62mm8", stat_code) == status::ok);
    assert(stat_code == 5);
    assert(
        c.registration(
            "tgC8qbEkRUn",
            "NazQ9zTACCycs6a3xPBRTYArd3cTgNd9Few6DkjpfqKxKeKzZy7ZQPJwKExJK", stat_code) == status::ok);
    assert(stat_code == 5);
    // Password of borderline length
    assert(
        c.registration(
            "J-_WwFLJeJ3",
            "9a*4mHEsrL9VKRsn6VrqRXFEoHH*7Mdupp4koZc*LXgCuTx7oVE_R!bRaiPc", stat_code) == status::ok);
    assert(stat_code == 0);

    // If both username and password invalid, username takes precedence
    assert(c.registration("", "", stat_code) == status::ok);
    assert(stat_code == 4);
    assert(c.registration("use", "psw", stat_code) == status::ok);
    assert(stat_code == 4);
    assert(c.registration("us",
                        "KwEpZ-gTg.FpTCm4kuVwfx-rRoZno@_WP@kUu@xAq!3TcznKQ4cyr*"
                        "QAJkX6X6Lamw", stat_code) == status::ok);
    assert(stat_code == 4);
    assert(c.registration("uvEE4RbgnCJBEdKWbkanAoXEqitNozfutxw2338jjD6bGHv2no2",
                        "_!n3ifG2TfMftXjxvL*EPi7dDnc97KVKiqqCm2hP@wDj!zAT@"
                        "aA47eCHHuCgibXJhL", stat_code) == status::ok);
    assert(stat_code == 4);
    assert(c.registration("mk8qhBZoABLReQJUixACWE7xE3nqRxtXp76jgpgNK6eD3g7C2jY",
                        "ok", stat_code) == status::ok);
    assert(stat_code == 4);

    // Try registering an existing user again
    assert(c.registration("3_BKn.4C", "pswd", stat_code) == status::ok);
    assert(stat_code == 2);

    return EXIT_SUCCESS;
}
