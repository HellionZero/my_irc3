IRC server: Implementation

ARCHITETURE:

[  ] define the architeture of the server, including the main components and their interactions.
[  ] implement the main components of the server.
[✅] implement the I/O layer of the server, including the socket handling and event loop.
[  ] definition of classes and data structures for all the components of the IRC server.
[  ] implement the command parser and handler for the IRC protocol.
[  ] implement the user authentication and registration process.
[  ] implement the channel management and message broadcasting.
[  ] implement the error handling and logging mechanisms.

Server:

[✅] I/O layer: Implement the socket handling and event loop for the server.
[  ] Command parser: Implement the command parser and handler for the IRC protocol.
[  ] User authentication: Implement the user authentication and registration process.
[  ] Channel management: Implement the channel management and message broadcasting.
[  ] Error handling: Implement the error handling and logging mechanisms.
[  ] Protocol layer: Implement the protocol layer for the IRC server, including the message formatting and parsing.
[  ] Domain layer: Implement the domain layer for the IRC server, including the user and channel management.

Client:

[  ] Implement the client-side application for the IRC server, including the user interface and command handling.
[  ] Implement the client-side socket handling and event loop for the client application.
[  ] Implement the client-side message formatting and parsing for the IRC protocol.

Parser:

[  ] Implement the command parser class for the IRC server, including the command parsing and validation.
[  ] Implement the command handler class for the IRC server, including the command execution and response generation.
[  ] Implement the error handling and logging mechanisms for the command parser.

Channel:

08/01/2056
[  ] Implement the channel management class for the IRC server, including the channel creation, deletion, and user management.
[  ] Implement the message broadcasting and delivery mechanisms for the channels.
[  ] Implement the error handling and logging mechanisms for the channel management.

Command:

[  ] Implement the command class for the IRC server, including the command definition and execution.
[  ] Implement the error handling and logging mechanisms for the command class.

TESTS:

[✅] connection test: test the server's ability to accept incoming connections from clients.
[✅] message test: test the server's ability to access the server and echo messages back to the clients.
[✅] disconnection test: test the server's ability to handle client disconnections gracefully.