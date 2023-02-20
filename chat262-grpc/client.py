import grpc
import chat_pb2
import chat_pb2_grpc
# import login
import tkinter as tk
import threading
import sys
import time


def receive_messages():
    global username
    while True:
        message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = username))
        for m in message_list:
            print(f'\n\n Received new message from {m.sender}: {m.body}\n\n>> Enter recipient username: ', end = '')
        time.sleep(1)

def send_messages():
    global username
    while True:
        receiver = input('\n>> Enter recipient username: ')
        body = input('>> Enter message body: ')
        request = chat_pb2.SendRequest(sender=username, receiver=receiver, body=body)
        response = chat_stub.SendMessage(request)
        
        if not response.success:
            print(response.message)
        time.sleep(1)

def run_home():
    global username
    print('\n----------')
    print("\nWELCOME TO THE CHAT HOME PAGE")

    print('\nInbox [new messages since last login]:')
    message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = username))
    empty_inbox = True
    for m in message_list:
        print(f'{m.sender}: {m.body}\n')
        empty_inbox = False
    if empty_inbox:
        print("No new messages to show.")

    user_list = chat_stub.GetUsers(chat_pb2.Empty())

    print('\n----------')

    print('\nChat with another user!')

    users = []
    for u in user_list:
        users.append(u.username)
    print('\nAll usernames: ', users)


    threading.Thread(target = send_messages).start()
    threading.Thread(target = receive_messages).start()
    return 1


def run_login():
    global username
    choice = input("\nRegister or Login?\n\n")

    if 'r' in choice.lower():
        print("Register with username and password.")
        username = input(">> Username: ")
        password = input(">> Password: ")
        request = chat_pb2.RegisterRequest(username=username, password=password)
        response = auth_stub.Register(request)
        print(response.message)

        if response.success:
            return 1
        else:
            return 0


    else:
        print("Login with your username and password.")
        username = input(">> Username: ")
        password = input(">> Password: ")
        request = chat_pb2.LoginRequest(username=username, password=password)
        response = auth_stub.Login(request)
        print(response.message)

        if response.success:
            return 1
        else:
            return 0



# set server IP address; if none provided, use local server
n_arg = len(sys.argv)
channel_name = ''
if n_arg == 1:
    channel_name = 'localhost:50051'
elif n_arg == 2:
    channel_name = sys.argv[1] + ':50051'

# 10.250.143.105:50051
channel = grpc.insecure_channel(channel_name)
auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

username = ''
page_status = 0 # 0 = login/register prompt, 1 = home page

while page_status == 0:
    page_status = run_login()

run_home()
