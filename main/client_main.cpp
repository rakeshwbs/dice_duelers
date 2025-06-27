#include <iostream>
#include "../include/Client.h"

int main() {
    Client client;

    std::string serverIP = "127.0.0.1";  // Replace with host IP if using LAN
    int port = 5000;

    if (!client.connectToServer(serverIP, port)) {
        std::cerr << "Connection to server failed.\n";
        return 1;
    }

    std::string message = "Hello from client!";
    if (client.sendMessage(message)) {
        std::cout << "Sent: " << message << "\n";
    } else {
        std::cerr << "Send failed.\n";
    }

    std::string reply = client.receive();
    std::cout << "Server replied: " << reply << "\n";

    client.close();
    return 0;
}
