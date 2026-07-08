# HTTP Web Server in C++

A multi-threaded HTTP/1.1 web server built from scratch in C++ using POSIX sockets and pthreads. No frameworks, no third-party libraries.

## Architecture (so far)
- `Server` class owns the listening socket.
- Main thread runs an infinite `accept()` loop and never blocks on client I/O.
- Each accepted connection is handed to a new pthread, which reads the raw bytes and closes the connection.

## Folder Structure

```text
http-server/
├── src/
│   ├── main.cpp
│   ├── server.h
│   ├── server.cpp
├── public/
│   └── index.html
├── Makefile
└── README.md
```