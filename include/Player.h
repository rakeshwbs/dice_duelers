#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player {
private:
    std::string name;
    int balance;
    bool isHost;

public:
    Player(const std::string& name, bool isHost);

    std::string getName() const;
    int getBalance() const;
    void updateBalance(int amount);
    void setBalance(int newBalance);
    bool getIsHost() const;
};

#endif
