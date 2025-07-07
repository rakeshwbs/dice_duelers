# Tutorial 11 - Server Main

```c++
#include "../include/Player.h"
#include "../include/GameManager.h"
#include "../include/NetworkManager.h"
#include <iostream>

int main() {
    std::string playerName;
    std::cout << "[Server] Enter your name: ";
    std::getline(std::cin, playerName);

    NetworkManager net;
    if (!net.host(5000)) {
        std::cerr << "[Server] Failed to start server.\n";
        return 1;
    }

    std::cout << "[Server] Client connected!\n";

    // Exchange names
    net.sendMessage("NAME|" + playerName);
    std::string nameMsg = net.receiveMessage(); // NAME|<clientName>
    std::string clientName = nameMsg.substr(nameMsg.find('|') + 1);

    std::cout << "[Server] Connected with player: " << clientName << "\n";

    Player local(playerName, true);        // Server is host
    Player remote(clientName, false);      // Client is remote
    GameManager game(local, remote, net);
    game.startGame();

    return 0;
}

```

### Prompting the Host Player for Their Name

```c++
std::string playerName;
std::cout << "[Server] Enter your name: ";
std::getline(std::cin, playerName);
```

------

#### ✅ **Purpose**

This snippet collects the **local host player's name** from user input. It allows the server (host) to personalize the session and share the name with the client for display in the game.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `std::string playerName;`

- Declares a variable to store the host player’s name.
- This will later be passed to the `Player` class constructor and sent to the client during name exchange.

------

##### 🧩 `std::cout << "[Server] Enter your name: ";`

- Displays a prompt in the terminal so the host knows they need to type in their name.
- The `[Server]` prefix clarifies the role of this player in the multiplayer setup.

------

##### 🧩 `std::getline(std::cin, playerName);`

- Reads an entire line of input from the user — **including spaces**, if any.
- Unlike `std::cin >> playerName`, this ensures that names like `"John Doe"` or `"The Host"` are captured in full.

------

#### 🔐 Why It’s Important

- The host’s name is sent to the client (and used in `GameManager`) for all game messages.

- Properly storing and sharing player names helps personalize messages like:

  ```c++
  Rakesh wins this round!
  ```

### Starting the Server and Waiting for Client Connection

```c++
NetworkManager net;
if (!net.host(5000)) {
    std::cerr << "[Server] Failed to start server.\n";
    return 1;
}

std::cout << "[Server] Client connected!\n";
```

------

#### ✅ **Purpose**

This snippet sets up the server to listen on a specific TCP port (`5000`), waits for a client to connect, and confirms when the connection has been established.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `NetworkManager net;`

- Creates an instance of the `NetworkManager` class.
- This object wraps all the low-level Winsock logic for hosting, sending, and receiving messages.
- Used throughout the game session for network communication between host and client.

------

##### 🧩 `if (!net.host(5000)) { ... }`

- Calls the `host()` method of `NetworkManager`, attempting to:
  1. Initialize Winsock
  2. Create a listening socket
  3. Bind it to port `5000`
  4. Accept a single incoming client connection
- If anything in that process fails:
  - The method returns `false`
  - The error message is printed
  - The program terminates with exit code `1`

✅ Port 5000 is an arbitrary but common choice for games or personal apps.

------

##### 🧩 `std::cerr << "[Server] Failed to start server.\n";`

- Logs an error if the server could not be started (due to port binding issues, socket creation failure, etc.).

------

##### 🧩 `std::cout << "[Server] Client connected!\n";`

- If `host()` succeeds, this message is displayed after the client connection is accepted.
- This lets the host know the game is ready to proceed to name exchange and gameplay.

### Exchanging Player Names Between Server and Client

```c++
// Exchange names
net.sendMessage("NAME|" + playerName);
std::string nameMsg = net.receiveMessage(); // NAME|<clientName>
std::string clientName = nameMsg.substr(nameMsg.find('|') + 1);

std::cout << "[Server] Connected with player: " << clientName << "\n";
```

------

#### ✅ **Purpose**

This code enables the **server and client to exchange their names** after the connection is established, ensuring both players are identified properly during the game.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `net.sendMessage("NAME|" + playerName);`

- Sends a message to the client containing the host player's name.
- The format `"NAME|<playerName>"` is a **simple custom protocol**.
  - `NAME|` is the **tag** or **prefix**.
  - `playerName` is the actual data (e.g., `"NAME|Rakesh"`).

This ensures that the client knows this message contains a name (not a game move or balance update).

------

##### 🧩 `std::string nameMsg = net.receiveMessage();`

- Waits to receive the client's name message, expected to also be in `"NAME|<clientName>"` format.
- `receiveMessage()` blocks until the client sends a message.
- The result might be: `"NAME|Hans"`

------

##### 🧩 `std::string clientName = nameMsg.substr(nameMsg.find('|') + 1);`

- Extracts the **client’s name** from the received message.
- `nameMsg.find('|') + 1` skips past the `NAME|` part.
- This gives you only the player name, e.g., `"Hans"`.

------

##### 🧩 `std::cout << "[Server] Connected with player: " << clientName << "\n";`

- Displays the client's name in the console, confirming the name exchange succeeded.
- This message is useful for confirmation before starting the game loop.

------

##### 📦 Why Use the `"NAME|"` Format?

- It acts as a **simple protocol** to distinguish message types.
- You could later extend this:
  - `ROLL|4`, `STAKE|10`, `EXIT|reason`, etc.
- This format makes it easy to parse messages with `substr()` and `find('|')`.

### Creating Player Objects and Starting the Game Loop

```c++
Player local(playerName, true);        // Server is host
Player remote(clientName, false);      // Client is remote
GameManager game(local, remote, net);
game.startGame();
```

------

#### ✅ **Purpose**

This snippet sets up the two players (host and client), passes them into the game controller (`GameManager`), and begins the actual gameplay loop.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `Player local(playerName, true);`

- Creates a `Player` object for the **host** (local player).
- Parameters:
  - `playerName`: the name entered by the host earlier.
  - `true`: indicates that this player **is the host**.
- This object will track the host’s name, balance, and game status.

------

##### 🧩 `Player remote(clientName, false);`

- Creates a `Player` object for the **connected client**.
- Parameters:
  - `clientName`: extracted from the `"NAME|..."` message sent by the client.
  - `false`: indicates this player is **not the host** (i.e., remote).
- This object allows the host to store and reference the other player’s data.

------

##### 🧩 `GameManager game(local, remote, net);`

- Constructs a `GameManager` object, which acts as the **game controller**.
- It takes:
  - `local` (host’s `Player` object)
  - `remote` (client’s `Player` object)
  - `net` (the `NetworkManager`, for sending/receiving messages)
- `GameManager` will use these objects to manage:
  - Turn-taking
  - Guess/stake inputs
  - Dice rolls
  - Win/loss evaluation
  - Balance updates
  - Network communication

------

##### 🧩 `game.startGame();`

- Starts the main game loop:
  - Continues until one player runs out of money or exits
  - Handles all input/output, including message sending/receiving
  - Keeps the game session running between both players