# The Dice Class – Random Number Generation in Modern C++

#### `Dice.h` — Class declaration (interface)

```c++
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
```

#### Key Parts:

| Member   | Description                                 |
| -------- | ------------------------------------------- |
| `engine` | A random number engine (internal generator) |
| `Dice()` | Constructor — seeds the engine              |
| `roll()` | Returns a random number from 1 to 6         |

#### `Dice.cpp` — Class implementation (behavior)

```c++
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
```

### **Objective:**

 Understand how the `Dice` class encapsulates random die rolling using C++'s `<random>` library, ensuring fair and unpredictable gameplay.

#### Class Overview

The `Dice` class is small but crucial. It’s responsible for **rolling a six-sided die** — the central mechanic of each round in Dice Duelers.

#### Constructor:

```c++
#include "Dice.h"
#include <ctime>

Dice::Dice() {
    engine.seed(static_cast<unsigned int>(std::time(nullptr)));
}
```

- Seeds the random engine with the **current time** (`std::time(nullptr)`).
- Ensures each game session produces different sequences.

> ❗ This step is critical — without it, you’d get the same numbers every time you play!

#### `roll()` Method:

```c++
int Dice::roll() {
    std::uniform_int_distribution<int> dist(1, 6);
    return dist(engine);
}
```

- Uses a **uniform distribution** to ensure **equal chance** of rolling any number from **1 to 6**.
- Calls the distribution with the internal `engine`.

> This mimics a fair six-sided die with just one line of modern C++ code!

##### Why Use `<random>` Instead of `rand()`?

| Feature        | `rand()`             | `<random>`                                |
| -------------- | -------------------- | ----------------------------------------- |
| Predictability | Easily predictable   | Stronger entropy                          |
| Range control  | Needs manual modding | Controlled via `uniform_int_distribution` |
| Reusability    | Global state         | Object-oriented                           |
| Portability    | Less precise         | Modern C++ standard                       |

#### Example Usage in Game Code

In `GameManager`, the host calls:

```c++
rollDie(); // Internally: dieResult = dice.roll();
```

Which in turn gives a fair result like `5`.

### Summary

| Component  | Role                                      |
| ---------- | ----------------------------------------- |
| `Dice()`   | Seeds the engine using current time       |
| `roll()`   | Returns a random integer from 1 to 6      |
| `engine`   | Internal generator state                  |
| Uses       | `<random>` + `<ctime>` for fairness       |
| Why needed | Fair, unpredictable dice rolls for rounds |