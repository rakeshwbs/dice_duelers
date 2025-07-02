#include "Player.h"

Player::Player(const std::string& name, bool isHost)
    : name(name), balance(100), isHost(isHost) {}

std::string Player::getName() const {
    return name;
}

int Player::getBalance() const {
    return balance;
}

void Player::updateBalance(int amount) {
    balance += amount;
    if (balance < 0) balance = 0;
}

void Player::setBalance(int newBalance) {
    balance = (newBalance >= 0) ? newBalance : 0;
}

bool Player::getIsHost() const {
    return isHost;
}
