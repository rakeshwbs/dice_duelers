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

| Category              | Syntax                                                       | Copies Data? | Can Modify Original? | Performance (large objects) | Typical Use Case                                   |
| --------------------- | ------------------------------------------------------------ | ------------ | -------------------- | --------------------------- | -------------------------------------------------- |
| **Pass by Value**     | `void func(int a) <br>void func(std::string s)`              | ✅ Yes        | ❌ No                 | ❌ Slow for objects          | Small data types where modification is not needed  |
| **Pass by Reference** | `void func(int& a)<br>void func(std::string& s)`             | ❌ No         | ✅ Yes                | ✅ Fast                      | When the function **needs to modify** the argument |
| **Const Reference**   | `void func(const int& a)<br>void func(const std::string& s)` | ❌ No         | ❌ No                 | ✅ Fast                      | Read-only input without copying large objects      |

### Detailed Explanation

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

