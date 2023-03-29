#include "chat262_protocol.h"
#include "client.h"
#include "server.h"

#include <arpa/inet.h>
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
    assert(c.registration("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.registration("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.registration("1234", "*raR*rF8KbRGhTa", stat_code) == status::ok);
    assert(stat_code == 0);

    // Normal sends should work
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("otheruser", "Hello!!!", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("1234", "HI!", stat_code) == status::ok);
    assert(stat_code == 0);

    // Sending to yourself should work
    assert(c.send_txt("testuser", "Hi myself", stat_code) == status::ok);
    assert(stat_code == 0);

    // Sending an empty text is ok
    assert(c.send_txt("1234", "", stat_code) == status::ok);
    assert(stat_code == 0);

    // Sending to a non-existing user should fail
    assert(c.send_txt("2345", "Oops", stat_code) == status::ok);
    assert(stat_code == 3);
    assert(c.send_txt("", "wrong", stat_code) == status::ok);
    assert(stat_code == 3);

    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // Sending if not logged in is unauthorized
    assert(c.send_txt("testuser", "hi", stat_code) == status::ok);
    assert(stat_code == 6);
    assert(c.send_txt("otheruser", "unauthorized", stat_code) == status::ok);
    assert(stat_code == 6);

    // Sending to a non-existing user should still be unauthorized
    assert(c.send_txt("2345", "hello", stat_code) == status::ok);
    assert(stat_code == 6);
    assert(c.send_txt("", "wget", stat_code) == status::ok);
    assert(stat_code == 6);
    assert(c.send_txt("", "", stat_code) == status::ok);
    assert(stat_code == 6);

    return EXIT_SUCCESS;
}
