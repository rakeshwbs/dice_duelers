#include <iostream>
#include "../include/Server.h"

int main() {
    Server server;

    int port = 5000;
    if (!server.initialize(port)) {
        std::cerr << "Server failed to start on port " << port << "\n";
        return 1;
    }

    if (!server.acceptClient()) {
        std::cerr << "Failed to accept client connection.\n";
        return 1;
    }

    std::cout << "Waiting for client message...\n";
    std::string clientMsg = server.receive();
    std::cout << "Client says: " << clientMsg << "\n";

    server.sendMessage("Hello from server!");
    server.close();

    return 0;
}
