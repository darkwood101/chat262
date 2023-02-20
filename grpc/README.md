#  Chat 262 Documentation: The gRPC version

- [1. Installation and Configuration](#1-installation)
- [2. Building and Running](#2-build)
- [3. Implementation](#3-implementation)


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


