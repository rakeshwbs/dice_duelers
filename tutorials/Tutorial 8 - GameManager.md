# **Tutorial 8: GameManager – Orchestrating the Game Flow**

###  What This Tutorial Will Cover:

The `GameManager` class is the **heart of the game logic**. In this tutorial, we’ll explore:

- The role of `GameManager` in controlling the game loop
- How it interacts with `Player`, `NetworkManager`, and `Dice`
- What happens in `startGame()` (initial setup and loop control)
- The entire flow of `playRound()` — including:
  - Guess/stake collection
  - Network messaging
  - Die rolling (if host)
  - Round evaluation (win/lose logic)
  - Balance syncing
- Why this class doesn't do I/O directly (except for prompts)
- How it acts as a **turn coordinator** and **referee**

> This tutorial ties together **everything you’ve learned so far** — players, networking, dice, and game rules — into one cohesive flow.

## GameManager.h

```c++
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
```

#### Constructor

```c++
GameManager::GameManager(Player& local, Player& remote, NetworkManager& network)
    : localPlayer(local), remotePlayer(remote), net(network), engine(std::random_device{}()) {}
```

- Accepts references to `Player` objects and a `NetworkManager`.
- Initializes a random number generator for rolling dice.

✅ Keeps logic decoupled — `GameManager` **doesn't create** players or network sockets. It just **coordinates** them.

#### `startGame()` – The Main Game Loop

This method controls the overall lifecycle:

##### 🔁 Loop Summary:

```c++
while (localPlayer.getBalance() > 0 && remotePlayer.getBalance() > 0) {
    if (!playRound()) break;

    // Show updated balances
    // Ask player: play again?
}
```

#####  Highlights:

- Loops while both players still have money.
- Calls `playRound()` to process one full turn.
- Asks the local player whether to continue (`y/n`).
- If player quits, it sends an `"EXIT"` message to the remote player.

✅ Clean structure. One place to control replay, end conditions, and network shutdown.

#### `playRound()` – The Core of Each Game Turn

This is where the magic happens:

##### Major Steps:

| Phase             | Description                                    |
| ----------------- | ---------------------------------------------- |
| 1. Input          | Local guess + stake input via `cin`            |
| 2. Messaging      | Send and receive data through `NetworkManager` |
| 3. Die Roll       | Host rolls; result sent to client              |
| 4. Evaluation     | Determine winner and apply balance logic       |
| 5. Sync           | Send and receive updated balances              |
| 6. Result Display | Print round outcome in color                   |

##### Role of Host vs Client

The method branches based on:

```c++
if (localPlayer.getIsHost()) { /* host-side logic */ } else { /* client-side logic */ }
```

- **Host** goes first, sends data, rolls die.
- **Client** waits for host input, then responds, and receives die result.

## GameManager.cpp

```c++
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
```

#### 1️⃣ **Constructor**

```c++
GameManager::GameManager(Player& local, Player& remote, NetworkManager& network)
    : localPlayer(local), remotePlayer(remote), net(network), engine(std::random_device{}()) {}
```

##### What It Does:

- Initializes references to:
  - The **local player**
  - The **remote player**
  - The **network layer**
- Seeds the random number generator (`engine`) using a system-provided random source (`std::random_device`).

✅ This setup makes the class **non-owning**: it **uses external objects** instead of creating them, promoting flexibility and testability.

#### 2️⃣ **startGame()**

```c++
void GameManager::startGame() {
    printColorLine("[INFO] Welcome to Dice Duelers over LAN!", ConsoleColor::Aqua);
```

##### Role:

- Controls the **lifecycle** of the game loop.
- Continues until someone **runs out of money** or **chooses to quit**.

##### Loop Structure:

```c++
while (localPlayer.getBalance() > 0 && remotePlayer.getBalance() > 0) {
    if (!playRound()) break;
```

- The game runs only while both players still have money.
- Each round is managed by `playRound()`.
- If `playRound()` returns `false` (exit/connection problem), loop breaks.

