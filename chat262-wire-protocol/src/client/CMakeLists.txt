add_library(
    client
    client.cc
    interface.cc
)
target_compile_options(
    client
    PUBLIC
    -Wall -Wextra -Werror -Wshadow -O2 -std=c++17
)
target_link_options(
    client
    PUBLIC
    -pthread
)
target_include_directories(
    client
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include/client/
    ${CMAKE_SOURCE_DIR}/include/chat262_protocol/
    ${CMAKE_SOURCE_DIR}/include/common/
)

add_executable(
    client.out
    main.cc
)
target_link_libraries(
    client.out
    PUBLIC
    client
    chat262_protocol
)
set_target_properties(
    client.out
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
