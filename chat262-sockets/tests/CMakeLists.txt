add_library(
    test-client
    ${CMAKE_SOURCE_DIR}/src/client/client.cc
    ${CMAKE_SOURCE_DIR}/src/client/interface.cc
)
target_compile_options(
    test-client
    PUBLIC
    -Wall -Wextra -Werror -Wshadow -O2 -std=c++17
)
target_link_options(
    test-client
    PUBLIC
    -pthread
)
target_compile_definitions(
    test-client
    PUBLIC
    TESTING
)
target_include_directories(
    test-client
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include/client/
    ${CMAKE_SOURCE_DIR}/include/chat262_protocol/
    ${CMAKE_SOURCE_DIR}/include/common/
)

enable_testing()

add_subdirectory(test_registration)
