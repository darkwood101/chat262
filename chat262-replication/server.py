import grpc
from concurrent import futures
import chat_pb2
import chat_pb2_grpc
from collections import defaultdict
import sys
import pickle

class server_params:
    def __init__(self):
        # True iff this server is the leader
        self.am_i_leader: bool = None
        # The ID of this server (in range [0, 3])
        self.my_id: int = None
        # IP addresses of all servers
        self.ip_addresses: list[int] = None
        # Chat service stubs for all servers with id > my_id
        self.chat_stubs: list[chat_pb2_grpc.ChatServiceStub] = None
        # Auth service stubs for all servers with id > my_id
        self.auth_stubs: list[chat_pb2_grpc.AuthServiceStub] = None
        # Name for this server's database file
        self.db_name: str = None
        # Database for this server
        self.db: dict = None

g_params = server_params()

# Dump the database from g_params.db into a database file
def storeData():
    global g_params
    with open(g_params.db_name, 'wb') as dbfile:
        pickle.dump(g_params.db, dbfile)

# Loads data from an existing database into g_params.db, or creates a new one
# if it doesn't exist
def loadData():
    global g_params
    try:
        with open(g_params.db_name, 'rb')  as dbfile:
            g_params.db = pickle.load(dbfile)
    except:
        # Database which keeps track of username-password combos and chat
        # messages
        g_params.db = {
            "passwords": dict(),
            "messages": defaultdict(list)
        }

# AuthService class provides Register, Login, and DeleteAccount functions
# These functions are called from the client using the auth_stub
class AuthService(chat_pb2_grpc.AuthServiceServicer):

    # Register a user into the database by associating a specific username and
    # password
    def Register(self, request, context):
        global g_params

        # If this server receives a client request, it must be the leader
        if request.is_client and not g_params.am_i_leader:
            print("Server %d: I am now the leader" % (g_params.my_id))
            g_params.am_i_leader = True

        # If this server is the leader, try to replicate
        if g_params.am_i_leader:
            print("Server %d: The leader received a request" % (g_params.my_id))
            for i in range(len(g_params.auth_stubs)):
                # If an auth stub does not exist, we already noted before the
                # server is down
                if g_params.auth_stubs[i] is None:
                    print("Server %d: server %d is down, not replicating to it" %
                          (g_params.my_id, g_params.my_id + i + 1))
                    continue
                try:
                    # Send a replication request, but label it as not coming
                    # from a client
                    request.is_client = False
                    g_params.auth_stubs[i].Register(request)
                    request.is_client = True
                    print("Server %d: Successfully replicated to server %d" %
                          (g_params.my_id, g_params.my_id + i + 1))
                except:
                    # Remove the auth stub so that we don't try again
                    print("Server %d: Server %d is down, failed to replicate" %
                          (g_params.my_id, g_params.my_id + i + 1))
                    g_params.auth_stubs[i] = None
        else:
            print("Server %d: Replicating a request" %
                  (g_params.my_id))

        # Check if the username and password are valid
        u = request.username
        p = request.password
        
        if u not in g_params.db['passwords']:
            # associate username with password
            g_params.db['passwords'][u] = p
            storeData()
            response = chat_pb2.RegisterResponse(
                success = True,
                message = "\nRegistration successful."
            )
        else:
            response = chat_pb2.RegisterResponse(
                success = False,
                message = "\nThe username you requested is already taken."
            )
        return response

    # Checks credentials and completes login 
    def Login(self, request, context):
        global g_params

        u = request.username
        p = request.password

        # Check if the username and password are valid
        if u in g_params.db['passwords'] and p == g_params.db['passwords'][u]:
            response = chat_pb2.LoginResponse(
                success = True,
                message = "\nLogin successful."
            )
        elif u not in g_params.db['passwords']:
            response = chat_pb2.LoginResponse(
                success = False,
                message = "\nERROR: Username does not exist in the database. Please try again."
            )
        else:
            response = chat_pb2.LoginResponse(
                success = False,
                message = "\nERROR: Invalid password. Please try again."
            )
        return response

    # Remove account from user-password database
    # Any pending messages are still sent
    def DeleteAccount(self, request, context):
        global g_params

        # If this server receives a client request, it must be the leader
        if request.is_client and not g_params.am_i_leader:
            print("Server %d: I am now the leader" % (g_params.my_id))
            g_params.am_i_leader = True

        # If this server is the leader, try to replicate
        if g_params.am_i_leader:
            print("Server %d: The leader received a request" % (g_params.my_id))
            for i in range(len(g_params.auth_stubs)):
                # If an auth stub does not exist, we already noted before the
                # server is down
                if g_params.auth_stubs[i] is None:
                    print("Server %d: server %d is down, not replicating to it" %
                          (g_params.my_id, g_params.my_id + i + 1))
                    continue
                try:
                    # Send a replication request, but label it as not coming
                    # from a client
                    request.is_client = False
                    g_params.auth_stubs[i].DeleteAccount(request)
                    request.is_client = True
                    print("Server %d: Successfully replicated to server %d" %
                          (g_params.my_id, g_params.my_id + i + 1))
                except:
                    # Remove the auth stub so that we don't try again
                    print("Server %d: Server %d is down, failed to replicate" %
                          (g_params.my_id, g_params.my_id + i + 1))
                    g_params.auth_stubs[i] = None
        else:
            print("Server %d: Replicating a request" %
                  (g_params.my_id))

        u = request.username
        p = request.password

        if u in g_params.db['passwords'] and p == g_params.db['passwords'][u]:
            g_params.db['passwords'].pop(u)
            storeData()
            response = chat_pb2.DeleteResponse(
                success = True,
                message = "\nAccount successfully deleted."
            )
        elif u not in g_params.db['passwords']:
            response = chat_pb2.DeleteResponse(
                success = False,
                message = "\nERROR: Username does not exist in the database."
            )
        else:
            response = chat_pb2.DeleteResponse(
                success = False,
                message = "\nERROR: Invalid password. Please try again."
            )
        return response

