# Tutorial 6: NetworkManager – Wrapping Winsock into a Simple Multiplayer Interface

#### NetworkManager.h

```c++
// NetworkManager.h
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>

class NetworkManager {
public:
    bool host(int port);
    bool connectToHost(const std::string& ip, int port);
    bool sendMessage(const std::string& message);
    std::string receiveMessage();
    void closeConnection();
};

#endif
```

#### NetworkManager.cpp

```c++
#include "NetworkManager.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

SOCKET sockfd;
static std::string messageBuffer;

bool NetworkManager::host(int port) {
    WSADATA wsaData;
    sockaddr_in serverAddr{}, clientAddr{};
    int clientLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cerr << "[Server] WSAStartup failed.\n";
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "[Server] Socket creation failed.\n";
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Server] Bind failed.\n";
        return false;
    }

    listen(sockfd, 1);
    SOCKET clientSock = accept(sockfd, (sockaddr*)&clientAddr, &clientLen);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "[Server] Accept failed.\n";
        return false;
    }

    closesocket(sockfd);
    sockfd = clientSock;
    return true;
}

bool NetworkManager::connectToHost(const std::string& ip, int port) {
    WSADATA wsaData;
    sockaddr_in serverAddr{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cerr << "[Client] WSAStartup failed.\n";
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "[Client] Socket creation failed.\n";
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Client] Connection failed.\n";
        return false;
    }

    return true;
}

bool NetworkManager::sendMessage(const std::string& message) {
    std::string msgWithDelimiter = message + "\n";
    return send(sockfd, msgWithDelimiter.c_str(), msgWithDelimiter.size(), 0) != SOCKET_ERROR;
}

std::string NetworkManager::receiveMessage() {
    char buffer[1024];
    int bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) return "[ERROR]";
    buffer[bytesReceived] = '\0';
    messageBuffer += buffer;

    size_t newlinePos = messageBuffer.find('\n');
    if (newlinePos != std::string::npos) {
        std::string msg = messageBuffer.substr(0, newlinePos);
        messageBuffer = messageBuffer.substr(newlinePos + 1);
        return msg;
    }

    return "[WAIT]"; // incomplete
}

void NetworkManager::closeConnection() {
    closesocket(sockfd);
    WSACleanup();
}
```

### Purpose of `NetworkManager`

This class wraps all Winsock code to:

- Host a server or connect to one
- Send/receive messages
- Handle message delimiting (`\n`)
- Clean up the socket connection

By encapsulating all that in one place, the rest of the game logic stays clean and focused.

2. ### `NetworkManager.h` – Interface

```c++
class NetworkManager {
public:
    bool host(int port);
    bool connectToHost(const std::string& ip, int port);
    bool sendMessage(const std::string& message);
    std::string receiveMessage();
    void closeConnection();
};
```

##### Note:

The ampersand (`&`) after `const std::string&` in your function parameters like:

```c++
bool connectToHost(const std::string& ip, int port);
bool sendMessage(const std::string& message);
```

has a very specific and important purpose in C++.

| Component       | Purpose                                                      |
| --------------- | ------------------------------------------------------------ |
| `std::string`   | A standard string object                                     |
| `&` (reference) | Passes the **reference to the original object**, not a copy  |
| `const`         | Ensures the function will **not modify** the passed-in string |

##### Why Use `const std::string&` Instead of `std::string`

### Without Reference: `std::string`

- Copies the entire string object.
- Less efficient, especially with large strings.
- Modifications won't affect the original — but wastes time/memory.

### With Reference: `const std::string&`

- **Avoids copying** the string.
- **Improves performance** (zero-copy).
- **Protects against modification** via `const`.

#### Example Comparison

##### Slower (copying string)

```c++
bool sendMessage(std::string message);  // Copies the string
```

##### Faster and safer

```c++
// Uses original without copying or modifying
bool sendMessage(const std::string& message);  
```

#### Notes:

Using `const std::string&`:

- Is a **best practice** in function parameters for read-only input.
- **Avoids performance overhead** of copying.
- **Prevents accidental modification** of the passed string.

#### Comparison Table: Value vs Reference in C++

