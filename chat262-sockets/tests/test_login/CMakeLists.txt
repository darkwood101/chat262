add_executable(
    test_login
    test_login.cc
)
target_link_libraries(
    test_login
    PRIVATE
    test-client
    server
    chat262_protocol
)

add_test(NAME "test_login" COMMAND test_login)
