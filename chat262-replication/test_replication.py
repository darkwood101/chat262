# 
# Unit tests for the gRPC chat service implementation
# NOTE: If any db*.pkl exist, make sure to delete them before running these tests
# 

import grpc
import chat_pb2
import chat_pb2_grpc
from client import g_params, send_request, main
import subprocess
import time
import os
import sys
import traceback
from itertools import permutations

# Double registration should be replicated
def test_double_registration():
    perms = list(permutations([0, 1, 2]))
    for perm in perms:
        print("Order of failures:", perm[:2])
        servers = start_servers()
        client_reset()

        assert(g_params.curr_leader == 0)

        request = chat_pb2.RegisterRequest(
            username = 'user1',
            password = 'pass1',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success == True)
        assert(g_params.curr_leader == 0)

        kill_server(servers[perm[0]])
        print("Server " + str(perm[0]) + " failed")

        request = chat_pb2.RegisterRequest(
            username = 'user1',
            password = 'pass1',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success == False)
        assert(response.message == "\nThe username you requested is already taken.")
        assert(g_params.curr_leader == min(perm[1], perm[2]))

        kill_server(servers[perm[1]])
        print("Server " + str(perm[1]) + " failed")

        request = chat_pb2.RegisterRequest(
            username = 'user1',
            password = 'pass1',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success == False)
        assert(response.message == "\nThe username you requested is already taken.")
        assert(g_params.curr_leader == perm[2])

        kill_server(servers[perm[2]])
        os.remove('db0.pkl')
        os.remove('db1.pkl')
        os.remove('db2.pkl')

# Login for registered users should succeed when a server fails
def test_login():
    servers = start_servers()
    client_reset()

    # Register 2 users
    request = chat_pb2.RegisterRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = send_request(
        "curr_auth_stub",
        "Register",
        request
    )
    assert(response.success)
    request = chat_pb2.RegisterRequest(
        username = 'user2',
        password = 'pass2',
        is_client = True
    )
    response = send_request(
        "curr_auth_stub",
        "Register",
        request
    )
    assert(response.success)

    # Kill all servers
    for server in servers:
        kill_server(server)

    def do_one_test():
        request = chat_pb2.LoginRequest(
            username = 'user1',
            password = 'pass1',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Login",
            request
        )
        assert(response.success == True)
        assert(response.message == "\nLogin successful.")
        request = chat_pb2.LoginRequest(
            username = 'user2',
            password = 'pass2',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Login",
            request
        )
        assert(response.success == True)
        assert(response.message == "\nLogin successful.")
        request = chat_pb2.LoginRequest(
            username = 'user3',
            password = 'pass3',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Login",
            request
        )
        assert(response.success == False)
        assert(response.message == "\nERROR: Username does not exist in the database. Please try again.")

    perms = list(permutations([0, 1, 2]))
    for perm in perms:
        print("Order of failures:", perm[:2])
        servers = start_servers()
        client_reset()

        assert(g_params.curr_leader == 0)

        do_one_test()
        assert(g_params.curr_leader == 0)

        kill_server(servers[perm[0]])
        print("Server " + str(perm[0]) + " failed")
        do_one_test()
        assert(g_params.curr_leader == min(perm[1], perm[2]))

        kill_server(servers[perm[1]])
        print("Server " + str(perm[1]) + " failed")
        do_one_test()
        assert(g_params.curr_leader == perm[2])

        kill_server(servers[perm[2]])
    os.remove("db0.pkl")
    os.remove("db1.pkl")
    os.remove("db2.pkl")

# List of users should be replicated across servers
def test_get_users():
    servers = start_servers()
    client_reset()

    # Register 2 users
    request = chat_pb2.RegisterRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = send_request(
        "curr_auth_stub",
        "Register",
        request
    )
    assert(response.success)
    request = chat_pb2.RegisterRequest(
        username = 'user2',
        password = 'pass2',
        is_client = True
    )
    response = send_request(
        "curr_auth_stub",
        "Register",
        request
    )
    assert(response.success)

    # Kill all servers
    for server in servers:
        kill_server(server)

    def do_one_test():
        request = chat_pb2.Empty()
        user_list = send_request(
            "curr_chat_stub",
            "GetUsers",
            request
        ).users
        expected_users = ['user1', 'user2']
        for user in user_list:
            assert(user in expected_users)
            expected_users.remove(user)
        assert(len(expected_users) == 0)

    perms = list(permutations([0, 1, 2]))
    for perm in perms:
        print("Order of failures:", perm[:2])
        servers = start_servers()
        client_reset()

        assert(g_params.curr_leader == 0)

        do_one_test()
        assert(g_params.curr_leader == 0)

        kill_server(servers[perm[0]])
        print("Server " + str(perm[0]) + " failed")
        do_one_test()
        assert(g_params.curr_leader == min(perm[1], perm[2]))

        kill_server(servers[perm[1]])
        print("Server " + str(perm[1]) + " failed")
        do_one_test()
        assert(g_params.curr_leader == perm[2])

        kill_server(servers[perm[2]])
    os.remove("db0.pkl")
    os.remove("db1.pkl")
    os.remove("db2.pkl")

