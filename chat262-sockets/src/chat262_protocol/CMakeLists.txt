add_library(
    chat262_protocol
    chat262_protocol.cc
)
target_compile_options(
    chat262_protocol
    PUBLIC
    -Wall -Wextra -Werror -Wshadow -O2 -std=c++17
)
target_include_directories(
    chat262_protocol
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include/chat262_protocol/
    ${CMAKE_SOURCE_DIR}/include/common/
)
