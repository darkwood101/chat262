add_executable(
    test_delete
    test_delete.cc
)
target_link_libraries(
    test_delete
    PRIVATE
    client
    server
    chat262_protocol
)

add_test(NAME "test_delete" COMMAND test_delete)
