#include "GameManager.h"
#include "ConsoleColor.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <chrono>
#include <thread>

GameManager::GameManager(Player& local, Player& remote, NetworkManager& network)
    : localPlayer(local), remotePlayer(remote), net(network), engine(std::random_device{}()) {}

void GameManager::startGame() {
    printColorLine("[INFO] Welcome to Dice Duelers over LAN!", ConsoleColor::Aqua);

    while (localPlayer.getBalance() > 0 && remotePlayer.getBalance() > 0) {
        if (!playRound()) break;

        std::cout << "\n";
        printColorLine("Current Balances:", ConsoleColor::LightPurple);
        printColorLine(localPlayer.getName() + ": $" + std::to_string(localPlayer.getBalance()), ConsoleColor::LightGreen);
        printColorLine(remotePlayer.getName() + ": $" + std::to_string(remotePlayer.getBalance()), ConsoleColor::LightBlue);

        std::string cont;
        printColor("[You] Play another round? (y/n): ", ConsoleColor::LightPurple);
        std::cin >> cont;
        std::cin.ignore();
        if (cont != "y" && cont != "Y") {
            net.sendMessage("EXIT|" + localPlayer.getName() + " quit the game.");
            break;
        }
    }

    printColorLine("\n[INFO] Game Over.", ConsoleColor::Aqua);
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
        printColorLine("[DEBUG] Received: " + msg, ConsoleColor::Gray);
        return msg;
    };

    if (localPlayer.getIsHost()) {
        printColor("[You] Enter your guess (1–6): ", ConsoleColor::Yellow);
        std::cin >> localGuess;
        std::cin.ignore();
        net.sendMessage("GUESS|" + std::to_string(localGuess));

        while (true) {
            printColor("[You] Enter your stake: ", ConsoleColor::Yellow);
            if (std::cin >> localStake && localStake > 0 && localStake <= localPlayer.getBalance()) {
                std::cin.ignore();
                break;
            }
            printColorLine("Invalid stake. Try again.", ConsoleColor::LightRed);
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
        printColorLine("[Host] Rolled: " + std::to_string(dieResult), ConsoleColor::LightYellow);
        net.sendMessage("ROLL|" + std::to_string(dieResult));
    } else {
        std::string msg = safeReceive();
        if (checkExit(msg)) return false;
        remoteGuess = std::stoi(msg.substr(msg.find('|') + 1));

        msg = safeReceive();
        if (checkExit(msg)) return false;
        remoteStake = std::stoi(msg.substr(msg.find('|') + 1));

        printColor("[You] Enter your guess (1–6): ", ConsoleColor::Yellow);
        std::cin >> localGuess;
        std::cin.ignore();
        net.sendMessage("GUESS|" + std::to_string(localGuess));

        while (true) {
            printColor("[You] Enter your stake: ", ConsoleColor::Yellow);
            if (std::cin >> localStake && localStake > 0 && localStake <= localPlayer.getBalance()) {
                std::cin.ignore();
                break;
            }
            printColorLine("Invalid stake. Try again.", ConsoleColor::LightRed);
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        net.sendMessage("STAKE|" + std::to_string(localStake));

        msg = safeReceive();
        if (checkExit(msg)) return false;
        dieResult = std::stoi(msg.substr(msg.find('|') + 1));
        printColorLine("[Client] Received roll: " + std::to_string(dieResult), ConsoleColor::LightYellow);
    }

    // Result evaluation
    bool localCorrect = (localGuess == dieResult);
    bool remoteCorrect = (remoteGuess == dieResult);

    if (!localCorrect && !remoteCorrect) {
        localPlayer.updateBalance(-localStake);
        remotePlayer.updateBalance(-remoteStake);
        printColorLine("No one guessed correctly. Both lose stakes.", ConsoleColor::Gray);
    } else if (localCorrect && !remoteCorrect) {
        localPlayer.updateBalance(localStake + remoteStake);
        remotePlayer.updateBalance(-remoteStake);
        printColorLine(localPlayer.getName() + " wins this round!", ConsoleColor::LightGreen);
    } else if (!localCorrect && remoteCorrect) {
        remotePlayer.updateBalance(remoteStake + localStake);
        localPlayer.updateBalance(-localStake);
        printColorLine(remotePlayer.getName() + " wins this round!", ConsoleColor::LightGreen);
    } else if (localGuess == remoteGuess && localStake == remoteStake) {
        printColorLine("Both guessed correctly with same guess/stake. No gain/loss.", ConsoleColor::Gray);
    } else if (localGuess == remoteGuess) {
        localPlayer.updateBalance(localStake);
        remotePlayer.updateBalance(remoteStake);
        printColorLine("Both guessed correctly. Each doubles their own stake.", ConsoleColor::LightAqua);
    } else {
        localPlayer.updateBalance(localStake);
        remotePlayer.updateBalance(remoteStake);
        printColorLine("Both guessed correctly with different numbers. Both gain their own stake.", ConsoleColor::LightAqua);
    }

    // Balance sync
    net.sendMessage("BALANCE|" + std::to_string(localPlayer.getBalance()));
    std::string balanceMsg = safeReceive();
    if (checkExit(balanceMsg)) return false;

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
