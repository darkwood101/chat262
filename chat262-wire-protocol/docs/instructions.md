# Setup Instructions

- [1. Prerequisites](#1-prerequisites)
  - [1.1. Operating System](#11-operating-system)
  - [1.2. Build](#12-build)
    - [1.2.1. Linux](#121-linux)
    - [1.2.2. macOS](#122-macos)
  - [1.3. Firewall](#13-firewall)
- [2. Build](#2-build)
- [3. Running Chat 262](#3-running-chat-262)
- [4. Testing](#4-testing)


## 1. Prerequisites

### 1.1. Operating System

Since Chat 262 relies on BSD sockets, it can only work on Unix-like systems. This means that Linux and macOS are supported, but Windows unfortunately is not. If you have Windows, you can try running Chat 262 with [Docker](https://www.docker.com/) or a virtual machine.

### 1.2. Build

To build Chat 262, you will need the following:

- A C++ compiler with support for C++17
- CMake (at least version 3.13)
- make

If you have these already, you can move to the next section.

#### 1.2.1. Linux

On Linux, we recommend installing from your distribution's package manager. If you have a Debian-based distribution (e.g. Ubuntu), you can install GCC, CMake, and make by running
```console
$ sudo apt-get install build-essential cmake
```

#### 1.2.2. macOS

On macOS, you can install GCC and make by running
```console
$ xcode-select --install
```
You can download CMake from [here](https://cmake.org/download/).

### 1.3. Firewall

If you intend to run Chat 262 on one device only (both the server and the client), you can move to the next section.

**IMPORTANT:** Disabling your firewall or setting up port forwarding has important security implications for your system. Do not proceed unless you know what you are doing.

In order for a Chat 262 server to accept connections that are not coming from the same device, the operating system firewall must be configured to allow this. The specifics of configuring this vary from system to system, but in general, you want to allow incoming TCP traffic on port 61079.

If you successfully do this step, your Chat 262 server should be able to accept connections coming from the same network. You will likely not be able to accept connections coming from other networks. To accomplish this, you would need to enable port forwarding. Again, the specifics of doing this vary from system to system, but in general, you need to configure your network router so that it forwards Chat 262 traffic to your device instead of dropping it. This will likely not be possible if you are on a school or work network.

## 2. Build

If you haven't already, clone this repository and navigate to the root directory of the socket version of Chat 262 by running
```console
$ git clone https://github.com/darkwood101/chat262.git
$ cd chat262/chat262-wire-protocol/
```
Alternatively, you can download the repository as a zip file and unzip it.

Chat 262 uses CMake as its build system. This allows for a portable build system that can easily be configured.

To configure CMake, run the following:
```console
$ cmake -S . -B build/
```
This tells CMake to use the current directory (`.`) as the source folder, and to store build configuration files in a `build/` folder.

To compile Chat 262, run the following:
```console
$ cmake --build build/
```
This tells CMake to compile everything, using configuration files stored in `build/`.

If everything goes well, you should see `client.out` and `server.out` executables in the top-level directory.

## 3. Running Chat 262

First, we need to start the Chat 262 server. The command is of the form
```console
$ ./server.out <IP address>
```
This tells the server to listen on the given IP address for incoming Chat 262 traffic. What IP address you should put here depends on your use case:

- If you want to run Chat 262 on your own device only, you can use `127.0.0.1`. This address is reserved for localhost, which means your server will only receive connections coming from the same device.
- If you want to run Chat 262 within one network (and you have added a firewall exception, see [Section 1.3](#13-firewall)), you will need your local IP address. On Linux, you can get this by running `hostname -I` (if you have multiple addresses, you want the one beginning with `192.168`, or `10`, or `172`). On macOS, you can get it by running `ipconfig getifaddr en1` (for an Ethernet connection) or `ipconfig getifaddr en0` (for a wireless connection).
- If you want to run a public Chat 262 server (and you have added a firewall exception AND enabled port forwarding, see [Section 1.3](#13-firewall)), you will need your public IP address. You can do this on both Linux and macOS by running `curl ifconfig.me`. Alternatively, you can simply ask a search engine: "What is my IP address?"

Once you have figured which IP address to use, you can start the server. Here, we'll assume it's `127.0.0.1` for localhost. Once you run the server, you should see something like the following output:
```console
$ ./server.out 127.0.0.1
[T-0x7f3f40184700 | 148us] Listening on 127.0.0.1:61079
```

Now, you can run the client. If you're using localhost, you can run the client from a different terminal window. The command is of the form
```console
$ ./client.out <IP address>
```
This tells the client to connect to the server on the given IP address. You should use the same IP address you used when you started the server. After running it, the client should display the following output:
```

*** Welcome to Chat262 ***

[X] Login
[ ] Register
[ ] Exit

Use Up and Down arrows to navigate. Press ENTER to confirm. Press ESC to go back.
```
The server should let you know that a client connected:
```
[T-0x7f3f40184700 | 3788811us] Accepted connection from 127.0.0.1
```

You got Chat 262 working! You can now interact with the client by following the on-screen instructions.

When you want to shut down the server, you can just hit Ctrl-C to terminate it, or close the terminal window. Note that all state (registered accounts, exchanged texts, etc.) will be lost.

## 4. Testing

Chat 262 comes with some included tests (see [here](tests.md)). If you are interested in running these tests, you should follow these steps.

By default (see [Section 2](#2-build)), CMake will not build the tests. To instruct CMake to build the tests as well, you will first need to reconfigure it:
```console
$ cmake -DTESTING=ON -S . -B build/
```
Like before, this tells CMake to use the current directory (.) as the source folder, and to store build configuration files in a build/ folder. This time, we also pass a flag (`TESTING=ON`), which tells CMake that we want to build tests as well. (`-D` is not a typo, it stands for define.)

To compile the tests, run
```console
$ cmake --build build/
```
This will compile Chat 262 as well if it's not compiled already.

To run the tests, first navigate to the built tests directory using
```console
$ cd build/tests/
```
Then, just run
```console
$ ctest
```
You should see something like the following:
```console
100% tests passed, 0 tests failed out of 9

Total Test time (real) =   1.00 sec
```
If you modify Chat 262, it is a good idea to run these tests to make sure your changes don't break any existing Chat 262 functionality. If you add new functionality to Chat 262, it is a good idea to add new tests as well.
