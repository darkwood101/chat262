#  Chat 262: The socket version

- [1. Prerequisites](#1-prerequisites)
  - [1.1. Linux](#11-linux)
  - [1.2. macOS](#12-macos)
- [2. Build](#2-build)


## 1. Prerequisites

You will need the following prerequisites to build Chat 262:

- A C++ compiler with support for C++17
- CMake (at least version 3.13)
- make

If you have these already, you can skip this section. Otherwise, see the following sections on how to install these on Linux and macOS.

### 1.1. Linux

We recommend installing from your distribution's package manager. If you have a Debian-based distribution (e.g. Ubuntu), you can run the following

```console
$ sudo apt install build-essential cmake make
```

### 1.2. macOS

You can install g++ and make by running

```console
$ xcode-select --install
```

For CMake, you can download it from [here](https://cmake.org/download/).


## 2. Build

If you haven't cloned this repository yet, do it by running

```console
$ git clone https://github.com/darkwood101/chat262.git
$ cd chat262/
```

To build the C++ version of Chat 262, run the following:
```console
$ cd chat262-sockets/
$ cmake -S . -B build/
$ cmake --build build/
```

This should generate `client` and `server` executables in the `chat262-sockets/` directory.
