# Tutorial 12 - Client Main

```c++
#include "../include/Player.h"
#include "../include/GameManager.h"
#include "../include/NetworkManager.h"
#include <iostream>

int main() {
    std::string playerName, serverIP;
    std::cout << "[Client] Enter your name: ";
    std::getline(std::cin, playerName);

    std::cout << "[Client] Enter server IP address: ";
    std::getline(std::cin, serverIP);

    NetworkManager net;
    if (!net.connectToHost(serverIP, 5000)) {
        std::cerr << "[Client] Connection to server failed.\n";
        return 1;
    }

    std::cout << "[Client] Connected to server.\n";

    // Exchange names
    net.sendMessage("NAME|" + playerName);
    std::string nameMsg = net.receiveMessage(); // NAME|<hostName>
    std::string hostName = nameMsg.substr(nameMsg.find('|') + 1);

    std::cout << "[Client] Playing with host: " << hostName << "\n";

    Player local(playerName, false);       // Client is not host
    Player remote(hostName, true);         // Host is remote
    GameManager game(local, remote, net);
    game.startGame();

    return 0;
}
```

### Capturing Client Name and Preparing for Server IP Input

```c++
std::string playerName, serverIP;
std::cout << "[Client] Enter your name: ";
std::getline(std::cin, playerName);
```

------

#### ✅ **Purpose**

This snippet captures the **local player’s name** (i.e., the client) as a string input from the user. This name is later sent to the server for identification during gameplay.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `std::string playerName, serverIP;`

- Declares two string variables:
  - `playerName`: will hold the name of the local client player
  - `serverIP`: will be used in the next step to store the server’s IP address for connection
- Declaring both here keeps input declarations grouped and clean.

------

##### 🧩 `std::cout << "[Client] Enter your name: ";`

- Prompts the user to type their name.
- The `[Client]` tag helps distinguish this output from server-side prompts, useful for debugging or LAN play on separate screens.

------

##### 🧩 `std::getline(std::cin, playerName);`

- Reads an entire line of text from standard input.
- This allows the user to enter a **name with spaces**, such as `"Player One"` or `"John Doe"`.
- Using `std::getline()` avoids the problem where `std::cin >> playerName` would stop reading at the first space.

### Connecting to the Host Server Using TCP

```c++
NetworkManager net;
if (!net.connectToHost(serverIP, 5000)) {
    std::cerr << "[Client] Connection to server failed.\n";
    return 1;
}
std::cout << "[Client] Connected to server.\n";
```

------

#### ✅ **Purpose**

This snippet creates a network connection to the host’s IP address on TCP port 5000. It uses the `NetworkManager` abstraction to hide the low-level Winsock details and handles failure gracefully.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `NetworkManager net;`

- Instantiates a `NetworkManager` object.
- This object wraps socket handling, send/receive operations, and cleanup.
- It allows the game code to call simple methods like `connectToHost()`, `sendMessage()`, and `receiveMessage()` without managing raw Winsock code.

------

##### 🧩 `if (!net.connectToHost(serverIP, 5000))`

- Attempts to connect to the server using the IP address the user provided (`serverIP`) and port `5000`.
- Internally:
  - Initializes Winsock with `WSAStartup`
  - Creates a TCP socket
  - Calls `connect()` with the IP and port
- If **any of these fail**, it returns `false`

------

##### 🧩 `std::cerr << "[Client] Connection to server failed.\n";`

- If the connection could not be established, this message is printed to `std::cerr` for visibility.
- This could happen if:
  - The server is not running
  - The IP address is wrong
  - A firewall is blocking the connection
  - The port number is incorrect or unavailable

------

##### 🧩 `return 1;`

- Ends the program early with an exit status `1`, indicating a controlled failure.
- Prevents further code (like name exchange or gameplay) from running without a valid connection.

### Exchanging Names with the Server After Connection

```c++
// Exchange names
net.sendMessage("NAME|" + playerName);
std::string nameMsg = net.receiveMessage(); // NAME|<hostName>
std::string hostName = nameMsg.substr(nameMsg.find('|') + 1);

std::cout << "[Client] Playing with host: " << hostName << "\n";
```

------

#### ✅ **Purpose**

This snippet allows the client to send its name to the server and receive the server’s (host's) name in return. This exchange is essential for **personalizing the game session** on both ends.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `net.sendMessage("NAME|" + playerName);`

- Sends a message to the server in the format:

  ```c++
  NAME|<playerName>
  ```

- The `NAME|` prefix serves as a **command identifier** so the receiver knows this message contains a name, not a game move.

- This is the **client's own name**, being sent to the server.

------

##### 🧩 `std::string nameMsg = net.receiveMessage();`

- Waits for a response from the server.
- The server is expected to send its own name in the same format:
   `"NAME|Rakesh"` or similar.
- This call blocks until the message is received.

------

##### 🧩 `std::string hostName = nameMsg.substr(nameMsg.find('|') + 1);`

- Extracts only the name portion from the incoming message.
- Uses `find('|')` to locate the separator.
- Then skips past it to capture just the name part.
  - For example: if `nameMsg = "NAME|Rakesh"`, then `hostName = "Rakesh"`

------

##### 🧩 `std::cout << "[Client] Playing with host: " << hostName << "\n";`

- Displays a confirmation message with the host’s name.
- This helps the player verify they’ve connected to the correct game.

------

##### 📦 Design Note

This is a part of your **application-level protocol**, which uses human-readable, prefixed string commands (`NAME|`, `GUESS|`, `ROLL|`, etc.) to communicate between players.

### Creating Player Objects and Starting the Game as Client

```c++
Player local(playerName, false);       // Client is not host
Player remote(hostName, true);         // Host is remote
GameManager game(local, remote, net);
game.startGame();
```

------

#### ✅ **Purpose**

This snippet:

- Creates the two `Player` objects for this game session
- Instantiates the `GameManager`, which controls all game logic
- Starts the main game loop

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `Player local(playerName, false);`

- Creates a `Player` instance for the **local player**, i.e., the client
- Arguments:
  - `playerName`: the name entered earlier by the user
  - `false`: indicates this player is **not the host**

🔹 This object will track:

- Client’s name
- Balance
- Game progress (wins/losses)

------

##### 🧩 `Player remote(hostName, true);`

- Creates a `Player` instance to represent the **host player**
- Arguments:
  - `hostName`: extracted from the `"NAME|..."` message received earlier
  - `true`: indicates that this player is the **host**, even though it’s remote from the client's perspective

🔹 This player is used to:

- Store the server player’s name

- Display messages like:

  ```c++
  Rakesh wins this round!
  ```

------

##### 🧩 `GameManager game(local, remote, net);`

- Constructs a `GameManager` instance — the main **controller** of the game.
- Arguments:
  - `local`: client-side player object
  - `remote`: server-side player object
  - `net`: the network interface used for communication

🔹 `GameManager` handles:

- Turn management
- Dice rolling
- Guess/stake collection
- Balance updates
- Win/loss evaluation
- Sending and receiving all game-related messages

------

##### 🧩 `game.startGame();`

- Begins the game loop
- It will:
  - Prompt each player in turn
  - Transmit data to/from the host
  - Process and display results
  - Terminate when someone runs out of money or exits