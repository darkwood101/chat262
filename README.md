# Chat 262

Chat 262 is a basic chatting app that uses a server-client architecture. It offers the following features:

- Register an account using a username and a password
- Look up other Chat 262 users
- Chat with other Chat 262 users in real time
- Delete your account and all your data

Chat 262 comes in two flavors:

1. Wire protocol version, where we define our custom wire protocol for exchanging data between a server and a client. Check it out [here](./chat262-wire-protocol/)!
2. gRPC version, where we use Google's gRPC framework for performing remote procedure calls. Check it out [here](./chat262-grpc/)!
