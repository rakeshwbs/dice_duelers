# Tutorial 5: Introduction to Socket Programming in C++ (Windows)

###  What it will cover:

- What is a socket?
- Client-server model overview
- Winsock basics (specific to Windows)
- Common functions: `WSAStartup()`, `socket()`, `bind()`, `listen()`, `accept()`, `connect()`, `send()`, `recv()`, `closesocket()`, `WSACleanup()`
- How data is sent and received over TCP
- Key challenges: message boundaries, blocking vs non-blocking, connection lifecycle

This provides the **theoretical and practical background** that your `NetworkManager` class builds upon.

### **Objective:**

Build a strong foundation in socket programming using C++ on Windows, so you can understand how multiplayer communication works in Dice Duelers.

### What Is a Socket?

A **socket** is like a virtual plug that allows a program to send or receive data over a **network**.

There are two main types:

- **Stream sockets (TCP)** — reliable, ordered, connection-based (used in your game)
- **Datagram sockets (UDP)** — faster but less reliable

### The Client-Server Model

Most networked applications follow this architecture:

| Role       | Description                                 |
| ---------- | ------------------------------------------- |
| **Server** | Waits for connections (like hosting a room) |
| **Client** | Initiates the connection (joins the room)   |

### Winsock: Windows Sockets API

On Windows, socket programming is done using the **Winsock API**, which wraps C-style networking functions. It’s required to initialize and clean up socket use.

### Common Winsock Functions

| Function        | Description                                    |
| --------------- | ---------------------------------------------- |
| `WSAStartup()`  | Initializes the Winsock library                |
| `socket()`      | Creates a socket                               |
| `bind()`        | Assigns an IP + port to a socket (server only) |
| `listen()`      | Listens for incoming connections               |
| `accept()`      | Accepts a new connection from a client         |
| `connect()`     | Connects to a server (client only)             |
| `send()`        | Sends data over the socket                     |
| `recv()`        | Receives data from the socket                  |
| `closesocket()` | Closes the socket connection                   |
| `WSACleanup()`  | Cleans up Winsock usage                        |

### Server Code Skeleton

```c++
WSADATA wsaData;
WSAStartup(MAKEWORD(2, 2), &wsaData);

SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

sockaddr_in serverAddr{};
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(5000);
serverAddr.sin_addr.s_addr = INADDR_ANY;

bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
listen(listenSock, SOMAXCONN);

SOCKET clientSock = accept(listenSock, nullptr, nullptr);
```

### Client Code Skeleton

```c++
WSADATA wsaData;
WSAStartup(MAKEWORD(2, 2), &wsaData);

SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

sockaddr_in serverAddr{};
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(5000);
inet_pton(AF_INET, "192.168.1.2", &serverAddr.sin_addr);

connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
```

### Sending Messages

```c++
// Send
std::string msg = "GUESS|3\n";
send(sock, msg.c_str(), msg.size(), 0);
```

> TCP doesn’t preserve message boundaries — that’s why we use a delimiter like `\n`.

#### Why TCP does not preserve message boundaries?

##### TCP Is a Stream-Oriented Protocol

Unlike UDP, which sends data in discrete packets (datagrams), **TCP is stream-based**. 

This means:

- TCP treats data as a **continuous flow of bytes**.
- It **does not understand where one message ends and another begins**.
- Your application must manually define and recognize message boundaries.

##### What Happens Without Delimiters?

Assume you send this:

```c++
send(sock, "GUESS|3\n", 8, 0);
send(sock, "GUESS|4\n", 8, 0);
```

The receiver might read:

- `"GUESS|3\nGUE"` on first `recv()`
- `"SS|4\n"` on the next

Or it could read the entire `"GUESS|3\nGUESS|4\n"` in one `recv()`.

> TCP doesn't guarantee that your messages will be **delivered in the same chunks** as they were sent — only that **all bytes will arrive in order**.

#####  Role of Delimiters (`\n`, `|`, etc.)

