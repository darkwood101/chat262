add_executable(
    test_correspondents
    test_correspondents.cc
)
target_link_libraries(
    test_correspondents
    PRIVATE
    client
    server
    chat262_protocol
)

add_test(NAME "test_correspondents" COMMAND test_correspondents)
