#  Chat 262 Documentation: Replication and Persistence

- [1. Installation and Configuration](#1-installation)
- [2. Building and Running](#2-build)
- [3. Implementation and Functionality](#3-implementation)

This implements a two-fault tolerant and persistent chatbot application.

## 1. Installation and Configuration

See the [README.md](https://github.com/darkwood101/chat262/tree/main/chat262-grpc) for the gRPC version for installing and configuring gRPC.

## 2. Building and Running

To generate the gRPC client and server stub code, run the following command:

```console
$ cd chat262-grpc/
$ python -m grpc_tools.protoc -I protos --python_out=. --pyi_out=. --grpc_python_out=. protos/chat.proto
```

To test out the two-fault tolerance and persistence capabilities, you can start up 3 servers (in the following order)

```console
$ python3 server.py 2 127.0.0.1 127.0.0.2 127.0.0.3
$ python3 server.py 1 127.0.0.1 127.0.0.2 127.0.0.3
$ python3 server.py 0 127.0.0.1 127.0.0.2 127.0.0.3
```

Then start up the client(s):
```console
$ python3 client.py 127.0.0.1 127.0.0.2 127.0.0.3
```

## 3. Implementation and Functionality

#### Persistence Design Decisions:
* To make message store persistent across server restarts, we create a .pkl database file associated with each of the three servers. These database files are maintained to have the same message content through our replication scheme explained below.
* Since the client is only communicating with the leader, when changes to the leader’s database are made through gRPC requests, the leader transfers these requests to the non-leader servers who then make the corresponding changes to their own databases. 
* We persist all user data, including username-password pairs and messages sent/received by all users.

#### Replication Design Decisions:
* Setup: we have 3 servers and 1 client communicating with the leader server, to ensure that our system is 2-fault tolerant in the face of crash/failstop failures.
* Leader election/order: 
  * We establish a pre-set leader order for transferring leadership in the case of crashes: server 0, server 1, server 2. All entities (the client and the
  servers) are aware of this order as well as the IP addresses of the other entities.
  * When the leader crashes, the client automatically connects to the next server as determined by the leader order above. If a server receives a message
  from the client, it assumes that it has become the leader.
* Detecting server crashes: When the client sends any request to any server, if it doesn’t receive a response within a timeout period of 1 second, it assumes that the server has crashed or failed. The client then connects with the next server, who is established to be the next leader.


