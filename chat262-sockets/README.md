# Chat 262: The socket version

## How to build

The build system is standard CMake. If you are unfamiliar with CMake, there are two steps to building a CMake project:

1. Configure CMake
2. Build

To configure CMake, run the following:
```bash
$ cmake -S . -B build/
```
This is telling CMake that the source directory is the current one, and to generate build files in `build/`.

To build, run the following:
```bash
$ cmake --build build/
```
