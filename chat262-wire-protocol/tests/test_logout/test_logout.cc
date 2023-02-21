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
    assert(c.registration("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.registration("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.registration("JoP4fqkvVpBQ", "                ", stat_code) ==
           status::ok);
    assert(stat_code == 0);
    assert(c.registration("Ea8jjQa2hzom", "*******", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.registration("1234", "*raR*rF8KbRGhTa", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.registration("3_BKn.4C", "pswd", stat_code) == status::ok);
    assert(stat_code == 0);

    // Normal login and logout
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // Unauthorized if not logged in
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 6);

    // Unauthorized logout if invalid credentials
    assert(c.login("testuser", "wrongpassword", stat_code) == status::ok);
    assert(stat_code == 1);
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 6);

    // OK if two login requests
    assert(c.login("JoP4fqkvVpBQ", "                ", stat_code) ==
           status::ok);
    assert(stat_code == 0);
    assert(c.login("3_BKn.4C", "pswd", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // OK if logged in on second attempt
    assert(c.login("3_BKn.4", "pswd", stat_code) == status::ok);
    assert(stat_code == 1);
    assert(c.login("Ea8jjQa2hzom", "*******", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // Not OK if second login invalid
    assert(c.login("3_BKn.4C", "pswd", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.login("3_BKn.4C", "ooops", stat_code) == status::ok);
    assert(stat_code == 1);
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 6);

    return EXIT_SUCCESS;
}
