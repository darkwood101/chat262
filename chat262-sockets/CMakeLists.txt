cmake_minimum_required(VERSION 3.13)

project(
    chat262
    VERSION 1.0
    LANGUAGES CXX
)

# Global CXX options
add_compile_options(-Wall -Wextra -Werror -Wshadow -O2 -std=c++17)
add_link_options(-pthread)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Build chat262_protocol separately, as it's shared between the server and
# the client
add_library(
    chat262_protocol
    OBJECT
    src/chat262_protocol.cc
)

# Build the server and output in the current directory
add_executable(
    server
    src/server.cc
    src/database.cc
    src/logger.cc
)
target_link_libraries(
    server
    chat262_protocol
)
set_target_properties(
    server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# Build the client and output in the current directory
add_executable(client
    src/client.cc
    src/interface.cc
)
target_link_libraries(
    client
    chat262_protocol
)
set_target_properties(
    client
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
