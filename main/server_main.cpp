#include <iostream>
#include <string>
#include "../include/Server.h"

int main() {
    Server server;

    int port = 5000;
    if (!server.initialize(port)) {
        std::cerr << "Server failed to start on port " << port << "\n";
        return 1;
    }

    if (!server.acceptClient()) {
        std::cerr << "Client connection failed.\n";
        return 1;
    }

    std::cout << "Chat started. Type 'exit' to quit.\n";

    while (true) {
        std::string clientMsg = server.receive();
        if (clientMsg == "exit" || clientMsg.empty()) break;

        std::cout << "Client: " << clientMsg << "\n";

        std::string reply;
        std::cout << "You (server): ";
        std::getline(std::cin, reply);

        server.sendMessage(reply);
        if (reply == "exit") break;
    }

    server.close();
    std::cout << "Server disconnected.\n";
    return 0;
}
