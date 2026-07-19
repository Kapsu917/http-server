#include "server.h"
#include "request.h"
#include "router.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

volatile sig_atomic_t Server::running = 1;

Server::Server(int port) : port(port), serverFd(-1) {}

void Server::handleSigint(int signum) {
    (void)signum; // unused parameter, silences compiler warning
    running = 0;
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

    if (listen(serverFd, 10) < 0) {
        perror("listen failed");
        exit(1);
    }

    std::cout << "Server listening on port " << port << std::endl;
}

void Server::acceptLoop() {
    while (running) {
        sockaddr_in clientAddress{};
        socklen_t clientLen = sizeof(clientAddress);

        int clientFd = accept(serverFd, (sockaddr*)&clientAddress, &clientLen);
        if (clientFd < 0) {
            // SIGINT interrupts the blocking accept() call, which returns -1.
            // If running is now false, this was a shutdown, not a real error.
            if (!running) break;
            perror("accept failed");
            continue;
        }

        int* clientFdPtr = new int(clientFd);
        pthread_t thread;
        pthread_create(&thread, nullptr, handleClient, clientFdPtr);
        pthread_detach(thread);
    }

    close(serverFd);
    std::cout << "\nShutting down. Total requests served: "
              << getRequestCount() << std::endl;
}

void* Server::handleClient(void* arg) {
    int clientFd = *(int*)arg;
    delete (int*)arg;

    char buffer[8192] = {0};
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead > 0) {
        std::string rawRequest(buffer, bytesRead);
        HttpRequest request = parseRequest(rawRequest);

        std::cout << request.method << " " << request.path << std::endl;

        std::string response = routeRequest(request);
        send(clientFd, response.c_str(), response.size(), 0);
    }

    close(clientFd);
    return nullptr;
}

void Server::start() {
    struct sigaction sa{};
    sa.sa_handler = handleSigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // deliberately NOT SA_RESTART, so accept() returns EINTR
    sigaction(SIGINT, &sa, nullptr);

    setupSocket();
    acceptLoop();
}