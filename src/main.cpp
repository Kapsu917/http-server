#include "server.h"

int main() {
    const int PORT = 8080;
    Server server(PORT);
    server.start();
    return 0;
}