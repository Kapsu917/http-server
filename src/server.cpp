#include "server.h"
#include "request.h"
#include "response.h"
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

// Builds a response for the parsed request.
// Static file serving isn't wired in yet (Milestone 4) so GET requests
// currently get a placeholder body just to prove the response builder works.
static std::string routeRequest(const HttpRequest& request) {
    if (request.method != "GET" && request.method != "POST") {
        return buildResponse(405, "text/plain", "Method Not Allowed");
    }

    if (request.method == "GET") {
        std::string body = "You requested: " + request.path;
        return buildResponse(200, "text/plain", body);
    }

    // POST requests: echo the body back (real /api/echo route comes in Milestone 5)
    return buildResponse(200, "text/plain", request.body);
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
        std::cout << "---------------------------" << std::endl;

        std::string response = routeRequest(request);
        send(clientFd, response.c_str(), response.size(), 0);
    }

    close(clientFd);
    return nullptr;
}

void Server::start() {
    setupSocket();
    acceptLoop();
}