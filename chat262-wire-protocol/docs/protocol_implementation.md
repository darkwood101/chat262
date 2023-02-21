# Chat 262 Protocol Implementation

- [1. Introduction](#1-introduction)
- [2. Structure Definitions](#2-structure-definitions)
- [3. Example](#3-example)


## 1. Introduction

This document provides a very brief description of our implementation of the Chat 262 Protocol. For the full documentation of available interfaces in our implementation, please refer to the [relevant header file](../include/chat262_protocol/chat262_protocol.h). For the specification of the protocol, see [here](chat262_protocol.md).

To facilitate the usage of the Chat 262 Protocol, we provide a C++ implementation. This implementation is also used by the Chat 262 server and the Chat 262 client.

## 2. Structure Definitions

Our implementation defines `message_header` structure according to the specification. The definition is as follows:
```C++
struct message_header {
    uint16_t version_;
    uint16_t type_;
    uint32_t body_len_;

    static status deserialize(const std::vector<uint8_t>& data,
                              message_header& hdr);
};
```
The `deserialize` method takes in a vector of bytes, which is usually received over the network, and populates `hdr` accordingly.

The message structure is defined as follows:
```C++
struct message {
    message_header hdr_;
    uint8_t body_[];
};
```
Since the length of the message body depends on the type of the message and its contents, it's defined as a flexible array member. This is NOT in accordance with the C++ standard, but it is a well-documented extension in all relevant compilers.

Each request/response type defines `serialization` and `deserialization` interfaces. As an example, we show the registration request:
```C++
struct registration_request {
    // Layout from the specification:
    //
    // uint32_t username_length;
    // uint32_t password_length;
    // uint8_t username[username_length];
    // uint8_t password[password_length]

    static std::shared_ptr<message> serialize(const std::string& username,
                                              const std::string& password);

    static status deserialize(const std::vector<uint8_t>& data,
                              std::string& username,
                              std::string& password);
};
```
The `serialize` interface will return a shared pointer to a complete `message` that contains the registration request. The `deserialize` interface takes in a vector of bytes that contains the body of the registration request, and populates the username and password strings.

The user of the implementation should **NEVER** instantiate a `message` without calling the appropriate serialization method. This is due to three reasons:

1. Instantiating `message` requires dynamic heap allocations of appropriate size, which are difficult to get right.
2. Alignment requirements for various data types might be violated. This is undefined behavior and causes an exception on some architectures.
3. Bytes in `message` must be stored in little-endian order, so just assigning to struct members is incorrect.

The user of the implementation should also **NEVER** interpret bytes in a received message without calling appropriate deserialization methods. This is due to two reasons:

1. Alignment requirements for various data types might be violated. This is undefined behavior and causes an exception on some architectures.
2. Bytes in `message` must be stored in little-endian order, so just reading from struct members is incorrect.

## 3. Example

This section demonstrates an example of using our Chat 262 Protocol implementation.

The client typically calls `serialize` for request message types, and the server calls `deserialize` on request message types. Preparing a send text message might look like this:
```C++
std::string recipient("user");
std::string txt("Hello, user! How are you?");

std::shared_ptr<chat262::message> msg = chat262::send_txt_request::serialize(recipient, txt);

// ...
// Send `msg` through a socket.
// The size to send is `sizeof(chat262::message_header) + e_le32toh(msg->hdr_.body_len_)`

```
The server might interpret this message like this:
```C++
std::vector<uint8_t> hdr_data;

// ...
// Receive a header into `hdr_data`
// The size to receive is always `sizeof(chat262::message_header)`
// ...

chat262::message_header hdr;
status s = chat262::message_header::deserialize(data, hdr);
if (s != status::ok) {
    // handle error
}

if (hdr.type_ != chat262::msgtype_registration_request) {
    // handle some other request
}

std::vector<uint8_t> body_data;

// ...
// Receive a body into `body_data`
// ...

std::string username;
std::string password;

chat262::registration_request::deserialize(data, username, password);

// ...
// `username` and `password` now contain the username and password
// the client wanted to send

```
