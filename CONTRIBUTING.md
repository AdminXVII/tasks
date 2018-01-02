# Contributing

Contributors are welcome, they simply need to respect the intended architecture.
The main developement goal should be oriented toward libraries supporting new languages, as the founder does not intend to support many langugages on its own.

## Architecture
                                                |--- Local task (via Library)
     Client  =========== Server ----------------+--- Local task (via Library)
    JS+HTML   WebSocket    C      unix socket   |--- Local task (via Library)

The local client uses a user-defined webSocket to communicate with the C backend. The backend is in fact a socket multiplexer which takes multiple unix sockets and merges them in the single WebSocket.

### Datagrams (Server - task)
The unix sockets are of type 5 (datagrams) to ensure messages are read as a whole.
The first packet contains the name, the followings contain the messages.
At the socket closure, the last message is declared the end message.

### WebSocket (Client server)
The websocket messages are JSON objects containing:
 + UID
 + End
 + Name or message
 
UID (unsigned byte): Identifier designated by the server to identify the running tasks (UIDs may be reused, as long as the task as finished running). Tasks with the same name can thus coexist and it reduces the JSON size.
Type (boolean): Define if it is the last packet. In the future, may be optional and the client checks for the key
Name or message (utf8 string, max 1000 letters): The first packet contains a name, subsequent packet with the same UID will contain messages

## Improvements

 + Ncurses client for CLI
 + More libraries
