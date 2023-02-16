import grpc
import chat_pb2
import chat_pb2_grpc
# import login
import tkinter as tk
import threading

channel = grpc.insecure_channel('localhost:50051')
auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

username = ''

def receive_messages():
    global username
    
    while True:
        message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = username))
        for m in message_list:
            print(f'\nNew Message from {m.sender}: {m.body}')

def send_messages():
    global username
    
    while True:
        print('\nMessaging')
        receiver = input('Recipient username: ')
        body = input('Message body: ')
        request = chat_pb2.SendRequest(sender=username, receiver=receiver, body=body)
        chat_stub.SendMessage(request)

def run_home():
    global username
    print("\nHome")

    print('\nInbox:')
    message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = username))
    empty_inbox = True
    for m in message_list:
        print(f'{m.sender}: {m.body}\n')
        empty_inbox = False
    if empty_inbox:
        print("No messages to show.")

    user_list = chat_stub.GetUsers(chat_pb2.Empty())

    users = []
    for u in user_list:
        users.append(u.username)
    print('\nAll Usernames: ', users)

    threading.Thread(target = send_messages).start()
    threading.Thread(target = receive_messages).start()


def run_login():
    global username
    choice = input("\nRegister or Login?\n\n")

    if 'r' in choice.lower():
        print("Register with username and password.")
        username = input("Username: ")
        password = input("Password: ")
        request = chat_pb2.RegisterRequest(username=username, password=password)
        response = auth_stub.Register(request)
        # if response.success:
        print(response.message)

    else:
        print("Login with your username and password.")
        username = input("Username: ")
        password = input("Password: ")
        request = chat_pb2.LoginRequest(username=username, password=password)
        response = auth_stub.Login(request)
        # if response.success:
        print(response.message)

run_login()
run_home()