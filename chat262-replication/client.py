import grpc
import chat_pb2
import chat_pb2_grpc
import tkinter as tk
import threading
import sys
import time
import threading

class client_params:
    def __init__(self):
        # The ID of the current leader
        self.curr_leader: int = None
        # IP addresses of all servers
        self.ip_addresses: list[int] = None
        # Chat service stub for the current leader
        self.curr_chat_stub: chat_pb2_grpc.ChatServiceStub = None
        # Auth service stub for the current leader
        self.curr_auth_stub: chat_pb2_grpc.AuthServiceStub = None
        # The logged in user's username
        self.username: str = None

g_params = client_params()

chat_stub_lock = threading.Lock()
num_messages = 0

def send_request(stub, rpc_name, *args, **kwargs):
    global g_params
    while True:
        try:
            return getattr(getattr(g_params, stub), rpc_name)(*args, **kwargs, timeout=1)
        except:
            # If send message fails / times out, assume that the current leader
            # has crashed and connect to new leader
            g_params.curr_leader += 1 

            # Check if all servers have failed
            if g_params.curr_leader > 2:
                print("All 3 servers have failed.")
                sys.exit(0)

            # Connect to next leader
            connect(g_params.ip_addresses[g_params.curr_leader])

# Function to receive a list of messages
def receive_messages():
    global num_messages
    global g_params

    while True:
        request = chat_pb2.User(username = g_params.username)
        chat_stub_lock.acquire()
        message_list = send_request("curr_chat_stub", "ReceiveMessage", request).chats
        chat_stub_lock.release()

        # Only print messages if there is a change in the number of messages
        if len(message_list) != num_messages:
            if num_messages != 0:
                print('')
                for m in message_list:
                    print(m + '\n')
                    # print(f'\n\n Message from {m.sender}: {m.body}\n\n>> Enter recipient username: ', end = '')
            print('>> Enter recipient username: ', end = '')
            num_messages = len(message_list)
        time.sleep(1)

# Function to send a single message to another specified user
# Can only be accessed if a user is logged in
def send_messages():
    global g_params
    while True:
        # poll for user input
        receiver = input('\n>> Enter recipient username: ')
        body = input('>> Enter message body: ')

        request = chat_pb2.SendRequest(sender=g_params.username, receiver=receiver, body=body, is_client=True)

        chat_stub_lock.acquire()
        response = send_request("curr_chat_stub", "SendMessage", request)
        if not response.success:
            print(response.message)
        chat_stub_lock.release()

# Runs the chat "home page", which displays an inbox of new messages since the last login
# and also lists all the current usernames in the database. This function then starts up
# two threads, one to send messages and one to receive messages, which constantly poll until
# there are send requests or incoming messages.
def run_home():
    global g_params
    print('\n----------')
    print("\nWELCOME TO THE CHAT HOME PAGE")

    print('\nInbox [all messages to you]:')
    request = chat_pb2.User(username = g_params.username)
    message_list = send_request("curr_chat_stub", "ReceiveMessage", request).chats
    empty_inbox = True
    print('')
    for m in message_list:
        print(m + '\n')
        empty_inbox = False
    if empty_inbox:
        print("No new messages to show.")

    request = chat_pb2.Empty()
    user_list = send_request("curr_chat_stub", "GetUsers", request)

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
    global g_params
    choice = input("\nRegister, Login, or Delete Account?\n\n")

    # register user
    if 'r' in choice.lower():
        print("Register with username and password.")
        username = input(">> Username: ")
        password = input(">> Password: ")
        request = chat_pb2.RegisterRequest(username=username, password=password, is_client=True)
        response = send_request("curr_auth_stub", "Register", request)
        print(response.message)
        if response.success:
            g_params.username = username

        return response.success

    # login user
    elif 'l' in choice.lower():
        print("Login with your username and password.")
        username = input(">> Username: ")
        password = input(">> Password: ")
        request = chat_pb2.LoginRequest(username=username, password=password, is_client=True)
        response = send_request("curr_auth_stub", "Login", request)
        print(response.message)
        if response.success:
            g_params.username = username

        return response.success

    # delete account
    else:
        print("To delete an account, you must log in with the username and password.")
        username = input(">> Username: ")
        password = input(">> Password: ")
        request = chat_pb2.DeleteRequest(username=username, password=password, is_client=True)
        response = send_request("curr_auth_stub", "DeleteAccount", request)
        print(response.message)
        return False

def connect(server_ip):
    global g_params
    channel = grpc.insecure_channel(server_ip + ':50051')
    g_params.curr_auth_stub = chat_pb2_grpc.AuthServiceStub(channel)

    # TODO: add lock here?
    g_params.curr_chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

def main():
    global g_params
    if len(sys.argv) != 4:
        print("Usage: python3 %s"
              " <server 0 IP> <server 1 IP> <server 2 IP>" %
              sys.argv[0])
        return
    # Server 0 is the leader in the beginning
    g_params.curr_leader = 0
    # Get IP addresses from command line
    g_params.ip_addresses = sys.argv[1:]
    # Bind to server channel and create auth and chat stubs
    connect(g_params.ip_addresses[g_params.curr_leader])

    # Run authorization until user is logged in
    logged_in = False
    while not logged_in:
        logged_in = run_login()

    # Run home page which provides chat services
    run_home()

if __name__=="__main__":
    main()
