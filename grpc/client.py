import grpc
import chat_pb2
import chat_pb2_grpc

channel = grpc.insecure_channel('localhost:50051')
auth_stub = chat_pb2_grpc.AuthServiceStub(channel)

# stub = chat_pb2_grpc.ServiceStub(channel)
# request = chat_pb2.MyRequest()
# request.message = 'world'
# response = stub.MyMethod(request)

print("Register with username and password.")
username = input("Username: ")
password = input("Password: ")

request = chat_pb2.RegisterRequest(username=username, password=password)
response = auth_stub.Register(request)
print(response.message)

# if response.success:
#     print(response.message)
# else:
#     print(response.message)

print("Login with username and password.")
username = input("Username: ")
password = input("Password: ")

request = chat_pb2.LoginRequest(username=username, password=password)
response = auth_stub.Login(request)
# if response.success:
print(response.message)


# LATER: ADD DICTIONARY OF STUBS
channel = grpc.insecure_channel('localhost:50051')
chat_stub = chat_pb2_grpc.ChatServiceStub(channel)

sender = 'cynthia' # WHOEVER IS LOGGED IN
receiver = input("Please type in the username of the person you wish to send a message to: \n")
body = input("Enter your message: \n")

request = chat_pb2.SendRequest(sender=username, receiver=receiver, body = body)
response = chat_stub.Send(request)
print(response.message)

# print(response.reply)