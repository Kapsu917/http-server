# HTTP Web Server in C++

A HTTP/1.1 web server built from scratch in C++ using POSIX sockets and Linux epoll. The server implements non-blocking I/O, persistent connections, static file serving, REST endpoints, graceful shutdown, and unit-tested HTTP request parsing without third-party frameworks.

## Architecture 
- **Event loop:** Single-threaded, non-blocking I/O using Linux `epoll` (level-triggered). The listening socket and every client socket are set non-blocking via `fcntl`.
- **Request buffering:** Per-connection read buffers are held in `clientBuffers` until a full HTTP request has arrived (`isRequestComplete()`), since a single `recv()` call is not guaranteed to return a complete request.
- **Request parsing:** `parseRequest()` splits the raw request into headers and body at `\r\n\r\n`, extracts method/path/version from the request line, and parses remaining lines as `Key: Value` headers.
- **Response building:** `buildResponse()` constructs a valid HTTP/1.1 response with status line, Content-Type, Content-Length, and a Connection header (`keep-alive` or `close`, decided per-request).
- **Routing:** `router.cpp` maps requests to static files, `/api/status`, or `/api/echo`. A mutex-protected counter (`std::mutex`) tracks total requests served, safely incremented per request.
- **Keep-alive:** `shouldKeepAlive()` decides whether to hold the connection open, based on the `Connection` header and HTTP version (HTTP/1.1 defaults to keep-alive). Idle connections are swept and closed after 15 seconds via a periodic check in the event loop.
- **Graceful shutdown:** `sigaction()` installs a SIGINT handler *without* `SA_RESTART`, so a blocking call (`epoll_wait`) is interrupted by Ctrl+C rather than silently restarted. On shutdown, the server prints total requests served before exiting.
- **Evolution:** Originally built as one-thread-per-connection using pthreads (tagged `before-epoll` in git history); migrated to epoll after identifying the thread-per-connection model as a scalability bottleneck. See Benchmarks for the measured impact.

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
├── tests/
│   └── test_request.cpp
├── public/
│   └── index.html
├── Makefile
└── README.md
```
## Features

- **Static file server** — serves files from `./public`, with MIME type detection for `.html`, `.css`, `.js`, `.png`, `.jpg`, and a 404 response for missing files.
- **REST API:**
  - `GET /api/status` → JSON with server status, uptime, and total requests served
  - `POST /api/echo` → returns the request body back unchanged
  - Request counter is thread-safe via `std::mutex`
- **HTTP/1.1 keep-alive** — connections stay open across multiple requests by default (or when explicitly requested), with a 15-second idle timeout to reclaim unused connections.
- **Graceful shutdown** — Ctrl+C cleanly stops the event loop and prints total requests served before exiting.
- **Epoll-based concurrency** — single-threaded, non-blocking event loop handling many connections without a thread per client.
- **Unit tests** — dependency-free `assert`-based tests covering parser edge cases: basic GET, POST with body, missing headers, empty body, malformed request line, and partial-request detection.

## How to Build

```bash
make
```

## How to Run

```bash
./server
```
Server listens on port `8080` by default.

## How to Test

**Manual testing:**
```bash
curl http://localhost:8080/
curl http://localhost:8080/nonexistent.html      # expect 404
curl http://localhost:8080/api/status
curl -X POST -d "hello server" http://localhost:8080/api/echo
curl -v -H "Connection: close" http://localhost:8080/   # force connection close
```

**Unit tests:**
```bash
make test
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


## Future Improvements

- Replace the single-threaded epoll loop with a small worker-thread pool, each running its own epoll instance, to use multiple CPU cores
- Add HTTPS/TLS support
- Add request logging to a file instead of stdout
- Support chunked transfer encoding for large response bodies
- Add config file support (port, idle timeout, public directory) instead of hardcoded values
