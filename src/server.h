#ifndef SERVER_H
#define SERVER_H

#include <csignal>

class Server {
public:
    explicit Server(int port);
    void start();

private:
    int port;
    int serverFd;

    void setupSocket();
    void acceptLoop();
    static void* handleClient(void* arg);

    static volatile sig_atomic_t running;
    static void handleSigint(int signum);
};

#endif