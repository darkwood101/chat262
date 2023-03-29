# Chat 262 Server

- [1. Introduction](#1-introduction)
- [2. Server](#2-server)
- [3. Database](#3-database)


## 1. Introduction

This document provides a very brief description of our implementation of the Chat 262 server. For instructions on how to run the server, see [here](instructions.md).

## 2. Server

This section briefly describes the implementation of the `server` class. For the full documentation on this class, see the [relevant header file](../include/server/server.h).

The `server` class is tasked with handling requests from Chat 262 clients and sending responses back, in accordance with Chat 262 Protocol.

The `server` class exposes only one public method, `server::run`. This method takes in command-line arguments, interprets them, and opens a listening socket on the given IP address.

For each accepted connection, the server spawns a thread to handle it. At the moment, the server can handle up to 32 concurrent connections. After the connection is established, the server waits to receive a request from the client. Depending on the request type, the server performs the appropriate action, and then sends a response to the client.

In the current implementation, the server runs indefinitely and there is no way to shut it down, other than sending it a signal.

## 3. Database

This section briefly described the implementation of the `database` class. For the full documentation on this class, see the [relevant header file](../include/server/database.h).

To support concurrent client connections, the server uses a thread-safe dabase. Any operation on the database acquires the database mutex, which prevents concurrent modifications and data races. This coarse-grained approach to synchronization guarantees correctness, but it may not scale well.

The database stores the list of currently registered users, the list of all previously used usernames, and each user's chats. For ease of implementation, each chat is replicated twice, once for each user. This allows easier account deletion.

The database also stores some per-thread state, which is equivalent to per-connection state, as there is a one-to-one mapping between threads and connections. The state stored is the username of the currently logged in user. That is, after a successful login request, the database associates the current thread's ID with the username. Then, when an operation that requires authorization is requested, the server is able to query the database on which user is logged in, if any. When the connection is terminated, or when the user sends a successful log out request, the per-thread state is removed from the database and the thread is "logged out".

This database is memory-only, which means that it's not persisted to durable storage. Upon server restart, the state is lost.
