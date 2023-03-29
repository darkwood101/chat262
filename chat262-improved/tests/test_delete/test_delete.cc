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

    std::vector<std::string> matched_usernames;
    std::vector<std::string> correspondents;
    chat curr_chat;

    // After deleting an account, cannot log back in
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.delete_account(stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 1);
    // Other operations should fail with unauthorized
    assert(c.list_accounts("*", stat_code, matched_usernames) == status::ok);
    assert(stat_code == 6);
    assert(matched_usernames.size() == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 6);
    assert(c.recv_txt("otheruser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 6);
    assert(c.send_txt("otheruser", "hello", stat_code) == status::ok);
    assert(stat_code == 6);

    // Cannot register with the same username again
    assert(c.registration("testuser", "newpassword", stat_code) == status::ok);
    assert(stat_code == 2);

    // Other users cannot interact with this user anymore
    assert(c.login("otheruser", "password", stat_code) == status::ok);
    assert(c.list_accounts("testuser", stat_code, matched_usernames) ==
           status::ok);
    assert(stat_code == 0);
    assert(matched_usernames.size() == 0);
    assert(c.recv_txt("testuser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 3);
    assert(c.send_txt("testuser", "hello", stat_code) == status::ok);
    assert(stat_code == 3);

    // Register a new user
    assert(c.registration("newuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);

    // Send some texts back and forth
    assert(c.send_txt("1234", "hello", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("newuser", "hello to you too", stat_code) == status::ok);
    assert(stat_code == 0);

    // Other users can see the texts
    assert(c.login("newuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 0);
    assert(correspondents.size() == 1);
    assert(correspondents[0] == "otheruser");
    assert(c.recv_txt("otheruser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 1);
    assert(curr_chat.texts_[0].sender_ == text::sender_other);
    assert(curr_chat.texts_[0].content_ == "hello to you too");

    // Delete account
    assert(c.login("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.delete_account(stat_code) == status::ok);
    assert(stat_code == 0);

    // Neither user can see the texts anymore
    assert(c.login("newuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 0);
    assert(correspondents.size() == 0);
    assert(c.recv_txt("otheruser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 3);
    assert(c.login("1234", "*raR*rF8KbRGhTa", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_correspondents(stat_code, correspondents) == status::ok);
    assert(stat_code == 0);
    assert(correspondents.size() == 0);
    assert(c.recv_txt("otheruser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 3);

    return EXIT_SUCCESS;
}
