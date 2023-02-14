import grpc
from concurrent import futures
import chat_pb2
import chat_pb2_grpc

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
            "user_pass": {}, # key: username (str), value: password (str)
            "login_status": {}, # key: username (str), value: login status (bool)
            "messages": {} # key: sender_username, value: dict{key: sender, value: list of messages}
        }
    return db

db = loadData()


# class ChatServiceServicer(chat_pb2_grpc.ChatServiceServicer):
#     def MyMethod(self, request, context):
#         response = chat_pb2.MyResponse()
#         response.reply = 'Hello, {}!'.format(request.message)
#         return response

class AuthService(chat_pb2_grpc.AuthServiceServicer):
    def Register(self, request, context):
        # Check if the username and password are valid
        if request.username not in db['user_pass'].keys():
            db['user_pass'][request.username] = request.password
            db['login_status'][request.username] = False # later: automatically log user in
            storeData(db)
            response = chat_pb2.LoginResponse(success=True, message = "Registration successful.")
        else:
            response = chat_pb2.LoginResponse(success=False, message = "Username already taken.")
        return response

    def Login(self, request, context):
        # Check if the username and password are valid
        if request.username in db['user_pass'].keys() and request.password == db['user_pass'][request.username]:
            db['login_status'][request.username] = True
            storeData(db)
            response = chat_pb2.LoginResponse(success=True, message = "Login successful.")
        else:
            response = chat_pb2.LoginResponse(success=False, message = "Invalid username or password.")
        return response

class ChatService(chat_pb2_grpc.AuthServiceServicer):
    def Send(self, request, context):
        # Check if the username and password are valid
        curr_users = db['user_pass'].keys()
        s = request.sender
        r = request.receiver
        print('sender', s)
        print('receiver', r)
        print('body', request.body)
        # make sure both users are registered + sender is logged in
        if s in curr_users and r in curr_users and s in db['login_status'] and db['login_status'][s]:
            # initialize sender chats
            if s not in db['messages']:
                db['messages'][s] = {}

            print('here')
            if r in db['messages'][s].keys():
                # add to existing chats
                db['messages'][s][r].append(request.body)
                response = chat_pb2.LoginResponse(success=True, message = "Message successfully added.")
            else:
                # create new list of chats
                db['messages'][s][r] = [request.body]
                response = chat_pb2.LoginResponse(success=True, message = "Chat successfully started.")
        else:
            response = chat_pb2.LoginResponse(success=False, message = "Error in sending message")
        return response

server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
chat_pb2_grpc.add_ChatServiceServicer_to_server(ChatService(), server)
chat_pb2_grpc.add_AuthServiceServicer_to_server(AuthService(), server)

server.add_insecure_port('[::]:50051')
server.start()
server.wait_for_termination()