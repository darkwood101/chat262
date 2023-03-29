#include "chat.h"
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

    std::vector<std::string> correspondents;
    // Resize to nonzero to catch bugs
    correspondents.resize(1);
    // Initially no correspondents
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 0);
    assert(correspondents.size() == 0);

    // Send a text, try again
    assert(c.send_txt("otheruser", "Hello!!!", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 0);
    assert(correspondents.size() == 1);
    assert(correspondents[0] == "otheruser");

    // Receive a text, try again
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.login("1234", "*raR*rF8KbRGhTa", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("testuser", "Hello from 1234", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 0);
    assert(correspondents.size() == 2);
    // No guarantee on the ordering of correspondents, either by the protocol or
    // by the implementation, so we test for both options
    assert((correspondents[0] == "otheruser" && correspondents[1] == "1234") ||
           (correspondents[0] == "1234" && correspondents[1] == "otheruser"));
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // Send text to yourself, and then retrieve
    assert(c.login("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("otheruser", "", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(correspondents.size() == 2);
    assert(
        (correspondents[0] == "otheruser" && correspondents[1] == "testuser") ||
        (correspondents[0] == "testuser" && correspondents[1] == "otheruser"));
    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);

    // Retrieving if not logged in is unauthorized
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 6);
    // Must not be touched from the previous call
    assert(correspondents.size() == 2);
    assert(
        (correspondents[0] == "otheruser" && correspondents[1] == "testuser") ||
        (correspondents[0] == "testuser" && correspondents[1] == "otheruser"));

    // Wrong log in
    assert(c.login("testuser", "passworD", stat_code) == status::ok);
    assert(stat_code == 1);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 6);
    assert(correspondents.size() == 2);
    assert(
        (correspondents[0] == "otheruser" && correspondents[1] == "testuser") ||
        (correspondents[0] == "testuser" && correspondents[1] == "otheruser"));

    return EXIT_SUCCESS;
}