##### Balance Reporting:

```c++
printColorLine("Current Balances:", ConsoleColor::LightPurple);
printColorLine(localPlayer.getName() + ": $" + std::to_string(localPlayer.getBalance()), ConsoleColor::LightGreen);
printColorLine(remotePlayer.getName() + ": $" + std::to_string(remotePlayer.getBalance()), ConsoleColor::LightBlue);
```

- After each round, balances are displayed in distinct colors.

##### Ask to Play Again:

```c++
std::string cont;
printColor("[You] Play another round? (y/n): ", ConsoleColor::LightPurple);
std::cin >> cont;
std::cin.ignore();
if (cont != "y" && cont != "Y") {
    net.sendMessage("EXIT|" + localPlayer.getName() + " quit the game.");
    break;
}
```

- The player chooses whether to continue.
- If not, an `"EXIT"` message is sent to the opponent.

##### Final Cleanup:

```c++
printColorLine("\n[INFO] Game Over.", ConsoleColor::Aqua);
net.closeConnection();
```

- Message shown, connection closed.

#### 3️⃣ **playRound()**

This is the **core gameplay function**.

##### Setup Safe Message Handling:

```c++
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
```

If `EXIT|` is received, the function will safely terminate.

`safeReceive()` waits for a complete message (handles `[WAIT]` cases from TCP fragmentation).

##### Host Flow

```c++
if (localPlayer.getIsHost()) {
```

#### Host does:

1. Prompt for guess and send it
2. Prompt for stake and send it
3. Receive client’s guess and stake
4. Roll the die and send the result

##### Input Handling:

```c++
std::cin >> localGuess;
std::cin.ignore();
// ...
if (std::cin >> localStake && localStake > 0 && localStake <= localPlayer.getBalance()) {
```

- Validates guess and stake
- Ensures stake is within balance

#### Client Flow

```c++
} else {
```

#### Client does:

1. Wait for host’s guess and stake
2. Enter and send own guess/stake
3. Receive die result

✅ Note: client **never rolls the die** — only receives it.

## Round Evaluation

### Match flags:

```c++
cppCopy codebool localCorrect = (localGuess == dieResult);
bool remoteCorrect = (remoteGuess == dieResult);
```

Then several `if...else if` branches:

| Condition                                 | Outcome                       |
| ----------------------------------------- | ----------------------------- |
| None guessed                              | Both lose stake               |
| One guessed                               | That player gains both stakes |
| Both guessed same number and same stake   | No gain/loss                  |
| Both guessed same number, different stake | Each doubles their own stake  |
| Both guessed different correct numbers    | Each doubles own stake        |



All actions are accompanied by:

- **Balance updates**
- **Colored output messages**

------

## 🔁 Balance Synchronization

```c++
cppCopy codenet.sendMessage("BALANCE|" + std::to_string(localPlayer.getBalance()));
std::string balanceMsg = safeReceive();
remotePlayer.setBalance(...);
```

- Sends current balance to remote player.
- Receives remote balance and stores it.

> ✅ Keeps both sides fully synchronized.

------

## 4️⃣ **rollDie()**

```c++
cppCopy codevoid GameManager::rollDie() {
    std::uniform_int_distribution<int> dist(1, 6);
    dieResult = dist(engine);
}
```

- Uses `std::uniform_int_distribution` to roll fair dice
- Stores result in `dieResult` for reuse

------

## 5️⃣ **getDieResult()**

```c++
cppCopy codeint GameManager::getDieResult() const {
    return dieResult;
}
```

- Simple getter method
- Used only for inspection/logging

------

## ✅ Summary of GameManager Methods

| Method             | Role                                    |
| ------------------ | --------------------------------------- |
| `GameManager(...)` | Constructor, sets up references and RNG |
| `startGame()`      | Controls game loop and ending           |
| `playRound()`      | Manages one full game round             |
| `rollDie()`        | Randomly rolls the die (host only)      |
| `getDieResult()`   | Returns last rolled value               |