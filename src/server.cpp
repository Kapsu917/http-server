#include "server.h"
#include "request.h"
#include "router.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

volatile sig_atomic_t Server::running = 1;
static const int IDLE_TIMEOUT_SECONDS = 15;

Server::Server(int port) : port(port), serverFd(-1), epollFd(-1) {}

void Server::handleSigint(int signum) {
    (void)signum;
    running = 0;
}

void Server::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::setupSocket() {
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("socket failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if (listen(serverFd, 128) < 0) {
        perror("listen failed");
        exit(1);
    }

    setNonBlocking(serverFd);
    std::cout << "Server listening on port " << port << std::endl;
}

void Server::acceptNewConnections() {
    while (true) {
        int clientFd = accept(serverFd, nullptr, nullptr);
        if (clientFd < 0) {
            break;
        }

        setNonBlocking(clientFd);

        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = clientFd;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &event);

        clientBuffers[clientFd] = "";
        lastActivity[clientFd] = time(nullptr);
    }
}

void Server::closeConnection(int clientFd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
    close(clientFd);
    clientBuffers.erase(clientFd);
    lastActivity.erase(clientFd);
}

void Server::closeIdleConnections() {
    time_t now = time(nullptr);
    std::vector<int> idleFds;

    for (const auto& entry : lastActivity) {
        if (difftime(now, entry.second) > IDLE_TIMEOUT_SECONDS) {
            idleFds.push_back(entry.first);
        }
    }

    for (int fd : idleFds) {
        closeConnection(fd);
    }
}

void Server::handleClientReadable(int clientFd) {
    char buffer[4096];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        closeConnection(clientFd);
        return;
    }

    clientBuffers[clientFd].append(buffer, bytesRead);
    lastActivity[clientFd] = time(nullptr);

    if (!isRequestComplete(clientBuffers[clientFd])) {
        return;
    }

    HttpRequest request = parseRequest(clientBuffers[clientFd]);
    std::cout << request.method << " " << request.path << std::endl;

    std::string response = routeRequest(request);
    send(clientFd, response.c_str(), response.size(), 0);

    if (shouldKeepAlive(request)) {
        clientBuffers[clientFd].clear(); // ready for the next request on this connection
    } else {
        closeConnection(clientFd);
    }
}

void Server::eventLoop() {
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (running) {
        // 5-second timeout lets us periodically sweep idle keep-alive connections
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, 5000);

        if (numEvents < 0) {
            if (!running) break;
            perror("epoll_wait failed");
            continue;
        }

        for (int i = 0; i < numEvents; i++) {
            int fd = events[i].data.fd;

            if (fd == serverFd) {
                acceptNewConnections();
            } else {
                handleClientReadable(fd);
            }
        }

        closeIdleConnections();
    }

    close(epollFd);
    close(serverFd);
    std::cout << "\nShutting down. Total requests served: "
              << getRequestCount() << std::endl;
}

void Server::start() {
    struct sigaction sa{};
    sa.sa_handler = handleSigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    setupSocket();

    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll_create1 failed");
        exit(1);
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = serverFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &event);

    eventLoop();
}