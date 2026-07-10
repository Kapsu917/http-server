#include "server.h"
#include "request.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

Server::Server(int port) : port(port), serverFd(-1) {}

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
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t clientLen = sizeof(clientAddress);

        int clientFd = accept(serverFd, (sockaddr*)&clientAddress, &clientLen);
        if (clientFd < 0) {
            perror("accept failed");
            continue;
        }

        int* clientFdPtr = new int(clientFd);
        pthread_t thread;
        pthread_create(&thread, nullptr, handleClient, clientFdPtr);
        pthread_detach(thread);
    }
}

void* Server::handleClient(void* arg) {
    int clientFd = *(int*)arg;
    delete (int*)arg;

    char buffer[8192] = {0};
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead > 0) {
        std::string rawRequest(buffer, bytesRead);
        HttpRequest request = parseRequest(rawRequest);

        std::cout << "----- Parsed Request -----" << std::endl;
        std::cout << "Method: " << request.method << std::endl;
        std::cout << "Path: " << request.path << std::endl;
        std::cout << "Version: " << request.version << std::endl;

        std::cout << "Headers:" << std::endl;
        for (const auto& header : request.headers) {
            std::cout << "  " << header.first << ": " << header.second << std::endl;
        }

        if (!request.body.empty()) {
            std::cout << "Body: " << request.body << std::endl;
        }
        std::cout << "---------------------------" << std::endl;
    }

    close(clientFd);
    return nullptr;
}

void Server::start() {
    setupSocket();
    acceptLoop();
}