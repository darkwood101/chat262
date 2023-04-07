from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Optional as _Optional

DESCRIPTOR: _descriptor.FileDescriptor

class AllChats(_message.Message):
    __slots__ = ["chats"]
    CHATS_FIELD_NUMBER: _ClassVar[int]
    chats: _containers.RepeatedScalarFieldContainer[str]
    def __init__(self, chats: _Optional[_Iterable[str]] = ...) -> None: ...

class AllUsers(_message.Message):
    __slots__ = ["users"]
    USERS_FIELD_NUMBER: _ClassVar[int]
    users: _containers.RepeatedScalarFieldContainer[str]
    def __init__(self, users: _Optional[_Iterable[str]] = ...) -> None: ...

class DeleteRequest(_message.Message):
    __slots__ = ["counter", "is_client", "password", "username"]
    COUNTER_FIELD_NUMBER: _ClassVar[int]
    IS_CLIENT_FIELD_NUMBER: _ClassVar[int]
    PASSWORD_FIELD_NUMBER: _ClassVar[int]
    USERNAME_FIELD_NUMBER: _ClassVar[int]
    counter: int
    is_client: bool
    password: str
    username: str
    def __init__(self, username: _Optional[str] = ..., password: _Optional[str] = ..., is_client: bool = ..., counter: _Optional[int] = ...) -> None: ...

class DeleteResponse(_message.Message):
    __slots__ = ["message", "success"]
    MESSAGE_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    message: str
    success: bool
    def __init__(self, success: bool = ..., message: _Optional[str] = ...) -> None: ...

class Empty(_message.Message):
    __slots__ = []
    def __init__(self) -> None: ...

class LoginRequest(_message.Message):
    __slots__ = ["is_client", "password", "username"]
    IS_CLIENT_FIELD_NUMBER: _ClassVar[int]
    PASSWORD_FIELD_NUMBER: _ClassVar[int]
    USERNAME_FIELD_NUMBER: _ClassVar[int]
    is_client: bool
    password: str
    username: str
    def __init__(self, username: _Optional[str] = ..., password: _Optional[str] = ..., is_client: bool = ...) -> None: ...

class LoginResponse(_message.Message):
    __slots__ = ["message", "success"]
    MESSAGE_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    message: str
    success: bool
    def __init__(self, success: bool = ..., message: _Optional[str] = ...) -> None: ...

class RegisterRequest(_message.Message):
    __slots__ = ["counter", "is_client", "password", "username"]
    COUNTER_FIELD_NUMBER: _ClassVar[int]
    IS_CLIENT_FIELD_NUMBER: _ClassVar[int]
    PASSWORD_FIELD_NUMBER: _ClassVar[int]
    USERNAME_FIELD_NUMBER: _ClassVar[int]
    counter: int
    is_client: bool
    password: str
    username: str
    def __init__(self, username: _Optional[str] = ..., password: _Optional[str] = ..., is_client: bool = ..., counter: _Optional[int] = ...) -> None: ...

class RegisterResponse(_message.Message):
    __slots__ = ["message", "success"]
    MESSAGE_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    message: str
    success: bool
    def __init__(self, success: bool = ..., message: _Optional[str] = ...) -> None: ...

class SendRequest(_message.Message):
    __slots__ = ["body", "counter", "is_client", "receiver", "sender"]
    BODY_FIELD_NUMBER: _ClassVar[int]
    COUNTER_FIELD_NUMBER: _ClassVar[int]
    IS_CLIENT_FIELD_NUMBER: _ClassVar[int]
    RECEIVER_FIELD_NUMBER: _ClassVar[int]
    SENDER_FIELD_NUMBER: _ClassVar[int]
    body: str
    counter: int
    is_client: bool
    receiver: str
    sender: str
    def __init__(self, sender: _Optional[str] = ..., receiver: _Optional[str] = ..., body: _Optional[str] = ..., is_client: bool = ..., counter: _Optional[int] = ...) -> None: ...

class SendResponse(_message.Message):
    __slots__ = ["message", "success"]
    MESSAGE_FIELD_NUMBER: _ClassVar[int]
    SUCCESS_FIELD_NUMBER: _ClassVar[int]
    message: str
    success: bool
    def __init__(self, success: bool = ..., message: _Optional[str] = ...) -> None: ...

class User(_message.Message):
    __slots__ = ["username"]
    USERNAME_FIELD_NUMBER: _ClassVar[int]
    username: str
    def __init__(self, username: _Optional[str] = ...) -> None: ...
