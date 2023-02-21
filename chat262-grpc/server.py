import grpc
from concurrent import futures
import chat_pb2
import chat_pb2_grpc
from collections import defaultdict
import sys
import pickle


def storeData(db):
    with open('db.pkl', 'wb') as dbfile:
        pickle.dump(db, dbfile)
  
# delete pkl file every time we make a change here
def loadData():
    try:
        with open('db.pkl', 'rb')  as dbfile:
            db = pickle.load(dbfile)
    except:
        db = {
            # "user_pass": {}, # key: username (str), value: password (str)
            # "login_status": {}, # key: username (str), value: login status (bool)
            # "active_streams": dict(), # whether we can talk to a user
            "passwords": dict(),
            "messages": defaultdict(list)
            # "messages": {} # key: sender_username, value: dict{key: sender, value: list of messages}
        }
    return db

db = loadData()

class AuthService(chat_pb2_grpc.AuthServiceServicer):
    def Register(self, request, context):
        # Check if the username and password are valid
        u = request.username
        p = request.password
        
        if u not in db['passwords']:
            # associate username with password
            db['passwords'][u] = p
            storeData(db)
            response = chat_pb2.RegisterResponse(success=True, message = "\nRegistration successful.")
        else:
            response = chat_pb2.RegisterResponse(success=False, message = "\n The username you requested is already taken.")
        return response

    def Login(self, request, context):
        u = request.username
        p = request.password

        # Check if the username and password are valid
        if u in db['passwords'] and p == db['passwords'][u]:
            response = chat_pb2.LoginResponse(success=True, message = "\nLogin successful.")
        elif u not in db['passwords']:
            response = chat_pb2.LoginResponse(success=False, message = "\nERROR: Username does not exist in the database. Please try again.")
        else:
            response = chat_pb2.LoginResponse(success=False, message = "\nERROR: Invalid password. Please try again.")
        
        return response

    # Remove account from user-password database
    # Any pending messages are still sent
    def DeleteAccount(self, request, context):
        u = request.username
        p = request.password
        if u in db['passwords'] and p == db['passwords'][u]:
            db['passwords'].pop(u)            
            response = chat_pb2.DeleteResponse(success=True, message = "\nAccount successfully deleted.")
        elif u not in db['passwords']:
            response = chat_pb2.DeleteResponse(success=False, message = "\nERROR: Username does not exist in the database.")
        else:
            response = chat_pb2.DeleteResponse(success=False, message = "\nERROR: Invalid password. Please try again.")
        return response

class ChatService(chat_pb2_grpc.AuthServiceServicer):
    def SendMessage(self, request, context):
        # Check if the username and password are valid
        curr_users = db['passwords'].keys()
        s = request.sender
        r = request.receiver
        b = request.body
        print('sender', s)
        print('receiver', r)
        print('body', request.body)

        # make sure both users are registered + sender is logged in
        if s in curr_users and r in curr_users:
            print(f'received message from {s} to {r}')
            db['messages'][r].append(request)
            response = chat_pb2.SendResponse(success = True, message = "Message successfully added.")
        else:
            response = chat_pb2.SendResponse(success = False, message = "\nERROR: either sender or receiver are not in username database. Please try again!\n")
        return response

    def GetUsers(self, request, context):
        for u in db['passwords'].keys():
            yield chat_pb2.User(username = u)
    
    def ReceiveMessage(self, request, context):
        r = request.username
        for i in range(len(db['messages'][r])):
            m = db['messages'][r][0]
            yield chat_pb2.ChatMessage(sender = m.sender, body = m.body)
            db['messages'][r].pop(0)
            storeData(db)

# function to start up server
def serve(channel_name):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    chat_pb2_grpc.add_ChatServiceServicer_to_server(ChatService(), server)
    chat_pb2_grpc.add_AuthServiceServicer_to_server(AuthService(), server)

    server.add_insecure_port(channel_name)
    server.start()
    server.wait_for_termination()

if __name__ == "__main__":
    
    # set server IP address; if none provided, use local server
    n_arg = len(sys.argv)
    channel_name = ''
    if n_arg == 1:
        channel_name = 'localhost:50051'
    elif n_arg == 2:
        channel_name = sys.argv[1] + ':50051'

    serve(channel_name)