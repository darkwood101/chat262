# 
# Unit tests for the gRPC chat service implementation
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



def test_auth1(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user1', password='pass1')
    response = auth_stub.Register(request)
    # print(response.message)
    assert(response.success == True)
    return True

def test_auth2(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user1', password='pass1')
    response = auth_stub.Register(request)
    assert(response.success == False)
    return True

def test_auth3(auth_stub, chat_stub):
    request = chat_pb2.LoginRequest(username='user1', password='pass1')
    response = auth_stub.Login(request)
    assert(response.success == True)
    return True

def test_auth4(auth_stub, chat_stub):
    request = chat_pb2.LoginRequest(username='user1', password='wrongpassword')
    response = auth_stub.Login(request)
    print(response.message)
    assert(response.success == False)
    return True

def test_getusers(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user2', password='pass2')
    response = auth_stub.Register(request)
    assert(response.success == True)

    user_list_response = chat_stub.GetUsers(chat_pb2.Empty())
    users = []
    for u in user_list_response:
        users.append(u.username)
    assert(users == ['user1', 'user2'])
    return True


# def test_auth4(auth_stub, chat_stub):
#     request = chat_pb2.LoginRequest(username='user1', password='pass1')
#     response = auth_stub.Login(request)
#     assert(response.success == True)
#     return True



if __name__ == "__main__":
    if exists('db.pkl'):
        print('here')
        os.remove('db.pkl')
        time.sleep(1)

    n_arg = len(sys.argv)
    channel_name = ''
    if n_arg == 1:
        channel_name = 'localhost:50051'
    elif n_arg == 2:
        channel_name = sys.argv[1] + ':50051'


    # start up server with given IP address
    server = threading.Thread(target=serve, args=(channel_name,)).start()

    time.sleep(0.1)

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
    

    print('All tests passed.')
