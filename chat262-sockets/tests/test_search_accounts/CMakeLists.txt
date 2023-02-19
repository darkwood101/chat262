add_executable(
    test_search_accounts
    test_search_accounts.cc
)
target_link_libraries(
    test_search_accounts
    PRIVATE
    client
    server
    chat262_protocol
)

add_test(NAME "test_search_accounts" COMMAND test_search_accounts)
