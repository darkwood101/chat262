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

    chat curr_chat;
    // Send and retrieve a text
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("otheruser", "Hello!!!", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_txt("otheruser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 1);
    assert(curr_chat.texts_[0].sender_ == text::sender_you);
    assert(curr_chat.texts_[0].content_ == "Hello!!!");
    assert(c.logout(stat_code) == status::ok);

    // The other user logs in and retrieves the text
    assert(c.login("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_txt("testuser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 1);
    assert(curr_chat.texts_[0].sender_ == text::sender_other);
    assert(curr_chat.texts_[0].content_ == "Hello!!!");

    // Retrieve from a third user should give an empty chat
    assert(c.recv_txt("1234", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 0);

    // Retrieve from a non-existing user
    assert(c.recv_txt("nonexisting", stat_code, curr_chat) == status::ok);
    assert(stat_code == 3);
    // Should not change
    assert(curr_chat.texts_.size() == 0);

    assert(c.logout(stat_code) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 0);

    // Retrieve without logging in is unauthorized
    assert(c.recv_txt("1234", stat_code, curr_chat) == status::ok);
    assert(stat_code == 6);
    assert(curr_chat.texts_.size() == 0);

    // Retrieve from a non-existing user while logged out is still unauthorized
    assert(c.recv_txt("nonexist", stat_code, curr_chat) == status::ok);
    assert(stat_code == 6);

    // Send two texts after a double login
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.login("otheruser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("testuser", "HI   !***", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("testuser", "", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.login("testuser", "password", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_txt("otheruser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 3);
    assert(curr_chat.texts_[0].sender_ == text::sender_you);
    assert(curr_chat.texts_[0].content_ == "Hello!!!");
    assert(curr_chat.texts_[1].sender_ == text::sender_other);
    assert(curr_chat.texts_[1].content_ == "HI   !***");
    assert(curr_chat.texts_[2].sender_ == text::sender_other);
    assert(curr_chat.texts_[2].content_ == "");

    // Retrieve an empty chat from yourself
    assert(c.recv_txt("testuser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 0);

    // Text yourself and retrieve again
    assert(c.send_txt("testuser", "text number one", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.send_txt("testuser", "text number two", stat_code) == status::ok);
    assert(stat_code == 0);
    assert(c.recv_txt("testuser", stat_code, curr_chat) == status::ok);
    assert(stat_code == 0);
    assert(curr_chat.texts_.size() == 4);
    // Sender comes before the recipient, if sending to yourself.
    // This is not specified by the Chat 262 protocol, but is an implementation
    // detail.
    assert(curr_chat.texts_[0].sender_ == text::sender_you);
    assert(curr_chat.texts_[0].content_ == "text number one");
    assert(curr_chat.texts_[1].sender_ == text::sender_other);
    assert(curr_chat.texts_[1].content_ == "text number one");
    assert(curr_chat.texts_[2].sender_ == text::sender_you);
    assert(curr_chat.texts_[2].content_ == "text number two");
    assert(curr_chat.texts_[3].sender_ == text::sender_other);
    assert(curr_chat.texts_[3].content_ == "text number two");

    return EXIT_SUCCESS;
}
