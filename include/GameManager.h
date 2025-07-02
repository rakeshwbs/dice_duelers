#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "Player.h"
#include "NetworkManager.h"
#include <random>

class GameManager {
private:
    Player& localPlayer;
    Player& remotePlayer;
    NetworkManager& net;
    int dieResult;
    std::default_random_engine engine;

public:
    GameManager(Player& local, Player& remote, NetworkManager& network);
    void startGame();
    bool playRound();
    void rollDie();
    int getDieResult() const;
};

#endif
