// Dice.cpp
#include "Dice.h"
#include <ctime>

Dice::Dice() {
    engine.seed(static_cast<unsigned int>(std::time(nullptr)));
}

int Dice::roll() {
    std::uniform_int_distribution<int> dist(1, 6);
    return dist(engine);
}
