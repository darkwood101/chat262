#include "chat.h"
#include "chat262_protocol.h"
#include "endianness.h"
#include "server.h"

#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <vector>

// The client doesn't expose the `send_msg` and `recv_*` interfaces, so we have
// to copy them here. Pretty ugly, but oh well.

constexpr uint32_t n_ip_addr = 0x0100007F;

int server_fd = -1;

static void connect_server() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd > 0);

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(chat262::port);
    server_addr.sin_addr.s_addr = n_ip_addr;

    assert(connect(server_fd,
                   (const sockaddr*) &server_addr,
                   sizeof(server_addr)) == 0);
}

static status send_msg(std::shared_ptr<chat262::message> msg) {
    size_t total_sent = 0;
    ssize_t sent = 0;
    size_t total_len = sizeof(chat262::message_header) + msg->hdr_.body_len_;
    while (total_sent != total_len) {
        sent = send(server_fd, msg.get(), total_len, MSG_NOSIGNAL);
        if (sent < 0) {
            return status::send_error;
        }
        total_sent += sent;
    }
    return status::ok;
}

static status recv_hdr(chat262::message_header& hdr) {
    std::vector<uint8_t> hdr_data;
    hdr_data.resize(sizeof(chat262::message_header));
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != sizeof(chat262::message_header)) {
        readed =
            read(server_fd, hdr_data.data(), sizeof(chat262::message_header));
        if (readed < 0) {
            return status::receive_error;
        } else if (readed == 0) {
            return status::closed_connection;
        }
        total_read += readed;
    }
    // This should always succeed
    chat262::message_header::deserialize(hdr_data, hdr);
    return status::ok;
}

