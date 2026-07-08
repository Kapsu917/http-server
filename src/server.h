#ifndef SERVER_H
#define SERVER_H

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
};

#endif