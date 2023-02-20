add_executable(
    test_wrong_message
    test_wrong_message.cc
)
target_link_libraries(
    test_wrong_message
    PRIVATE
    server
    chat262_protocol
)

add_test(NAME "test_wrong_message" COMMAND test_wrong_message)
