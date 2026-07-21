# HTTP Web Server in C++

A multi-threaded HTTP/1.1 web server built from scratch in C++ using POSIX sockets and pthreads. No frameworks, no third-party libraries.

## Architecture (so far)
- `Server` class owns the listening socket.
- Main thread runs an infinite `accept()` loop and never blocks on client I/O.
- Each accepted connection is handed to a new pthread, which reads the raw bytes and closes the connection.
- `buildResponse()` constructs a valid HTTP/1.1 response string with status line, Content-Type, Content-Length, and Connection: close headers.
- `routeRequest()` is a temporary placeholder router real routing (static files, /api/status, /api/echo) is added later.

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
│   ├── response.cpp
│   ├── router.h
│   └── router.cpp
├── public/
│   └── index.html
├── Makefile
└── README.md
```
## Benchmarks

Load tested with:

```bash
wrk -t4 -c100 -d15s http://localhost:8080/
```

| Version | Requests/sec | Avg Latency |
|---------|-------------:|------------:|
| Thread-per-connection (before) | 21637.57 | 27.11 ms |
| Epoll event loop (after) | 24835.62 | 4.00 ms |