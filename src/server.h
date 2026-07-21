#ifndef SERVER_H
#define SERVER_H

#include <csignal>
#include <unordered_map>
#include <string>
#include <ctime>

class Server {
public:
    explicit Server(int port);
    void start();

private:
    int port;
    int serverFd;
    int epollFd;

    std::unordered_map<int, std::string> clientBuffers;
    std::unordered_map<int, time_t> lastActivity;

    void setupSocket();
    void setNonBlocking(int fd);
    void eventLoop();
    void acceptNewConnections();
    void handleClientReadable(int clientFd);
    void closeConnection(int clientFd);
    void closeIdleConnections();

    static volatile sig_atomic_t running;
    static void handleSigint(int signum);
};

#endif