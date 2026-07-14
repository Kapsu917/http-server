# HTTP Web Server in C++

A multi-threaded HTTP/1.1 web server built from scratch in C++ using POSIX sockets and pthreads. No frameworks, no third-party libraries.

## Architecture (so far)
- `Server` class owns the listening socket.
- Main thread runs an infinite `accept()` loop and never blocks on client I/O.
- Each accepted connection is handed to a new pthread, which reads the raw bytes and closes the connection.
- `buildResponse()` constructs a valid HTTP/1.1 response string with status line, Content-Type, Content-Length, and Connection: close headers.
- `routeRequest()` is a temporary placeholder router — real routing (static files, /api/status, /api/echo) is added in Milestones 4–5.

## Folder Structure

```text
http-server-cpp/
├── src/
│   ├── main.cpp
│   ├── server.h
│   ├── server.cpp
│   ├── request.h
│   ├── request.cpp
│   ├── response.h
│   └── response.cpp
├── public/
│   └── index.html
├── Makefile
└── README.md
```