| **Category**          | **Syntax**                                                | **Copies Data?** | **Can Modify Original?** | **Performance (large objects)** | **Typical Use Case**                               |
| --------------------- | --------------------------------------------------------- | ---------------- | ------------------------ | ------------------------------- | -------------------------------------------------- |
| **Pass by Value**     | `void func(int a)`<br>`func(std::string s)`               | ✅ Yes            | ❌ No                     | ❌ Slow for objects              | Small data types where modification is not needed  |
| **Pass by Reference** | `void func(int& a)`<br>`func(std::string& s)`             | ❌ No             | ✅ Yes                    | ✅ Fast                          | When the function **needs to modify** the argument |
| **Const Reference**   | `void func(const int& a)`<br>`func(const std::string& s)` | ❌ No             | ❌ No                     | ✅ Fast                          | Read-only input without copying large objects      |

#### ✅ **Pass by Value**

- Makes a copy of the argument.
- Safe, but **inefficient for large objects**.
- Changes inside the function do **not affect** the original variable.

#### ✅ **Pass by Reference**

- No copy made; the function works directly with the original.
- Efficient, but **allows modification** of the original (which may be unintended).

#### ✅ **Pass by Const Reference**

- No copy made; the original is **protected** by `const`.
- Best of both worlds: **efficient** and **safe**.
- Most commonly used for **read-only inputs**, especially with class objects.

##### Example (with `std::string`)

```c++
// Passby value
void show(std::string s);        // copies the whole string

// Pass by reference
void modify(std::string& s);     // can change original

// Pass by const reference
void print(const std::string& s); // efficient, safe, read-only
```

### `NetworkManager.cpp` – Implementation

#### Global Declarations

```c++
SOCKET sockfd;
static std::string messageBuffer;
```

`sockfd`: The main connection socket (shared by host or client)

`messageBuffer`: Used to accumulate data across multiple `recv()` calls

#### host(int port)

```c++
bool NetworkManager::host(int port) {
    WSADATA wsaData;
    sockaddr_in serverAddr{}, clientAddr{};
    int clientLen = sizeof(clientAddr);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cerr << "[Server] WSAStartup failed.\n";
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "[Server] Socket creation failed.\n";
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Server] Bind failed.\n";
        return false;
    }

    listen(sockfd, 1);
    SOCKET clientSock = accept(sockfd, (sockaddr*)&clientAddr, &clientLen);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "[Server] Accept failed.\n";
        return false;
    }

    closesocket(sockfd);
    sockfd = clientSock;
    return true;
}
```

1. Calls `WSAStartup()` to initialize Winsock.
2. Creates a socket with `socket()`.
3. Binds it to the given port (`bind()`).
4. Listens for one client (`listen()`).
5. Accepts the connection (`accept()`).
6. Replaces `sockfd` with the client socket.

✅ After this, the server is ready to communicate.

```c++
WSADATA wsaData;
sockaddr_in serverAddr{}, clientAddr{};
int clientLen = sizeof(clientAddr);
```

##### Purpose:

- Prepares necessary structures for Winsock and socket addressing.
- `wsaData` → for Winsock startup
- `serverAddr` → server's own IP/port config
- `clientAddr` → stores connecting client info after `accept()`
- `clientLen` → length required by `accept()`

##### Step 1: Initialize Winsock

```c++
if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
    std::cerr << "[Server] WSAStartup failed.\n";
    return false;
}
```

- `WSAStartup()` is **mandatory on Windows**
- Loads version **2.2** of Winsock.
- Returns non-zero on failure.

✅ Without this call, all subsequent socket functions will fail.

##### Step 2: Create the Server Socket

```c++
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd == INVALID_SOCKET) {
    std::cerr << "[Server] Socket creation failed.\n";
    return false;
}
```

- `AF_INET`: use IPv4
- `SOCK_STREAM`: use TCP (not UDP)
- `0`: use default protocol (TCP)

✅ This line creates a socket for accepting connections.

##### Step 3: Configure Server Address

```c++
serverAddr.sin_family = AF_INET;
serverAddr.sin_addr.s_addr = INADDR_ANY;
serverAddr.sin_port = htons(port);
```

| Field                    | Meaning                                            |
| ------------------------ | -------------------------------------------------- |
| `sin_family`             | IPv4                                               |
| `s_addr = INADDR_ANY`    | Accept connections from **any IP on this machine** |
| `sin_port = htons(port)` | Convert port to **network byte order**             |

`htons()` is required because network protocols use **big-endian** format.

##### Step 4: Bind Socket to the IP + Port

