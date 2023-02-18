add_executable(
    test_logout
    test_logout.cc
)
target_link_libraries(
    test_logout
    PRIVATE
    test-client
    server
    chat262_protocol
)

add_test(NAME "test_logout" COMMAND test_logout)
