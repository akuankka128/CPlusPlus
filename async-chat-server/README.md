# async-chat-server
A "chat" server written in C++ for Linux that uses event polling for async I/O.

## Usage
Build and run the server by running `make && ./main`. Clients can then connect to (your machine):42069 over TCP. Any sent messages will be broadcasted to other clients currently connected to the server.
