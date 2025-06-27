#include <iostream>
#include <string>
#include "../include/Client.h"

int main() {
    Client client;

    std::string serverIP = "127.0.0.1";
    int port = 5000;

    if (!client.connectToServer(serverIP, port)) {
        std::cerr << "Connection failed.\n";
        return 1;
    }

    std::cout << "Chat started. Type 'exit' to quit.\n";

    while (true) {
        std::string msg;
        std::cout << "You (client): ";
        std::getline(std::cin, msg);

        client.sendMessage(msg);
        if (msg == "exit") break;

        std::string reply = client.receive();
        if (reply == "exit" || reply.empty()) break;

        std::cout << "Server: " << reply << "\n";
    }

    client.close();
    std::cout << "Client disconnected.\n";
    return 0;
}