# Messages should persist across restarts
def test_messages():
    # Register 2 users
    def register_users():
        request = chat_pb2.RegisterRequest(
            username = 'user1',
            password = 'pass1',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success)
        request = chat_pb2.RegisterRequest(
            username = 'user2',
            password = 'pass2',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success)

    perms = list(permutations([0, 1, 2]))
    for perm in perms:
        print("Order of failures:", perm[:2])
        servers = start_servers()
        client_reset()

        assert(g_params.curr_leader == 0)

        register_users()

        request = chat_pb2.SendRequest(
            sender = 'user1',
            receiver = 'user2',
            body = 'Hello from user1 to user2 first time',
            is_client = True
        )
        response = send_request(
            "curr_chat_stub",
            "SendMessage",
            request
        )
        assert(response.success)
        assert(response.message == "Message successfully added.")
        assert(g_params.curr_leader == 0)

        kill_server(servers[perm[0]])
        print("Server " + str(perm[0]) + " failed")

        request = chat_pb2.User(username = 'user2')
        message_list = send_request(
            "curr_chat_stub",
            "ReceiveMessage",
            request
        ).chats
        assert(len(message_list) == 1)
        assert(message_list[0] == 'From user1: Hello from user1 to user2 first time')
        assert(g_params.curr_leader == min(perm[1], perm[2]))
        request = chat_pb2.SendRequest(
            sender = 'user1',
            receiver = 'user2',
            body = 'Hello from user1 to user2 second time',
            is_client = True
        )
        response = send_request(
            "curr_chat_stub",
            "SendMessage",
            request
        )
        assert(response.success)
        assert(response.message == "Message successfully added.")
        assert(g_params.curr_leader == min(perm[1], perm[2]))

        kill_server(servers[perm[1]])
        print("Server " + str(perm[1]) + " failed")

        request = chat_pb2.User(username = 'user2')
        message_list = send_request(
            "curr_chat_stub",
            "ReceiveMessage",
            request
        ).chats
        assert(len(message_list) == 2)
        assert(message_list[0] == 'From user1: Hello from user1 to user2 first time')
        assert(message_list[1] == 'From user1: Hello from user1 to user2 second time')
        assert(g_params.curr_leader == perm[2])

        kill_server(servers[perm[2]])

        os.remove("db0.pkl")
        os.remove("db1.pkl")
        os.remove("db2.pkl")

def test_delete():
    # Register 2 users
    def register_users():
        request = chat_pb2.RegisterRequest(
            username = 'user1',
            password = 'pass1',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success)
        request = chat_pb2.RegisterRequest(
            username = 'user2',
            password = 'pass2',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success)
        request = chat_pb2.RegisterRequest(
            username = 'user3',
            password = 'pass3',
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "Register",
            request
        )
        assert(response.success)

    def do_one_test(i):
        request = chat_pb2.DeleteRequest(
            username = 'user' + str(i),
            password = 'pass' + str(i),
            is_client = True
        )
        response = send_request(
            "curr_auth_stub",
            "DeleteAccount",
            request
        )
        assert(response.success)
        assert(response.message == "\nAccount successfully deleted.")

        request = chat_pb2.Empty()
        user_list = send_request(
            "curr_chat_stub",
            "GetUsers",
            request
        ).users
        expected_users = []
        for k in range(i + 1, 4):
            expected_users.append('user' + str(k))
        assert(user_list == expected_users)

    perms = list(permutations([0, 1, 2]))
    for perm in perms:
        print("Order of failures:", perm[:2])
        servers = start_servers()
        client_reset()

        assert(g_params.curr_leader == 0)

        register_users()

        do_one_test(1)
        assert(g_params.curr_leader == 0)

        kill_server(servers[perm[0]])
        print("Server " + str(perm[0]) + " failed")
        do_one_test(2)
        assert(g_params.curr_leader == min(perm[1], perm[2]))

        kill_server(servers[perm[1]])
        print("Server " + str(perm[1]) + " failed")
        do_one_test(3)
        assert(g_params.curr_leader == perm[2])

        kill_server(servers[perm[2]])

        os.remove("db0.pkl")
        os.remove("db1.pkl")
        os.remove("db2.pkl")

def start_servers():
    server_2 = subprocess.Popen(["python3","server.py", "2", "127.0.0.1", "127.0.0.2", "127.0.0.3"])
    time.sleep(0.5)
    server_1 = subprocess.Popen(["python3","server.py", "1", "127.0.0.1", "127.0.0.2", "127.0.0.3"])
    time.sleep(0.5)
    server_0 = subprocess.Popen(["python3","server.py", "0", "127.0.0.1", "127.0.0.2", "127.0.0.3"])
    time.sleep(0.5)
    return [server_0, server_1, server_2]

def kill_server(server):
    server.terminate()

def client_reset():
    main(["client.py", "127.0.0.1", "127.0.0.2", "127.0.0.3"])

if __name__ == "__main__":
    try:
        test_double_registration()
        print('Test 1 (replicated double registration) passed.')
        time.sleep(0.05)
        test_login()
        print('Test 2 (replicated login) passed.')
        time.sleep(0.05)
        test_get_users()
        print('Test 3 (replicated user list) passed.')
        time.sleep(0.05)
        test_messages()
        print('Test 4 (replicated messages) passed.')
        time.sleep(0.05)
        test_delete()
        print('Test 5 (replicated account deletion) passed.')
        time.sleep(0.05)
        print('All tests passed')
    except:
        traceback.print_exc()
        print('Tests failed')


