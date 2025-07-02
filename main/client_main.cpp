#include "../include/Player.h"
#include "../include/GameManager.h"
#include "../include/NetworkManager.h"
#include <iostream>

int main() {
    std::string playerName, serverIP;
    std::cout << "[Client] Enter your name: ";
    std::getline(std::cin, playerName);

    std::cout << "[Client] Enter server IP address: ";
    std::getline(std::cin, serverIP);

    NetworkManager net;
    if (!net.connectToHost(serverIP, 5000)) {
        std::cerr << "[Client] Connection to server failed.\n";
        return 1;
    }

    std::cout << "[Client] Connected to server.\n";

    // Send and receive names
    net.sendMessage("NAME|" + playerName);
    std::string nameMsg = net.receiveMessage(); // NAME|<name>
    std::string hostName = nameMsg.substr(nameMsg.find('|') + 1);

    std::cout << "[Client] Playing with host: " << hostName << "\n";

    Player client(playerName, false);
    Player host(hostName, true);

    GameManager game(client, host, net);
    game.startGame();

    return 0;
}