Delimiters help your program detect **where one logical message ends**.

```c++
// Send:
"GUESS|3\n"

// On receive:
while (data available) {
    buffer += recv(...);
    if (buffer contains '\n') {
        extract and process message;
    }
}
```

So, you use:

- **`\n`** to mark the **end of a message**
- **`|`** to separate **fields inside a message** (e.g., command and parameter)

#### Best Practices in TCP Message Framing

| Approach               | Description                                          |
| ---------------------- | ---------------------------------------------------- |
| **Delimiter-based**    | Use a special character (e.g., `\n`, `;`, `\0`)      |
| **Length-prefixing**   | Send a fixed-size header first (e.g., 4-byte length) |
| **Protocol structure** | Use structured formats (e.g., JSON, Protobuf, XML)   |

#### `send()` – Sending Data

###### Prototype:

```c++
int send(SOCKET s, const char* buf, int len, int flags);
```

###### Parameters:

| Parameter | Description                                  |
| --------- | -------------------------------------------- |
| `s`       | The socket to send through                   |
| `buf`     | The data to send (as a char pointer)         |
| `len`     | Length of the data in bytes                  |
| `flags`   | Usually 0 (used for advanced socket options) |

###### Returns:

- **Bytes sent**, or
- `SOCKET_ERROR` on failure

###### Example:

```c++
std::string msg = "GUESS|4\n";
send(sock, msg.c_str(), msg.size(), 0);
```

- Sends the raw characters of `msg` over the network
- `\n` at the end acts as a **delimiter** so the receiver knows where the message ends

### `recv()` – Receiving Data

```c++
// Receive
char buffer[1024] = {};
int bytes = recv(sock, buffer, sizeof(buffer), 0);
std::string received(buffer, bytes);
```

###### Prototype:

```c++
int recv(SOCKET s, char* buf, int len, int flags);
```

###### Parameters:

| Parameter | Description                     |
| --------- | ------------------------------- |
| `s`       | The socket to read from         |
| `buf`     | Buffer to store incoming data   |
| `len`     | Maximum number of bytes to read |
| `flags`   | Usually 0                       |

###### Returns:

- Number of **bytes actually received**
- 0 → connection has been closed
- `SOCKET_ERROR` on error

######  Example:

```c++
char buffer[1024];
int bytes = recv(sock, buffer, sizeof(buffer), 0);
std::string msg(buffer, bytes);
```

- Reads **up to 1024 bytes** from the socket
- Result is turned into a `std::string` for easier processing

### Important Behavior of `send()` and `recv()`

##### TCP is a **stream** protocol

This means:

- Messages may **arrive in chunks**, even if they were sent as one piece
- You might get **half a message**, or **multiple messages in one call**
- Therefore, it's up to **you (the programmer)** to:
  - Define **message boundaries** (use delimiters like `\n`)
  - **Buffer and reassemble** partial messages

#### Closing the Connection

```c++
closesocket(sock);
WSACleanup();
```

### Important Concepts

| Concept             | Why It Matters                                       |
| ------------------- | ---------------------------------------------------- |
| TCP is stream-based | You may receive partial or multiple messages at once |
| Delimiters          | Needed to define message boundaries (`\n`)           |
| Blocking calls      | `recv()` can freeze your app if no data arrives      |
| Cleanup             | You **must** call `WSACleanup()` when done           |

## Scenario: Client Sends a Guess | Host Receives It

Let’s say:

- The **client guesses 4**
- It sends `"GUESS|4\n"` to the host

### On the Client Side

##### 1. The GameManager sends the guess:

```c++
std::string msg = "GUESS|4\n";
send(sock, msg.c_str(), msg.length(), 0);
```

- `msg.length()` = 8 (includes the delimiter `\n`)
- `sock` is the connected socket
- **All 8 bytes** are sent

Note:

- In C++ strings, `"\n"` is a **single character**: the **newline character**.
- Although it's **written using two symbols** (`\` and `n`), it is interpreted as **one byte** with ASCII value `10`.

