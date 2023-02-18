add_executable(
    test_send_txt
    test_send_txt.cc
)
target_link_libraries(
    test_send_txt
    PRIVATE
    test-client
    server
    chat262_protocol
)

add_test(NAME "test_send_txt" COMMAND test_send_txt)
