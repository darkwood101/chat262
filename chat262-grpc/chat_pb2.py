# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: chat.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\nchat.proto\x12\x0b\x63hatservice\"\x07\n\x05\x45mpty\"\x18\n\x04User\x12\x10\n\x08username\x18\x01 \x01(\t\"2\n\x0cLoginRequest\x12\x10\n\x08username\x18\x01 \x01(\t\x12\x10\n\x08password\x18\x02 \x01(\t\"1\n\rLoginResponse\x12\x0f\n\x07success\x18\x01 \x01(\x08\x12\x0f\n\x07message\x18\x02 \x01(\t\"5\n\x0fRegisterRequest\x12\x10\n\x08username\x18\x01 \x01(\t\x12\x10\n\x08password\x18\x02 \x01(\t\"4\n\x10RegisterResponse\x12\x0f\n\x07success\x18\x01 \x01(\x08\x12\x0f\n\x07message\x18\x02 \x01(\t\"3\n\rDeleteRequest\x12\x10\n\x08username\x18\x01 \x01(\t\x12\x10\n\x08password\x18\x02 \x01(\t\"2\n\x0e\x44\x65leteResponse\x12\x0f\n\x07success\x18\x01 \x01(\x08\x12\x0f\n\x07message\x18\x02 \x01(\t\"=\n\x0bSendRequest\x12\x0e\n\x06sender\x18\x01 \x01(\t\x12\x10\n\x08receiver\x18\x02 \x01(\t\x12\x0c\n\x04\x62ody\x18\x03 \x01(\t\"0\n\x0cSendResponse\x12\x0f\n\x07success\x18\x01 \x01(\x08\x12\x0f\n\x07message\x18\x02 \x01(\t\"+\n\x0b\x43hatMessage\x12\x0e\n\x06sender\x18\x01 \x01(\t\x12\x0c\n\x04\x62ody\x18\x02 \x01(\t2\xe6\x01\n\x0b\x41uthService\x12@\n\x05Login\x12\x19.chatservice.LoginRequest\x1a\x1a.chatservice.LoginResponse\"\x00\x12I\n\x08Register\x12\x1c.chatservice.RegisterRequest\x1a\x1d.chatservice.RegisterResponse\"\x00\x12J\n\rDeleteAccount\x12\x1a.chatservice.DeleteRequest\x1a\x1b.chatservice.DeleteResponse\"\x00\x32\xcd\x01\n\x0b\x43hatService\x12\x44\n\x0bSendMessage\x12\x18.chatservice.SendRequest\x1a\x19.chatservice.SendResponse\"\x00\x12\x41\n\x0eReceiveMessage\x12\x11.chatservice.User\x1a\x18.chatservice.ChatMessage\"\x00\x30\x01\x12\x35\n\x08GetUsers\x12\x12.chatservice.Empty\x1a\x11.chatservice.User\"\x00\x30\x01\x62\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'chat_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _EMPTY._serialized_start=27
  _EMPTY._serialized_end=34
  _USER._serialized_start=36
  _USER._serialized_end=60
  _LOGINREQUEST._serialized_start=62
  _LOGINREQUEST._serialized_end=112
  _LOGINRESPONSE._serialized_start=114
  _LOGINRESPONSE._serialized_end=163
  _REGISTERREQUEST._serialized_start=165
  _REGISTERREQUEST._serialized_end=218
  _REGISTERRESPONSE._serialized_start=220
  _REGISTERRESPONSE._serialized_end=272
  _DELETEREQUEST._serialized_start=274
  _DELETEREQUEST._serialized_end=325
  _DELETERESPONSE._serialized_start=327
  _DELETERESPONSE._serialized_end=377
  _SENDREQUEST._serialized_start=379
  _SENDREQUEST._serialized_end=440
  _SENDRESPONSE._serialized_start=442
  _SENDRESPONSE._serialized_end=490
  _CHATMESSAGE._serialized_start=492
  _CHATMESSAGE._serialized_end=535
  _AUTHSERVICE._serialized_start=538
  _AUTHSERVICE._serialized_end=768
  _CHATSERVICE._serialized_start=771
  _CHATSERVICE._serialized_end=976
# @@protoc_insertion_point(module_scope)
