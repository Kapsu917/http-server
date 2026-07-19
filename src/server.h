#ifndef SERVER_H
#define SERVER_H

#include <csignal>
#include <unordered_map>
#include <string>

class Server {
public:
    explicit Server(int port);
    void start();

private:
    int port;
    int serverFd;
    int epollFd;

    // Buffers incoming bytes per client fd until a full HTTP request has arrived
    std::unordered_map<int, std::string> clientBuffers;

    void setupSocket();
    void setNonBlocking(int fd);
    void eventLoop();
    void acceptNewConnections();
    void handleClientReadable(int clientFd);
    void closeConnection(int clientFd);

    static volatile sig_atomic_t running;
    static void handleSigint(int signum);
};

#endif