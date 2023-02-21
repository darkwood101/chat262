add_executable(
    test_registration
    test_registration.cc
)
target_link_libraries(
    test_registration
    PRIVATE
    client
    server
    chat262_protocol
)

add_test(NAME "test_registration" COMMAND test_registration)
