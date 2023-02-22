import grpc
import chat_pb2
import chat_pb2_grpc
import tkinter as tk
import threading
import sys
import time

# Function to receive a list of messages
def receive_messages():
    global username
    while True:
        message_list = chat_stub.ReceiveMessage(chat_pb2.User(username = username))
        for m in message_list:
            print(f'\n\n Received new message from {m.sender}: {m.body}\n\n>> Enter recipient username: ', end = '')
        time.sleep(1)

# Function to send a single message to another specified user
# Can only be accessed if a user is logged in
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

# Runs the chat "home page", which displays an inbox of new messages since the last login
# and also lists all the current usernames in the database. This function then starts up
# two threads, one to send messages and one to receive messages, which constantly poll until
# there are send requests or incoming messages.
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

    # start up threads for sending + receiving messages
    threading.Thread(target = send_messages).start()
    threading.Thread(target = receive_messages).start()
    return 1

# Runs authorization services including register, login, and delete account.
# This is always the first prompt that is run as a user needs to be logged in
# before they are able to access the "home page".
def run_login():
    global username
    choice = input("\nRegister, Login, or Delete Account?\n\n")

    # register user
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

    # login user
    elif 'l' in choice.lower():
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

    # delete account
    else:
        print("To delete an account, you must log in with the username and password.")
        username = input(">> Username: ")
        password = input(">> Password: ")
        request = chat_pb2.DeleteRequest(username=username, password=password)
        response = auth_stub.DeleteAccount(request)
        print(response.message)
        return 0


# Run client.
# Set channel IP address; if none provided, use local server
n_arg = len(sys.argv)
channel_name = ''
if n_arg == 1:
    channel_name = 'localhost:50051'
elif n_arg == 2:
    channel_name = sys.argv[1] + ':50051'

# Bind to server channel and create auth and chat stubs
channel = grpc.insecure_channel(channel_name)
auth_stub = chat_pb2_grpc.AuthServiceStub(channel)
chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

# Create global username + logged_in variables
username = ''
logged_in = 0 # 0 = not logged in, 1 = logged in

# Run authorization until user is logged in
while logged_in == 0:
    logged_in = run_login()

# Run home page which provides chat services
run_home()
