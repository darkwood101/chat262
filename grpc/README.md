#  Chat 262 Documentation: The gRPC version

- [1. Installation and Configuration](#1-installation)
- [2. Building and Running](#2-build)
- [3. Implementation and Functionality](#3-implementation)


## 1. Installation and Configuration

You will need the following prerequisites to build Chat 262:

- Python 3.7 or higher
- pip version 9.0.1 or higher
- Install gRPC and gRPC tools:
```console
$ python -m pip install grpcio
$ python -m pip install grpcio-tools
```
For more information on getting started with gRPC, see [this tutorial](https://grpc.io/docs/languages/python/quickstart/). 


## 2. Building and Running

To generate the gRPC client and server stub code, run the following command:

```console
$ cd chat262-grpc/
$ python -m grpc_tools.protoc -I protos --python_out=. --pyi_out=. --grpc_python_out=. protos/chat.proto
```

To run the server on a specified IP address, run:

```console
$ python server.py [IP-ADDRESS]
```

To start a client:

```console
$ python client.py [IP-ADDRESS]
```
If no IP address is provided, then the default local host is used. 

## 3. Implementation and Functionality

The services and message types are defined in the protos/chat.proto file. In particular, we define two services (AuthService and ChatService) and protocol buffer message type definitions for all the request and response types used in our service methods. AuthService handles register, login, and delete account requests and ChatService handles send/receive message requests and getting all the current users. We define message types for Users, Login requests/responses, Register request/responses, Send request/responses, and ChatMessage.

The server.py file contains the service method implementations for AuthService and ChatService, handles storing and loading data from the database, and also starts up the server on a specified IP address. The database is stored on disk and updated with pickle loading and dumping so that database information persists across server restarts. Methods implemented in AuthService include Register, Login, and DeleteAccount, and methods implemented in ChatService incldue SendMessage, GetUsers, and ReceiveMessage. These functions are called from the client side using the stub code.

The client.py file includes client-side implementations for registering and logging in, sending and receiving messages, and the UI design for the chat service. There are two main "pages": a login page where a user is prompted to register or log in, and a home page where a user can see all the usernames in the database and then send chats to another user and receive incoming chats. When a user logs in, they are given all new unread messages since the last time they logged in. Messages are delivered on demand (with a slight up to one second delay), and sending + receiving messages occurs simultaneously.


