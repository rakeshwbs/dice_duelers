#include "../include/Player.h"
#include "../include/GameManager.h"
#include "../include/NetworkManager.h"
#include <iostream>

int main() {
    std::string playerName;
    std::cout << "[Server] Enter your name: ";
    std::getline(std::cin, playerName);

    NetworkManager net;
    if (!net.host(5000)) {
        std::cerr << "[Server] Failed to start server.\n";
        return 1;
    }

    std::cout << "[Server] Client connected!\n";

    // Exchange names
    net.sendMessage("NAME|" + playerName);
    std::string nameMsg = net.receiveMessage(); // NAME|<clientName>
    std::string clientName = nameMsg.substr(nameMsg.find('|') + 1);

    std::cout << "[Server] Connected with player: " << clientName << "\n";

    Player local(playerName, true);        // Server is host
    Player remote(clientName, false);      // Client is remote
    GameManager game(local, remote, net);
    game.startGame();

    return 0;
}
