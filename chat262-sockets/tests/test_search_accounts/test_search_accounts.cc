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
constexpr const char* test_usernames[] = {"1234",
                                          "3_BKn.4C",
                                          "Ea8jjQa2hzom",
                                          "JoP4fqkvVpBQ",
                                          "otheruser",
                                          "testuser"};

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

    std::vector<std::string> matched_usernames;
    // Should return all usernames
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.list_accounts("*", stat_code, matched_usernames) == status::ok);
    assert(stat_code == 0);
    assert(matched_usernames.size() == 6);
    for (int i = 0; i != 6; ++i) {
        assert(strcmp(matched_usernames[i].c_str(), test_usernames[i]) == 0);
    }
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // Unauthorized if not logged in
    assert(c.list_accounts("*", stat_code, matched_usernames) == status::ok);
    assert(stat_code == 6);
    // Matched usernames should be unchanged
    assert(matched_usernames.size() == 6);

    // OK if no match, but returns no usernames
    assert(c.login("1234", "*raR*rF8KbRGhTa", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.list_accounts("**completelynonexistent",
                           stat_code,
                           matched_usernames) == status::ok);
    assert(stat_code == 0);
    assert(matched_usernames.size() == 0);

    // Should return matching usernames
    assert(c.list_accounts("*u*", stat_code, matched_usernames) == status::ok);
    assert(stat_code == 0);
    assert(matched_usernames.size() == 2);
    assert(strcmp(matched_usernames[0].c_str(), "otheruser") == 0);
    assert(strcmp(matched_usernames[1].c_str(), "testuser") == 0);

    // After registration, should work
    assert(c.registration("iamcompletelynonexistent", "password", stat_code) ==
           status::ok);
    assert(stat_code == 0);
    assert(c.list_accounts("**completelynonexistent",
                           stat_code,
                           matched_usernames) == status::ok);
    assert(stat_code == 0);
    assert(matched_usernames.size() == 1);
    assert(strcmp(matched_usernames[0].c_str(), "iamcompletelynonexistent") ==
           0);

    // Almost full match
    assert(c.list_accounts("Ea8jjQa2hzo", stat_code, matched_usernames) ==
           status::ok);
    assert(stat_code == 0);
    assert(matched_usernames.size() == 0);

    return EXIT_SUCCESS;
}