In the example:

```c++
std::string msg = "GUESS|4\n";
```

The actual string contains these characters:

| Character | ASCII Code | Byte |
| --------- | ---------- | ---- |
| G         | 71         | 1    |
| U         | 85         | 1    |
| E         | 69         | 1    |
| S         | 83         | 1    |
| S         | 83         | 1    |
| \|        | 124        | 1    |
| 4         | 52         | 1    |
| \n        | 10         | 1    |

Total = **8 bytes**

- \n is a single byte/character (newline, ASCII code 10)
- It’s treated as one unit in memory and network transmission
- That’s why msg.length() returns 8 in your example

### On the Network

TCP ensures:

- The 8 bytes are reliably delivered
- But delivery is **streamed**: they may arrive together or in parts

Possibilities:

- All 8 bytes arrive at once
- Or: 5 bytes arrive, then 3 later
- Or: 2 messages arrive back-to-back in one `recv()`

### On the Host Side

##### 2. `recv()` is called inside `receiveMessage()`

```c++
char temp[1024];
int bytes = recv(sock, temp, sizeof(temp), 0);
messageBuffer += std::string(temp, bytes);
```

Let’s say all 8 bytes arrive in one go.

Now:

```c++
messageBuffer == "GUESS|4\n"
```

3. ##### Extract the complete message:

```c++
size_t newlinePos = messageBuffer.find('\n');
if (newlinePos != std::string::npos) {
    std::string msg = messageBuffer.substr(0, newlinePos);
    messageBuffer = messageBuffer.substr(newlinePos + 1);
}
```

Now:

- `msg == "GUESS|4"`
- `messageBuffer` becomes empty again (ready for next message)

✅ Host now has the client’s guess!

##### What if Only Half the Message Arrives?

Suppose only `"GUESS|"` arrives in the first `recv()`.

- `messageBuffer = "GUESS|"` → no `\n` yet
- `receiveMessage()` returns `[WAIT]`

In the **next call** to `recv()`, `"4\n"` arrives.

- `messageBuffer += "4\n"` → now it becomes `"GUESS|4\n"`
- `receiveMessage()` extracts the full message `"GUESS|4"`

✅ This shows why buffering + newline delimiters are crucial.

### Final Notes: Socket Programming in C++

Socket programming is the foundation of **networked applications**, allowing separate programs — often on different machines — to communicate in real time. In Dice Duelers, it enables two players to interact over LAN using **TCP sockets**, ensuring reliable message delivery for game synchronization.

#### Key Takeaways

1. **Socket is like a network pipe**
   - Think of it as a two-way communication tunnel between two programs.
2. **Winsock (Windows Sockets)**
   - Windows-specific API for socket operations: `WSAStartup()`, `socket()`, `connect()`, etc.
3. **TCP is stream-based**
   - Data is not guaranteed to arrive in message-sized chunks.
   - You must **define message boundaries** (e.g., with `\n`) and **buffer incomplete messages**.
4. **Client-Server model**
   - One side hosts (`bind()` + `listen()` + `accept()`), the other connects (`connect()`).
   - After connection, both use `send()` and `recv()`.
5. **Always clean up**
   - Use `closesocket()` and `WSACleanup()` to prevent leaks and crashes.
6. **Delimiters are your friend**
   - `\n` helps detect where one message ends and another begins in a TCP stream.
7. **Buffering is essential**
   - Use an internal buffer (`messageBuffer`) to accumulate data until full messages are available.

### Why This Matters for the Game

In **Dice Duelers**, the entire multiplayer experience depends on:

- **timely and accurate message exchange**
- **synchronizing game state across machines**
- **handling disconnects, timeouts, and partial messages gracefully**

By understanding socket fundamentals, you’re now equipped to:

- Debug and expand the networking layer
- Extend it to more players or a chat feature
- Port it to other systems (e.g., Linux or cross-platform)