import grpc
from concurrent import futures
import chat_pb2
import chat_pb2_grpc
from collections import defaultdict
import sys
import pickle

# Function to update db.pkl file
def storeData(db):
    with open('db.pkl', 'wb') as dbfile:
        pickle.dump(db, dbfile)
  
# Function to load data from existing pickle file
def loadData():
    try:
        with open('db.pkl', 'rb')  as dbfile:
            db = pickle.load(dbfile)
    except:
        # Database which keeps track of username-password combos and chat messages
        db = {
            "passwords": dict(),
            "messages": defaultdict(list)
        }
    return db

# Load data, which persists across server restarts
db = loadData()

# AuthService class provides Register, Login, and DeleteAccount functions
# These functions are called from the client using the auth_stub
class AuthService(chat_pb2_grpc.AuthServiceServicer):

    # Register a user into the database by associating a specific username and password
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
            response = chat_pb2.RegisterResponse(success=False, message = "\nThe username you requested is already taken.")
        return response

    # Checks credentials and completes login 
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

# AuthService class provides SendMessage, GetUsers, and ReceiveMessage functions
# These functions are called from the client using the chat_stub
class ChatService(chat_pb2_grpc.AuthServiceServicer):

    # Send message to specified receiver with body of message
    def SendMessage(self, request, context):
        curr_users = db['passwords'].keys()
        s = request.sender
        r = request.receiver
        b = request.body

        # Ensure both usernames are valid
        if s in curr_users and r in curr_users:
            db['messages'][r].append(request)
            response = chat_pb2.SendResponse(success = True, message = "Message successfully added.")
        else:
            response = chat_pb2.SendResponse(success = False, message = "\nERROR: either sender or receiver are not in username database. Please try again!\n")
        return response

    # Get all users currently in database
    def GetUsers(self, request, context):
        for u in db['passwords'].keys():
            yield chat_pb2.User(username = u)
    
    # Receives all unread messages for a specific user and removes these messages from the database
    def ReceiveMessage(self, request, context):
        r = request.username
        for i in range(len(db['messages'][r])):
            m = db['messages'][r][0]
            yield chat_pb2.ChatMessage(sender = m.sender, body = m.body)
            # db['messages'][r].pop(0)
            storeData(db)

# Function to start up server
def serve(channel_name):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    chat_pb2_grpc.add_ChatServiceServicer_to_server(ChatService(), server)
    chat_pb2_grpc.add_AuthServiceServicer_to_server(AuthService(), server)

    server.add_insecure_port(channel_name)
    server.start()
    server.wait_for_termination()


def main():
    global am_i_leader
    global my_id
    global ip_addresses
    if len(sys.argv) != 5:
        print("Error...")
        return
    my_id = int(sys.argv[1])
    if my_id < 0 or my_id > 2:
        print("Error")
        return
    am_i_leader = (my_id == 0)
    ip_addresses = sys.argv[2:]
    channel_name = ip_addresses[my_id] + ':50051'
    serve(channel_name)

# Main method that collects server IP address as a command line argument
# If no IP address is provided, localhost is used
if __name__ == "__main__":
    main()
