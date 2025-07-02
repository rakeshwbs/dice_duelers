#include "GameManager.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

GameManager::GameManager(Player& local, Player& remote, NetworkManager& network)
    : localPlayer(local), remotePlayer(remote), net(network), engine(std::random_device{}()) {}

void GameManager::startGame() {
    std::cout << "[INFO] Welcome to Dice Duelers over LAN!\n";

    while (localPlayer.getBalance() > 0 && remotePlayer.getBalance() > 0) {
        if (!playRound()) break;

        std::cout << "\nCurrent Balances:\n";
        std::cout << localPlayer.getName() << ": $" << localPlayer.getBalance() << "\n";
        std::cout << remotePlayer.getName() << ": $" << remotePlayer.getBalance() << "\n";

        std::string cont;
        std::cout << "\nPlay another round? (y/n): ";
        std::cin >> cont;
        if (cont != "y" && cont != "Y") {
            net.sendMessage("EXIT|" + localPlayer.getName() + " quit the game.");
            break;
        }
    }

    std::cout << "\n🎮 Game Over.\n";
    net.closeConnection();
}

bool GameManager::playRound() {
    int localGuess, localStake, remoteGuess, remoteStake;

    auto checkExit = [](const std::string& msg) -> bool {
        return msg.rfind("EXIT|", 0) == 0; // starts with "EXIT|"
    };

    if (localPlayer.getIsHost()) {
        std::cout << "[You] Enter your guess (1–6): ";
        std::cin >> localGuess;
        net.sendMessage("GUESS|" + std::to_string(localGuess));

        std::cout << "[You] Enter your stake: ";
        std::cin >> localStake;
        net.sendMessage("STAKE|" + std::to_string(localStake));

        std::string msg = net.receiveMessage(); // GUESS or EXIT
        if (checkExit(msg)) {
            std::cout << "\n " << msg.substr(5) << "\n";
            return false;
        }
        remoteGuess = std::stoi(msg.substr(msg.find('|') + 1));

        msg = net.receiveMessage(); // STAKE or EXIT
        if (checkExit(msg)) {
            std::cout << "\n " << msg.substr(5) << "\n";
            return false;
        }
        remoteStake = std::stoi(msg.substr(msg.find('|') + 1));

        rollDie();
        std::cout << "[Host] Rolled:  " << dieResult << "\n";
        net.sendMessage("ROLL|" + std::to_string(dieResult));

    } else {
        std::string msg;

        std::cout << "[You] Enter your guess (1–6): ";
        std::cin >> localGuess;
        net.sendMessage("GUESS|" + std::to_string(localGuess));

        std::cout << "[You] Enter your stake: ";
        std::cin >> localStake;
        net.sendMessage("STAKE|" + std::to_string(localStake));

        msg = net.receiveMessage(); // GUESS or EXIT
        if (checkExit(msg)) {
            std::cout << "\n " << msg.substr(5) << "\n";
            return false;
        }
        remoteGuess = std::stoi(msg.substr(msg.find('|') + 1));

        msg = net.receiveMessage(); // STAKE or EXIT
        if (checkExit(msg)) {
            std::cout << "\n " << msg.substr(5) << "\n";
            return false;
        }
        remoteStake = std::stoi(msg.substr(msg.find('|') + 1));

        msg = net.receiveMessage(); // ROLL or EXIT
        if (checkExit(msg)) {
            std::cout << "\n " << msg.substr(5) << "\n";
            return false;
        }
        dieResult = std::stoi(msg.substr(msg.find('|') + 1));
        std::cout << "[Client] Received roll:  " << dieResult << "\n";
    }

    // Game logic
    bool localCorrect = (localGuess == dieResult);
    bool remoteCorrect = (remoteGuess == dieResult);

    if (!localCorrect && !remoteCorrect) {
        localPlayer.updateBalance(-localStake);
        remotePlayer.updateBalance(-remoteStake);
        std::cout << "No one guessed correctly. Both lose stakes.\n";
    } else if (localCorrect && !remoteCorrect) {
        localPlayer.updateBalance(localStake + remoteStake);
        remotePlayer.updateBalance(-remoteStake);
        std::cout << localPlayer.getName() << " wins this round!\n";
    } else if (!localCorrect && remoteCorrect) {
        remotePlayer.updateBalance(remoteStake + localStake);
        localPlayer.updateBalance(-localStake);
        std::cout << remotePlayer.getName() << " wins this round!\n";
    } else if (localGuess == remoteGuess && localStake == remoteStake) {
        std::cout << "Both guessed correctly with same guess/stake. No gain/loss.\n";
    } else if (localGuess == remoteGuess) {
        localPlayer.updateBalance(localStake);
        remotePlayer.updateBalance(remoteStake);
        std::cout << "Both guessed correctly. Each doubles their own stake.\n";
    } else {
        localPlayer.updateBalance(localStake);
        remotePlayer.updateBalance(remoteStake);
        std::cout << "Both guessed correctly with different numbers. Both gain their own stake.\n";
    }

    // Balance sync
    net.sendMessage("BALANCE|" + std::to_string(localPlayer.getBalance()));
    std::string balanceMsg = net.receiveMessage();
    if (checkExit(balanceMsg)) {
        std::cout << "\n " << balanceMsg.substr(5) << "\n";
        return false;
    }
    int remoteBalance = std::stoi(balanceMsg.substr(balanceMsg.find('|') + 1));
    remotePlayer.setBalance(remoteBalance);

    return true;
}

void GameManager::rollDie() {
    std::uniform_int_distribution<int> dist(1, 6);
    dieResult = dist(engine);
}

int GameManager::getDieResult() const {
    return dieResult;
}
