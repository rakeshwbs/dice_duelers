# Software Requirements Specification (SRS)

## Project Title: 🎲 Dice Duelers – Console-Based C++ Game  
**Author:** Rakesh Sonea  
**Date:** June 27, 2025  
**Platform:** C++ Console (Cross-platform)  
**Mode:** Two-player (LAN or offline hotseat)

---

## 1. Introduction

### 1.1 Purpose
This document specifies the requirements for a two-player dice-based guessing game called **Dice Duelers**. Players bet virtual money by guessing the outcome of a die roll. The game is designed to test luck and decision-making with strategic betting rules. It runs as a console application using C++.

### 1.2 Scope
The system allows two players to connect either on the same machine or over a **local area network (LAN)**. It features turn-based guessing and betting mechanics, dynamic balance updates, and well-defined winning conditions. The game ends when one or both players are bankrupt or they choose to quit.

### 1.3 Definitions and Abbreviations
| Term | Definition |
|------|------------|
| Die | A standard six-sided dice (1–6) |
| Stake | Amount of money wagered in a round |
| Balance | Total money a player has |
| Pot | Combined amount staked in a round |
| Host | Player running the server |
| Client | Player connecting to the host |
| LAN | Local Area Network |

---

## 2. Overall Description

### 2.1 Product Perspective
The game is a standalone console application built in C++. It supports real-time two-player gameplay over a **local area network (LAN)**. One player acts as the **host**, listening on a port. The other joins by connecting to the host's **local IP address**. The system uses TCP sockets for communication.

All input/output is text-based, and the game uses standard C++ and platform sockets (e.g., Winsock or BSD sockets).

---

### 2.2 Product Features
- Turn-based two-player guessing and betting game
- LAN multiplayer (server-client over TCP)
- Dice roll simulation and visual output
- Bank balance and stake mechanics
- Clear rule-based win/loss outcome logic
- Graceful handling of player exit or bankrupt state

---

### 2.3 User Classes
- **Players**: Two human users with access to separate computers on the same subnet

---

### 2.4 Assumptions and Dependencies
- Both players are connected to the **same subnet** (e.g., WiFi or LAN)
- Firewall allows inbound TCP on the specified port
- The game uses TCP sockets to communicate between host and client
- Players agree to fair turn-taking and do not interfere with socket communications

---

## 3. Functional Requirements

### FR-01 – Game Initialization
- Both players start with a balance of **$100**
- Game allows setup as **host or client**

### FR-02 – LAN Connectivity
- Host listens on a TCP port (e.g., 5000)
- Client enters IP address and port to join the game

### FR-03 – Turn Sequence
- Each round:
    - Players guess a number (1–6)
    - Players enter their stake
    - System rolls the die and displays result

### FR-04 – Rule Evaluation Logic

#### Case 1 – No Player Wins
- Neither player guessed correctly  
➡️ Both lose their stake

#### Case 2 – One Player Wins
- Only one player guessed correctly  
➡️ Winner receives: `2 × their stake + the loser's stake`  
➡️ Loser loses their stake

#### Case 3 – Both Players Guess Same Number (Even if stakes differ)
- Both guessed correctly and picked the same number  
➡️ **Each player receives double their own stake**  
➡️ No stake is transferred or lost

#### Case 4 – Both Guess Same Number, Same Stake
- Both guessed correctly and staked equally  
➡️ **No one gains or loses**. Players keep their stakes

### FR-05 – Balance Updates
- System updates each player’s balance after every round

### FR-06 – Game Exit Conditions
- Game ends when:
    - Either player's balance reaches $0
    - A player chooses to exit

### FR-07 – Multiplayer LAN Support
- Host and client communicate using TCP sockets
- Messages include guesses, stakes, results, and game control
- Host synchronizes state across both players

---

## 4. Non-Functional Requirements

| ID | Requirement |
|----|-------------|
| NFR-01 | System responds to input within 1 second |
| NFR-02 | Game runs in Windows/Linux/macOS terminal |
| NFR-03 | Implemented using modern C++ (11 or above) |
| NFR-04 | Uses TCP sockets (Winsock/BSD) for networking |
| NFR-05 | Object-oriented class design is used |
| NFR-06 | Well-commented and modular code structure |

---

## 5. External Interface Requirements

### 5.1 User Interface
- Console prompts and outputs
- Example:  
    ```
    Enter your guess (1-6):  
    Enter stake amount:  
    Waiting for other player...
    Die rolled: 🎲 5  
    You win this round!
    ```

### 5.2 Hardware Interface
- Two computers connected to the same router/subnet

### 5.3 Software Interface
- C++ standard libraries
- OS sockets (Winsock, BSD, etc.)

---

## 6. Game Flow (Use Case Overview)

### Actors:
- Host
- Client

### Use Case: Play Round
1. Both players input guess and stake
2. Host rolls the die
3. System evaluates results and applies rules
4. Balances are updated
5. Game continues or exits

---

## 7. Future Enhancements

- AI single-player mode
- Leaderboard and persistent stats
- Optional tournament or round mode
- Sound/graphical dice animation in future GUI version

---

## 8. Example Console Output

```
Player 1, guess a number (1–6): 3
Player 1, enter stake: 20
Player 2, guess a number (1–6): 5
Player 2, enter stake: 15
Rolling the die...
🎲 It's a 3!
Player 1 guessed correctly!
Player 1 wins $55!
Player 1 Balance: $135
Player 2 Balance: $85
```

---

## 9. License

**MIT License**  
This project is free to use, modify, and distribute for personal or academic purposes. See LICENSE file for full terms.
