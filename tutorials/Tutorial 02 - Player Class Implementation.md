# Tutorial 2:  Player Class Implementation

Player.cpp

```c++
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
```

**Objective:**
 Explore the implementation of `Player.cpp` and understand how each method from `Player.h` works under the hood.

##### Constructor:

```c++
#include "Player.h"

Player::Player(const std::string& name, bool isHost)
    : name(name), balance(100), isHost(isHost) {}
```

- Initializes the player's **name** and **host role**.
- Sets the initial **balance** to `$100`.

> This ensures every player starts the game with an equal chance, regardless of role.



##### getName()

```c++
std::string Player::getName() const {
    return name;
}
```

Returns the player's name.

Marked `const` → it guarantees not to modify object state.

##### getBalance()

```c++
int Player::getBalance() const {
    return balance;
}
```

Provides read-only access to the current balance.

Used to validate stake input and show balances during rounds.

##### updateBalance(int amount)

```c++
void Player::updateBalance(int amount) {
    balance += amount;
    if (balance < 0) balance = 0;
}
```

Adjusts balance by the given amount (positive = win, negative = loss).

**Clamp Protection**: Ensures balance never drops below 0.

Example:

- Win: `updateBalance(+30)` → adds $30

- Loss: `updateBalance(-50)` → subtracts $50

- If result < 0, it's reset to 0

  ------

  What does Clamp Protection mean?

> Clamp protection refers to a programming technique that limits a value within a specific range — in this case, to ensure that the player's balance does not fall below zero.

###### In Context of the Game:

When adjusting a player's balance:

- **Positive amount** → the player gains money.
- **Negative amount** → the player loses money.

Now imagine the player has **$10**, and you deduct **$20**:

```c++
balance = balance - 20;  // Results in -10
```

This leads to a **negative balance**, which might not make sense in your game.

#### What Clamp Protection Does

It "clamps" (limits) the minimum value of the balance to **0**.

```c++
if (balance < 0)
    balance = 0;
```

Or, more compactly (in C++17 or using `std::max`):

```c++
balance = std::max(0, balance - amount);
```

So the balance **never drops below 0**, no matter how large the deduction is.

------

**setBalance(int newBalance)**

```c++
void Player::setBalance(int newBalance) {
    balance = (newBalance >= 0) ? newBalance : 0;
}
```

Used to **synchronize** remote player's balance during networking.

Accepts any value ≥ 0.

Rejects negative inputs by forcing balance to 0.

**bool Player::getIsHost()**

This method simply returns whether this player is acting as the **host** (i.e., the server).

In Dice Duelers, the host is the one who:

- Initializes the game and listens for a connection
- Rolls the die each round
- Controls the main game flow

##### Why This Matters

In code logic, especially in `GameManager`, you’ll often see checks like:
```c++
if (localPlayer.getIsHost()) {
    rollDie(); // Host rolls the die
}
```

This distinction is critical because:

- The **host** must make decisions the **client** cannot (e.g., generating random values, maintaining the true state).
- It also helps the `GameManager` determine **who waits** and **who sends first** during a round.

### Summary

| Method               | Purpose                                                      |
| -------------------- | ------------------------------------------------------------ |
| `Player(...)`        | Constructor — initializes name, host/client role, and balance |
| `getName()`          | Returns the player's name                                    |
| `getBalance()`       | Returns the current balance                                  |
| `getIsHost()`        | Returns whether the player is acting as host                 |
| `updateBalance(int)` | Adds or subtracts from balance, ensures it doesn't go below zero |
| `setBalance(int)`    | Sets the balance directly, clamping to zero if the input is negative |