```c++
if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cerr << "[Server] Bind failed.\n";
    return false;
}
```

- Tells the OS: “Associate this socket with this IP and port.”
- Returns `SOCKET_ERROR` if the port is already in use or blocked.

✅ This must be done before listening.

#####  Step 5: Listen for Incoming Connections

```c++
listen(sockfd, 1);
```

- Places the socket into **listening mode**.
- `1` means allow **only one queued connection** (you only support 2 players).

✅ The server is now ready to accept connections.

##### Step 6: Accept a Connection

```c++
SOCKET clientSock = accept(sockfd, (sockaddr*)&clientAddr, &clientLen);
if (clientSock == INVALID_SOCKET) {
    std::cerr << "[Server] Accept failed.\n";
    return false;
}
```

- Blocks until a client connects.
- When it does, `accept()` returns a **new socket** (`clientSock`) for communication.

> The original `sockfd` is still a *listening* socket, but we only accept **one client**, so...

##### Step 7: Promote Client Socket

```c++
closesocket(sockfd);
sockfd = clientSock;
```

- No longer need the listening socket.
- Close it.
- Replace `sockfd` with the **connected socket** so that `send()` and `recv()` now use it.

### Step 8: Hosting Successful 🎉

```c++
return true;
```

Now your server is:

- Live on the chosen port
- Connected to one client
- Ready to send and receive data over `sockfd`

#### Visual Recap

> WSAStartup() → socket() → bind() → listen() → accept() → closesocket(listen) → ✅ connected!

#### bool NetworkManager::connectToHost(const std::string& ip, int port)

This method is used by the **client** to connect to a host that is listening on a given IP address and port.

```c++
bool NetworkManager::connectToHost(const std::string& ip, int port) {
    WSADATA wsaData;
    sockaddr_in serverAddr{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
        std::cerr << "[Client] WSAStartup failed.\n";
        return false;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "[Client] Socket creation failed.\n";
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[Client] Connection failed.\n";
        return false;
    }

    return true;
}
```

#### Explanation:

```c++
WSADATA wsaData;
sockaddr_in serverAddr{};
```

##### Purpose:

- `wsaData`: Structure used to initialize Winsock
- `serverAddr`: Structure to describe the host's address (IP and port)

#####  Step 1: Initialize Winsock

```c++
if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
    std::cerr << "[Client] WSAStartup failed.\n";
    return false;
}
```

- Same as the server.
- Required before using any Winsock functionality.
- Returns non-zero on failure.

##### Step 2: Create a Socket

```c++
sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd == INVALID_SOCKET) {
    std::cerr << "[Client] Socket creation failed.\n";
    return false;
}
```

- `AF_INET` = IPv4
- `SOCK_STREAM` = TCP
- `0` = use default TCP protocol

✅ This socket will be used to initiate the connection to the server.

#####  Step 3: Configure Server Address

```c++
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(port);
inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);
```

| Field         | Description                    |
| ------------- | ------------------------------ |
| `AF_INET`     | IPv4                           |
| `htons(port)` | Convert to network byte order  |
| `inet_pton()` | Convert string IP to binary IP |

- `inet_pton()` converts `"192.168.1.5"` → actual 32-bit binary address
- Fills `serverAddr` with IP + port of the server to connect to

##### Step 4: Attempt to Connect

```c++
if (connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cerr << "[Client] Connection failed.\n";
    return false;
}
```

- Tries to connect the socket to the **host** described in `serverAddr`
- Returns `SOCKET_ERROR` if the server isn’t reachable, or connection is refused

✅ On success, the connection is established — ready to `send()` and `recv()`

#####  Step 5: Success

```c++
return true;
```

- The client is now connected to the host.
- GameManager can proceed with message exchange.

#### Recap of `connectToHost()` Logic:

> WSAStartup() → socket() → configure serverAddr → inet_pton() → connect() → ✅ connected!



#### bool sendMessage(const std::string& message)

```c++
bool NetworkManager::sendMessage(const std::string& message) {
    std::string msgWithDelimiter = message + "\n";
    return send(sockfd, msgWithDelimiter.c_str(), msgWithDelimiter.size(), 0) != SOCKET_ERROR;
}
```

##### Explanation:

##### 1. Add a newline delimiter:

```c++
std::string msgWithDelimiter = message + "\n";
```

- CP is **stream-based**, so it doesn't preserve message boundaries.
- `\n` is used as a **manual delimiter** to signal **end of message**.