static status recv_body(uint32_t body_len, std::vector<uint8_t>& data) {
    data.resize(body_len);
    size_t total_read = 0;
    ssize_t readed = 0;
    while (total_read != body_len) {
        readed = read(server_fd, data.data(), body_len);
        if (readed < 0) {
            return status::receive_error;
        } else if (readed == 0) {
            return status::closed_connection;
        }
        total_read += readed;
    }
    return status::ok;
}

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

    connect_server();

    std::shared_ptr<chat262::message> msg;
    chat262::message_header hdr;
    std::vector<uint8_t> body;
    uint16_t correct_version;
    uint32_t stat_code;
    std::vector<std::string> matched_usernames;

    // Send a registration request with wrong version
    msg = chat262::registration_request::serialize("testuser", "password");
    assert(e_le16toh(msg->hdr_.version_) == 1);
    assert(e_le16toh(msg->hdr_.type_) == 101);
    assert(e_le32toh(msg->hdr_.body_len_) == 24);
    msg->hdr_.version_ = e_htole32(2);
    assert(send_msg(msg) == status::ok);
    // We should receive a wrong version response
    assert(recv_hdr(hdr) == status::ok);
    assert(e_le16toh(hdr.version_) == 1);
    assert(e_le16toh(hdr.type_) == 301);
    assert(e_le32toh(hdr.body_len_) == 2);
    assert(recv_body(2, body) == status::ok);
    assert(
        chat262::wrong_version_response::deserialize(body, correct_version) ==
        status::ok);
    assert(correct_version == 1);

    // Server should have closed the connection
    assert(recv_hdr(hdr) == status::closed_connection);

    close(server_fd);
    connect_server();

    // Send a request with invalid type
    msg = chat262::accounts_request::serialize("*");
    assert(e_le16toh(msg->hdr_.version_) == 1);
    assert(e_le16toh(msg->hdr_.type_) == 104);
    assert(e_le32toh(msg->hdr_.body_len_) == 5);
    msg->hdr_.type_ = e_htole16(262);
    assert(send_msg(msg) == status::ok);
    // We should receive an invalid type response
    assert(recv_hdr(hdr) == status::ok);
    assert(e_le16toh(hdr.version_) == 1);
    assert(e_le16toh(hdr.type_) == 302);
    assert(e_le32toh(hdr.body_len_) == 0);
    assert(recv_body(0, body) == status::ok);
    assert(chat262::invalid_type_response::deserialize(body) == status::ok);

    // The socket should still be open, and a normal request should go through
    msg = chat262::accounts_request::serialize("*");
    assert(e_le16toh(msg->hdr_.version_) == 1);
    assert(e_le16toh(msg->hdr_.type_) == 104);
    assert(e_le32toh(msg->hdr_.body_len_) == 5);
    assert(send_msg(msg) == status::ok);
    assert(recv_hdr(hdr) == status::ok);
    assert(e_le16toh(hdr.version_) == 1);
    assert(e_le16toh(hdr.type_) == 204);
    assert(e_le32toh(hdr.body_len_) == 4);
    assert(recv_body(4, body) == status::ok);
    assert(chat262::accounts_response::deserialize(body,
                                                   stat_code,
                                                   matched_usernames) ==
           status::ok);
    assert(stat_code == 6);

    body.resize(0);
    // Lie about the body size, and send less bytes than we should
    msg = chat262::registration_request::serialize("testuser", "password");
    assert(e_le16toh(msg->hdr_.version_) == 1);
    assert(e_le16toh(msg->hdr_.type_) == 101);
    assert(e_le16toh(msg->hdr_.body_len_) == 24);
    msg->hdr_.body_len_ = e_htole32(20);
    assert(send_msg(msg) == status::ok);
    // We should receive an invalid body response
    assert(recv_hdr(hdr) == status::ok);
    assert(e_le16toh(hdr.version_) == 1);
    assert(e_le16toh(hdr.type_) == 303);
    assert(e_le32toh(hdr.body_len_) == 0);
    assert(chat262::invalid_body_response::deserialize(body) == status::ok);

    // Lie about the body size, and send more bytes than we should -- we need to
    // hack the message manually
    void* m = malloc(sizeof(chat262::message_header) + 100);
    memset(m, 0, sizeof(chat262::message_header) + 100);
    assert(m);
    chat262::message_header* h = static_cast<chat262::message_header*>(m);
    h->version_ = e_htole16(1);
    h->type_ = e_htole16(101);
    h->body_len_ = e_htole32(sizeof(chat262::message_header) + 100);
    size_t total_sent = 0;
    ssize_t sent = 0;
    size_t total_len = sizeof(chat262::message_header) + h->body_len_;
    while (total_sent != total_len) {
        sent = send(server_fd, h, total_len, MSG_NOSIGNAL);
        assert(sent > 0);
        total_sent += sent;
    }
    // We should receive an invalid body response
    assert(recv_hdr(hdr) == status::ok);
    assert(e_le16toh(hdr.version_) == 1);
    assert(e_le16toh(hdr.type_) == 303);
    assert(e_le32toh(hdr.body_len_) == 0);
    assert(chat262::invalid_body_response::deserialize(body) == status::ok);

    // Send a header and then close the connection, the server should still be
    // up and running
    total_sent = 0;
    while (total_sent != sizeof(chat262::message_header)) {
        sent =
            send(server_fd, h, sizeof(chat262::message_header), MSG_NOSIGNAL);
        assert(sent > 0);
        total_sent += sent;
    }
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);

    connect_server();
    msg = chat262::accounts_request::serialize("*");
    assert(e_le16toh(msg->hdr_.version_) == 1);
    assert(e_le16toh(msg->hdr_.type_) == 104);
    assert(e_le32toh(msg->hdr_.body_len_) == 5);
    assert(send_msg(msg) == status::ok);
    assert(recv_hdr(hdr) == status::ok);
    assert(e_le16toh(hdr.version_) == 1);
    assert(e_le16toh(hdr.type_) == 204);
    assert(e_le32toh(hdr.body_len_) == 4);
    assert(recv_body(4, body) == status::ok);
    assert(chat262::accounts_response::deserialize(body,
                                                   stat_code,
                                                   matched_usernames) ==
           status::ok);
    assert(stat_code == 6);

    free(m);

    return EXIT_SUCCESS;
}
