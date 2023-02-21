add_executable(
    test_recv_txt
    test_recv_txt.cc
)
target_link_libraries(
    test_recv_txt
    PRIVATE
    client
    server
    chat262_protocol
)

add_test(NAME "test_recv_txt" COMMAND test_recv_txt)
