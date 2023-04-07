# 
# Unit tests for the gRPC chat service implementation
# NOTE: If any db*.pkl exist, make sure to delete them before running these tests
# 

import grpc
import chat_pb2
import chat_pb2_grpc
import subprocess
import time
import os
import sys
import traceback

# Double registration should fail across restarts
def test_double_registration():
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.RegisterRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = auth_stub.Register(request)
    assert(response.success == True)

    kill_server(server)
    server = start_server()
    auth_stub, chat_stub = connect()

    response = auth_stub.Register(request)
    assert(response.success == False)
    assert(response.message == "\nThe username you requested is already taken.")

    kill_server(server)

# Login for registered users should succeed after a restart
def test_login():
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.RegisterRequest(
        username = 'user2',
        password = 'pass2',
        is_client = True
    )
    response = auth_stub.Register(request)
    assert(response.success == True)

    kill_server(server)
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.LoginRequest(
        username = 'user2',
        password = 'pass2',
        is_client = True
    )
    response = auth_stub.Login(request)
    assert(response.success == True)
    assert(response.message == "\nLogin successful.")
    request = chat_pb2.LoginRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = auth_stub.Login(request)
    assert(response.success == True)
    assert(response.message == "\nLogin successful.")
    request = chat_pb2.LoginRequest(
        username = 'user3',
        password = 'pass3',
        is_client = True
    )
    response = auth_stub.Login(request)
    assert(response.success == False)
    assert(response.message == "\nERROR: Username does not exist in the database. Please try again.")

    kill_server(server)

# List of users should persist across restarts
def test_get_users():
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.Empty()
    user_list = chat_stub.GetUsers(request)
    expected_users = ['user1', 'user2']
    for user in user_list:
        assert(user.username in expected_users)
        expected_users.remove(user.username)
    assert(len(expected_users) == 0)

    kill_server(server)
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.Empty()
    user_list = chat_stub.GetUsers(request)
    expected_users = ['user1', 'user2']
    for user in user_list:
        assert(user.username in expected_users)
        expected_users.remove(user.username)
    assert(len(expected_users) == 0)

    kill_server(server)

# Messages should persist across restarts
def test_messages():
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.SendRequest(
        sender = 'user1',
        receiver = 'user2',
        body = 'Hello from user1 to user2',
        is_client = True
    )
    response = chat_stub.SendMessage(request)
    assert(response.success)
    assert(response.message == "Message successfully added.")

    kill_server(server)
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.User(username = 'user2')
    message_list = chat_stub.ReceiveMessage(request).chats
    assert(len(message_list) == 1)
    assert(message_list[0] == 'From user1: Hello from user1 to user2')

    kill_server(server)

# Deleted accounts should persist across restarts
def test_delete():
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.DeleteRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = auth_stub.DeleteAccount(request)
    assert(response.success)
    assert(response.message == "\nAccount successfully deleted.")

    kill_server(server)
    server = start_server()
    auth_stub, chat_stub = connect()

    request = chat_pb2.Empty()
    user_list = chat_stub.GetUsers(request)
    expected_users = ['user2']
    for user in user_list:
        assert(user.username in expected_users)
        expected_users.remove(user.username)
    assert(len(expected_users) == 0)

    kill_server(server)

def start_server():
    server = subprocess.Popen(["python3","server.py", "2", "", "", "127.0.0.1"])
    time.sleep(1)
    return server

def kill_server(server):
    server.terminate()

def connect():
    channel = grpc.insecure_channel('127.0.0.1:50051')
    auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
    chat_stub = chat_pb2_grpc.ChatServiceStub(channel)
    return auth_stub, chat_stub

if __name__ == "__main__":
    try:
        test_double_registration()
        print('Test 1 (persistent double registration) passed.')
        time.sleep(0.05)
        test_login()
        print('Test 2 (persistent login) passed.')
        time.sleep(0.05)
        test_get_users()
        print('Test 3 (persistent user list) passed.')
        time.sleep(0.05)
        test_messages()
        print('Test 4 (persistent messages) passed.')
        time.sleep(0.05)
        test_delete()
        print('Test 5 (persistent account deletion) passed.')
        time.sleep(0.05)
        print('All tests passed')
    except:
        traceback.print_exc()
        print('Tests failed')

    os.remove('db2.pkl')
