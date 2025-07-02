// Dice.h
#ifndef DICE_H
#define DICE_H

#include <random>

class Dice {
private:
    std::default_random_engine engine;

public:
    Dice();
    int roll();
};

#endif