> Example: `"GUESS|3"` becomes `"GUESS|3\n"`

2. ##### Send the data:

```c++
send(sockfd, msgWithDelimiter.c_str(), msgWithDelimiter.size(), 0)
```

| Argument                   | Meaning                    |
| -------------------------- | -------------------------- |
| `sockfd`                   | The connected socket       |
| `msgWithDelimiter.c_str()` | Pointer to char array      |
| `msgWithDelimiter.size()`  | Length of message in bytes |
| `0`                        | Flags (not used here)      |

- `send()` returns number of bytes sent.
- If it returns `SOCKET_ERROR`, the send failed.

3. ##### Return success/failure:

```c++
!= SOCKET_ERROR
```

✅ Return `true` if message was sent, `false` if failed.

#### std::string receiveMessage()

```c++
std::string NetworkManager::receiveMessage() {
    char buffer[1024];
    int bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) return "[ERROR]";
    buffer[bytesReceived] = '\0';
    messageBuffer += buffer;

    size_t newlinePos = messageBuffer.find('\n');
    if (newlinePos != std::string::npos) {
        std::string msg = messageBuffer.substr(0, newlinePos);
        messageBuffer = messageBuffer.substr(newlinePos + 1);
        return msg;
    }

    return "[WAIT]"; // incomplete
}
```

#### Explanation:

##### 1. Read incoming data:

```c++
int bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
```

- Reads **up to 1023 bytes** into `buffer`.
- `recv()` blocks until data arrives or connection closes.
- `bytesReceived <= 0` means error or disconnect.

2. ##### Append to the message buffer:

   ```c++
   buffer[bytesReceived] = '\0';
   messageBuffer += buffer;
   ```

   TCP may deliver data in chunks.

   We append everything to a persistent `messageBuffer` string.

   This buffer stores **partial** or **multiple** messages.

3. ##### Check for complete message:

```c++
size_t newlinePos = messageBuffer.find('\n');
```

- Looks for the newline delimiter.
- If found → extract the first complete message.

4. ##### Extract and return the message:

```c++
std::string msg = messageBuffer.substr(0, newlinePos);
messageBuffer = messageBuffer.substr(newlinePos + 1);
return msg;
```

- Returns the full message (`GUESS|4`, `STAKE|20`, etc.)
- Removes the extracted portion from the buffer.

5. ##### If no full message yet:

```c++
return "[WAIT]";
```

- The function returns this signal so that `GameManager` knows to **keep waiting**.

#### Benefits of This Design

| Feature         | Benefit                               |
| --------------- | ------------------------------------- |
| `\n` delimiter  | Easy to detect message boundaries     |
| `messageBuffer` | Handles partial and merged messages   |
| `[WAIT]` signal | Prevents blocking logic in game       |
| Safe extraction | Avoids cutting off or misreading data |

#### void NetworkManager::closeConnection()

```c++
void NetworkManager::closeConnection() {
    closesocket(sockfd);
    WSACleanup();
}
```

##### What This Method Does:

This method is responsible for **cleanly shutting down the network connection** when the game ends — whether due to player choice or game logic.

##### `closesocket(sockfd);`

- Closes the socket that was used for communication.
- This:
  - Releases the system resources tied to the socket
  - Terminates the underlying TCP connection
- After this call, you **can’t send or receive data** on `sockfd` anymore.

> 🔐 Always close your sockets. Leaving them open can cause **resource leaks** or **port conflicts**.

##### `WSACleanup();`

- Cleans up the **Winsock library** on Windows.
- Should be called exactly once for each `WSAStartup()` you did.
- It:
  - Frees internal Winsock structures
  - Closes underlying network service handles

> 🧼 Without this, Winsock may remain in memory or block future connections.

##### When Is `closeConnection()` Called?

In the game:

- After the game ends (win/loss/exit), `GameManager::startGame()` calls:

```c++
net.closeConnection();
```

 This ensures:

- No sockets are left open
- Windows doesn’t complain or leave ports in a TIME_WAIT state
- The app can restart without crashing or requiring a reboot

#### Summary

| Line                   | Purpose                                 |
| ---------------------- | --------------------------------------- |
| `closesocket(sockfd);` | Ends the TCP connection cleanly         |
| `WSACleanup();`        | Cleans up all Winsock usage for the app |