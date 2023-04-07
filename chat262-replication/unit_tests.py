# 
# Unit tests for the gRPC chat service implementation
# NOTE: If any db*.pkl exist, make sure to delete them before running these tests
# 

import grpc
import chat_pb2
import chat_pb2_grpc
from server import main
import subprocess
import time
import os
import sys

# Basic registration test
def test_auth1(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = auth_stub.Register(request)
    assert(response.success == True)
    return True

# Attempt to register with username that already exists
def test_auth2(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = auth_stub.Register(request)
    assert(response.success == False)
    return True

# Basic login test
def test_auth3(auth_stub, chat_stub):
    request = chat_pb2.LoginRequest(
        username = 'user1',
        password = 'pass1',
        is_client = True
    )
    response = auth_stub.Login(request)
    assert(response.success == True)
    return True

# Attempt to login with wrong password
def test_auth4(auth_stub, chat_stub):
    request = chat_pb2.LoginRequest(
        username = 'user1',
        password = 'wrongpassword',
        is_client = True
    )
    response = auth_stub.Login(request)
    assert(response.success == False)
    return True

# Test for getting all registered users
def test_getusers(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(
        username = 'user2',
        password = 'pass2',
        is_client = True
    )
    response = auth_stub.Register(request)
    assert(response.success == True)

    request = chat_pb2.RegisterRequest(
        username = 'user3',
        password = 'pass3',
        is_client = True
    )
    response = auth_stub.Register(request)
    assert(response.success == True)

    user_list_response = chat_stub.GetUsers(chat_pb2.Empty())
    users = []
    for u in user_list_response:
        users.append(u.username)
    assert(users == ['user1', 'user2', 'user3'])
    return True

# Basic send + receive message test
def test_chat1(auth_stub, chat_stub):
    request = chat_pb2.SendRequest(
        sender = 'user1',
        receiver = 'user2',
        body = 'Hello World',
        is_client = True
    )
    response = chat_stub.SendMessage(request)
    assert(response.success == True)

    message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = 'user2')).chats
    assert(len(message_list) == 1)
    for m in message_list:
        assert(m == 'From user1: Hello World')
    return True

# Attempt to send message to user that doesn't exist
def test_chat2(auth_stub, chat_stub):
    request = chat_pb2.SendRequest(sender='user1', receiver='baduser', body='Oops')
    response = chat_stub.SendMessage(request)
    assert(response.success == False)
    return True

# Sending + receive messages from different users
def test_chat3(auth_stub, chat_stub):
    request = chat_pb2.SendRequest(sender='user1', receiver='user3', body='1')
    response = chat_stub.SendMessage(request)
    assert(response.success == True)

    request = chat_pb2.SendRequest(sender='user2', receiver='user3', body='2')
    response = chat_stub.SendMessage(request)
    assert(response.success == True)

    message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = 'user3')).chats
    c = 1
    for m in message_list:
        assert(m == 'From user' + str(c) + ': ' + str(c))
        c += 1
    return True

# Delete account test
def test_delete(auth_stub, chat_stub):
    request = chat_pb2.DeleteRequest(username='user3', password='pass3')
    response = auth_stub.DeleteAccount(request)
    assert(response.success == True)

    request = chat_pb2.LoginRequest(username='user3', password='pass3')
    response = auth_stub.Login(request)
    assert(response.success == False)

    return True

if __name__ == "__main__":
    # start up server with given IP address
    # sleep to avoid race conditions

    server = subprocess.Popen(["python3","server.py", "2", "", "", "127.0.0.1"])
    time.sleep(1)

    channel = grpc.insecure_channel('127.0.0.1:50051')
    auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
    chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

    try:
        if test_auth1(auth_stub, chat_stub): print('Test 1 (basic register) passed.')
        time.sleep(0.05)
        if test_auth2(auth_stub, chat_stub): print('Test 2 (register attempt with same username) passed.')
        time.sleep(0.05)
        if test_auth3(auth_stub, chat_stub): print('Test 3 (basic login) passed.')
        time.sleep(0.05)
        if test_auth4(auth_stub, chat_stub): print('Test 4 (login attempt with wrong password) passed.')
        time.sleep(0.05)
        if test_getusers(auth_stub, chat_stub): print('Test 5 (getting all users) passed.')
        time.sleep(0.05)
        if test_chat1(auth_stub, chat_stub): print('Test 6 (basic send + receive chat) passed.')
        time.sleep(0.05)
        if test_chat2(auth_stub, chat_stub): print('Test 7 (attempt to send chat to invalid user) passed.')
        time.sleep(0.05)
        if test_chat3(auth_stub, chat_stub): print('Test 7 (receiving chats from multiple users) passed.')
        time.sleep(0.05)
        if test_delete(auth_stub, chat_stub): print('Test 8 (deleting an account) passed.')
        time.sleep(0.05)
        print('All tests passed')
    except Exception as e:
        print(e)
        print('Tests failed')

    os.remove('db2.pkl')
    server.terminate()
