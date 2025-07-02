#include "GameManager.h"
#include <iostream>
#include <sstream>
#include <limits>
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
        std::cin.ignore();  // Clear newline from buffer
        if (cont != "y" && cont != "Y") {
            net.sendMessage("EXIT|" + localPlayer.getName() + " quit the game.");
            break;
        }
    }

    std::cout << "\n[INFO] Game Over.\n";
    net.closeConnection();
}

bool GameManager::playRound() {
    int localGuess = 0, localStake = 0, remoteGuess = 0, remoteStake = 0;

    auto checkExit = [](const std::string& msg) -> bool {
        return msg.rfind("EXIT|", 0) == 0;
    };

    auto safeReceive = [&]() -> std::string {
        std::string msg;
        do {
            msg = net.receiveMessage();
        } while (msg == "[WAIT]");
        std::cout << "[DEBUG] Received: " << msg << "\n";
        return msg;
    };

    if (localPlayer.getIsHost()) {
        std::cout << "[You] Enter your guess (1–6): ";
        std::cin >> localGuess;
        std::cin.ignore();  // Clear newline
        net.sendMessage("GUESS|" + std::to_string(localGuess));

        // Stake input validation
        while (true) {
            std::cout << "[You] Enter your stake: ";
            if (std::cin >> localStake && localStake > 0 && localStake <= localPlayer.getBalance()) {
                std::cin.ignore();
                break;
            }
            std::cout << "Invalid stake. Try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        net.sendMessage("STAKE|" + std::to_string(localStake));

        std::string msg = safeReceive();
        if (checkExit(msg)) return false;
        remoteGuess = std::stoi(msg.substr(msg.find('|') + 1));

        msg = safeReceive();
        if (checkExit(msg)) return false;
        remoteStake = std::stoi(msg.substr(msg.find('|') + 1));

        rollDie();
        std::cout << "[Host] Rolled: " << dieResult << "\n";
        net.sendMessage("ROLL|" + std::to_string(dieResult));
    } else {
        std::string msg = safeReceive();
        if (checkExit(msg)) return false;
        remoteGuess = std::stoi(msg.substr(msg.find('|') + 1));

        msg = safeReceive();
        if (checkExit(msg)) return false;
        remoteStake = std::stoi(msg.substr(msg.find('|') + 1));

        std::cout << "[You] Enter your guess (1–6): ";
        std::cin >> localGuess;
        std::cin.ignore();
        net.sendMessage("GUESS|" + std::to_string(localGuess));

        while (true) {
            std::cout << "[You] Enter your stake: ";
            if (std::cin >> localStake && localStake > 0 && localStake <= localPlayer.getBalance()) {
                std::cin.ignore();
                break;
            }
            std::cout << "Invalid stake. Try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        net.sendMessage("STAKE|" + std::to_string(localStake));

        msg = safeReceive();
        if (checkExit(msg)) return false;
        dieResult = std::stoi(msg.substr(msg.find('|') + 1));
        std::cout << "[Client] Received roll: " << dieResult << "\n";
    }

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

    net.sendMessage("BALANCE|" + std::to_string(localPlayer.getBalance()));
    std::string balanceMsg = safeReceive();
    if (checkExit(balanceMsg)) return false;

    std::cout << "[DEBUG] Raw balance message: " << balanceMsg << "\n";
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
