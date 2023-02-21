# 
# Unit tests for the gRPC chat service implementation
# NOTE: Before running these unit tests, delete any existing db.pkl file (which stores data on disk)
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

def test_login1(auth_stub, chat_stub):
    request = chat_pb2.RegisterRequest(username='user1', password='pass1')
    response = auth_stub.Register(request)
    print(response.message)
    
    assert(response.success == True)
    
    return True


if __name__ == "__main__":
    # start up server with given IP address
    n_arg = len(sys.argv)
    channel_name = ''
    if n_arg == 1:
        channel_name = 'localhost:50051'
    elif n_arg == 2:
        channel_name = sys.argv[1] + ':50051'

    print('channel_name', channel_name)
    
    server = threading.Thread(target=serve, args=(channel_name,)).start()

    print('here')

    channel = grpc.insecure_channel(channel_name)
    auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
    chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

    if test_login1(auth_stub, chat_stub): print('Test 1 (basic login) passed.')
