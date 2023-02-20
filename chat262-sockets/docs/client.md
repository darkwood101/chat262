# Chat 262 Client

- [1. Introduction](#1-introduction)
- [2. Client](#2-client)
- [3. Example](#3-example)
- [4. Interface](#4-interface)


## 1. Introduction

This document provides a very brief description of our implementation of the Chat 262 client.

## 2. Client

This section briefly describes the implementation of the `client` class. For the full documentation on this class, see the [relevant header file](../include/client/client.h).

The `client` class is tasked with communicating requests to a Chat 262 server and interpreting the responses from the server, in accordance with Chat 262 Protocol.

Each publicly exposed method in the `client` class returns a `status` enum value. A value of `status::ok` indicates that the operation went as expected.

Before calling any other method, `connect_server` must be called first. This method takes in the IP address of a Chat 262 server, and attempts to establish a connection to the server.

After the connection is established, any of the other available interfaces can be called. The interfaces available correspond to operations supported by Chat 262. For instance, to send a login request, `client::login` should be used. The information returned by the server, such as status codes, are passed as parameters to the client interfaces.

The client does not know how to react to special server responses (wrong version, invalid type, and invalid body). In case of these responses, the client will return a `status::header_error`. The error should be handled at higher levels of the application. Note that these responses should not occur if both the server and the client comply with the Chat 262 Protocol specification.

## 3. Example

This section demonstrates an example of using our Chat 262 client implementation.

```C++
// localhost
constexpr uint32_t ip_addr = 0x0100007F;

status s;
uint32_t stat_code;

client c;
s = c.connect_server(ip_addr);
if (s != status::ok) {
    // handle error
}

s = c.registration("user", "password", stat_code);
if (s != status::ok) {
    // handle error
}

if (stat_code == chat262::status_code_ok) {
    // All ok
} else {
    // The server refused to allow registration
}

```

## 4. Interface

This section briefly describes the implementation of the `interface` class. For the full documentation on this class, see the [relevant header file](../include/client/interface.h).

The `interface` class faces the user of the Chat 262 application, and is tasked with driving the client.

The interface is implemented as a state machine, where a state is equivalent to a screen that is displayed to the user. The interface interprets user input and accordingly determines how the state changes.

The interface is single-threaded for most of its execution, except when a chat is open and the user is typing the message. In order to implement automatic message delivery, the interface spawns a another thread. One thread listens to user input, while another thread sends periodic requests to the server to receive new messages. If a new message is received, the screen is cleared, and the message is printed to the screen. To avoid losing user output from the screen due to line buffering, the interface switches the terminal to non-canonical mode.

We mentioned in [Section 2](#2-client) that the client does not know what to do in case of a special sever response or another kind of error, and that this should be handled at higher levels. Our current interface implementation does not attempt to recover from these kinds of errors, and silently exits.
