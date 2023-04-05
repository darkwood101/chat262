import grpc
import chat_pb2
import chat_pb2_grpc
import tkinter as tk
import threading
import sys
import time
import threading

curr_leader = None
ip_addresses = None
curr_auth_stub = None
curr_chat_stub = None

chat_stub_lock = threading.Lock()
num_messages = 0

# Function to receive a list of messages
def receive_messages():
    global username
    global num_messages

    while True:
        chat_stub_lock.acquire()
        message_list = curr_chat_stub.ReceiveMessage(chat_pb2.User(username = username))
        chat_stub_lock.release()

        # Only print messages if there is a change in the number of messages
        if len(message_list) != num_messages:
            num_messages = len(message_list)
            for m in message_list:
                print(m + '\n')
                # print(f'\n\n Message from {m.sender}: {m.body}\n\n>> Enter recipient username: ', end = '')
            time.sleep(1)

# Function to send a single message to another specified user
# Can only be accessed if a user is logged in
def send_messages():
    global username
    global curr_leader
    while True:
        # poll for user input
        receiver = input('\n>> Enter recipient username: ')
        body = input('>> Enter message body: ')

        request = chat_pb2.SendRequest(sender=username, receiver=receiver, body=body)
        
        try:
            chat_stub_lock.acquire()
            response = curr_chat_stub.SendMessage(request, timeout = 1) # timeout if more than 1 second
            if not response.success:
                print(response.message)
            chat_stub_lock.release()
        except:
            # If send message fails / times out, assume that the current leader has crashed and connect to new leader
            curr_leader += 1 
            
            # Check if all servers have failed
            if curr_leader > 2:
                print("All 3 servers have failed.")
                return
            
            # Connect to next leader
            connect(ip_addresses[curr_leader])
        
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
    message_list = curr_chat_stub.ReceiveMessage(chat_pb2.User(username = username))
    empty_inbox = True
    for m in message_list:
        # TODO: change this
        print(f'{m.sender}: {m.body}\n')
        empty_inbox = False
    if empty_inbox:
        print("No new messages to show.")

    user_list = curr_chat_stub.GetUsers(chat_pb2.Empty())

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
        response = curr_auth_stub.Register(request)
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
        response = curr_auth_stub.Login(request)
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
        response = curr_auth_stub.DeleteAccount(request)
        print(response.message)
        return 0

def connect(server_ip):
    global curr_auth_stub
    global curr_chat_stub
    channel = grpc.insecure_channel(server_ip + ':50051')
    curr_auth_stub = chat_pb2_grpc.AuthServiceStub(channel)

    # TODO: add lock here?
    curr_chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

def main():
    if len(sys.argv) != 4:
        print("Error...")
        return
    curr_leader = 0
    ip_addresses = sys.argv[1:]
    # Bind to server channel and create auth and chat stubs
    connect(ip_addresses[curr_leader])

    logged_in = 0 # 0 = not logged in, 1 = logged in

    # Run authorization until user is logged in
    while logged_in == 0:
        logged_in = run_login()

    # Run home page which provides chat services
    run_home()

if __name__=="__main__":
    main()
