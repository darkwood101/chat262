# 
# Unit tests for the gRPC chat service implementation
# NOTE: If db.pkl exists as a file, make sure to delete it before running these tests
# 

import grpc
from concurrent import futures
import chat_pb2
import chat_pb2_grpc
from collections import defaultdict
import sys
import pickle
from server import serve
import threading
import time
import os
from os.path import exists


# Basic registration test
def test_auth1(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user1', password='pass1')
    response = auth_stub.Register(request)
    # print(response.message)
    assert(response.success == True)
    return True

# Attempt to register with username that already exists
def test_auth2(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user1', password='pass1')
    response = auth_stub.Register(request)
    assert(response.success == False)
    return True

# Basic login test
def test_auth3(auth_stub, chat_stub):
    request = chat_pb2.LoginRequest(username='user1', password='pass1')
    response = auth_stub.Login(request)
    assert(response.success == True)
    return True

# Attempt to login with wrong password
def test_auth4(auth_stub, chat_stub):
    request = chat_pb2.LoginRequest(username='user1', password='wrongpassword')
    response = auth_stub.Login(request)
    assert(response.success == False)
    return True

# Test for getting all registered users
def test_getusers(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user2', password='pass2')
    response = auth_stub.Register(request)
    assert(response.success == True)

    request = chat_pb2.RegisterRequest(username='user3', password='pass3')
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
    request = chat_pb2.SendRequest(sender='user1', receiver='user2', body='Hello World')
    response = chat_stub.SendMessage(request)
    assert(response.success == True)

    message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = 'user2'))
    for m in message_list:
        assert(m.sender == 'user1')
        assert(m.body == 'Hello World')
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

    message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = 'user3'))
    c = 1
    for m in message_list:
        assert(m.sender == 'user'+ str(c))
        assert(m.body == str(c))
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
    n_arg = len(sys.argv)
    channel_name = ''
    if n_arg == 1:
        channel_name = 'localhost:50051'
    elif n_arg == 2:
        channel_name = sys.argv[1] + ':50051'

    # start up server with given IP address
    server = threading.Thread(target=serve, args=(channel_name,)).start()
    # sleep to avoid race conditions
    time.sleep(0.5) 

    channel = grpc.insecure_channel(channel_name)
    auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
    chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

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
    

    print('All tests passed.')
    os.remove('db.pkl')
