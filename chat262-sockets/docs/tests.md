# Testing

Our implementation includes three parts:

1. Server
2. Client
3. Chat 262 Protocol

All three parts are tested by writing custom client drivers, which make a wide range of requests to the server. Specifically, we test for the following:

- The client sends a valid registration request and the server sends a valid registration response, even when the username-password should not be accepted by the server (a duplicate username, username too short or too long, password too short or too long). Multiple registration requests should work.
- The client sends a valid login request and the server sends a valid login response, even when the credentials are invalid (non-existent user, wrong password). Multiple login requests should work, and should change which user is currently logged in.
- The client sends a valid search accounts request and the server correctly matches existing usernames. The request should be denied if the client is not logged in.
- The client sends a valid send text request and the server sends a valid response, even when the recipient does not exist or the client is not logged in.
- The client sends a valid receive text request and the server sends a valid response, even when the recipient does not exist or the client is not logged in.
- The client sends a valid retrieve correspondents reqest and the server sends a valid response, even when the client is not logged in.
- The client sends a valid delete account request and the server sends a valid response. All semantics of delete should be preserved - chats can no longer be retrieved, texts can no longer be sent, and the username no longer appears when searching accounts. The username cannot be registered with the service again.

The interface is not unit-tested, since it largely performs visual operations, so it's tested by extensive usage.

All tests are located in the [tests/](../tests/) directory.