# ChatService class provides SendMessage, GetUsers, and ReceiveMessage functions
# These functions are called from the client using the chat_stub
class ChatService(chat_pb2_grpc.AuthServiceServicer):

    # Send message to specified receiver with body of message
    def SendMessage(self, request, context):
        global g_params

        # If this server receives a client request, it must be the leader
        if request.is_client and not g_params.am_i_leader:
            print("Server %d: I am now the leader" % (g_params.my_id))
            g_params.am_i_leader = True

        # If this server is the leader, try to replicate
        if g_params.am_i_leader:
            print("Server %d: The leader received a request" % (g_params.my_id))
            for i in range(len(g_params.chat_stubs)):
                # If an chat stub does not exist, we already noted before the
                # server is down
                if g_params.chat_stubs[i] is None:
                    print("Server %d: server %d is down, not replicating to it" %
                          (g_params.my_id, g_params.my_id + i + 1))
                    continue
                try:
                    # Send a replication request, but label it as not coming
                    # from a client
                    request.is_client = False
                    g_params.chat_stubs[i].SendMessage(request)
                    request.is_client = True
                    print("Server %d: Successfully replicated to server %d" %
                          (g_params.my_id, g_params.my_id + i + 1))
                except:
                    # Remove the chat stub so that we don't try again
                    print("Server %d: Server %d is down, failed to replicate" %
                          (g_params.my_id, g_params.my_id + i + 1))
                    g_params.chat_stubs[i] = None
        else:
            print("Server %d: Replicating a request" %
                  (g_params.my_id))

        curr_users = g_params.db['passwords'].keys()
        s = request.sender
        r = request.receiver
        b = request.body

        # Ensure both usernames are valid
        if s in curr_users and r in curr_users:
            g_params.db['messages'][r].append(request)
            storeData()
            response = chat_pb2.SendResponse(
                success = True,
                message = "Message successfully added."
            )
        else:
            response = chat_pb2.SendResponse(
                success = False,
                message = "\nERROR: either sender or receiver are not in"
                          " username database. Please try again!\n"
            )
        return response

    # Get all users currently in database
    def GetUsers(self, request, context):
        for u in g_params.db['passwords'].keys():
            yield chat_pb2.User(username = u)

    # Receives all unread messages for a specific user and removes these
    # messages from the database
    def ReceiveMessage(self, request, context):
        r = request.username
        chats = []
        for m in g_params.db['messages'][r]:
            chats.append('From ' + m.sender + ': ' + m.body)
        return chat_pb2.AllChats(chats=chats)

# Function to start up server
def serve(channel_name):
    server = grpc.server(futures.ThreadPoolExecutor(max_workers = 10))
    chat_pb2_grpc.add_ChatServiceServicer_to_server(ChatService(), server)
    chat_pb2_grpc.add_AuthServiceServicer_to_server(AuthService(), server)

    server.add_insecure_port(channel_name)
    server.start()
    server.wait_for_termination()

def main():
    global g_params
    if len(sys.argv) != 5:
        print("Usage: python3 %s"
              " <server ID> <server 0 IP> <server 1 IP> <server 2 IP>" %
              sys.argv[0])
        return
    # Get server ID from command line
    g_params.my_id = int(sys.argv[1])
    if g_params.my_id < 0 or g_params.my_id > 2:
        print("Server ID has to be in \{0, 1, 2\}")
        return
    # In the beginning, only server 0 is the leader
    g_params.am_i_leader = (g_params.my_id == 0)
    # Get IP addresses from command line
    g_params.ip_addresses = sys.argv[2:]
    # Initialize auth and chat stubs for replicas
    # Only connect to servers with id > my_id
    g_params.auth_stubs = []
    g_params.chat_stubs = []
    for i in range(g_params.my_id + 1, 3):
        channel = grpc.insecure_channel(g_params.ip_addresses[i] + ':50051')
        g_params.auth_stubs.append(chat_pb2_grpc.AuthServiceStub(channel))
        g_params.chat_stubs.append(chat_pb2_grpc.ChatServiceStub(channel))
    # Load data, which persists across server restarts
    g_params.db_name = "db" + str(g_params.my_id) + ".pkl"
    loadData()
    # Start the server on the given IP
    serve(g_params.ip_addresses[g_params.my_id] + ':50051')

# Main method that collects server IP address as a command line argument
# If no IP address is provided, localhost is used
if __name__ == "__main__":
    main()
