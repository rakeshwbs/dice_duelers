# Tutorial 10 - Client Class

### Initializing the Client Socket Handle

```c++
Client::Client() {
    clientSocket = INVALID_SOCKET;
}
```

------

#### ✅ **Purpose**

This is the **constructor** of the `Client` class. It prepares the client object for use by initializing the internal socket handle to a known safe value.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `Client::Client()`

- This is a default constructor.
- It is automatically called when an instance of the `Client` class is created.
- Used to initialize member variables before the actual connection is made.

##### 🧩 `clientSocket = INVALID_SOCKET;`

- `INVALID_SOCKET` is a Winsock constant that represents an uninitialized or invalid socket.

- Setting `clientSocket` to this value tells the rest of the program:

  > “There is no active socket connection yet.”

- This prevents functions like `closesocket()` or `send()` from accidentally being called on an undefined socket handle.

------

##### 🔐 Why It's Important

- Ensures safe socket lifecycle management

- Helps the `close()` function later on:

  ```c++
  if (clientSocket != INVALID_SOCKET)
      closesocket(clientSocket);
  ```

- Makes the class state predictable from the moment it’s created

### Initializing the Winsock Library on the Client Side

------

```
if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed.\n";
    return false;
}
```

------

#### ✅ **Purpose**

This snippet initializes the **Windows Sockets API (Winsock)** for use in the client application. Winsock must be started before any socket-related functions can be used (like `socket()`, `connect()`, `recv()`, etc.).

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `WSAStartup(...)`

- This is a required Winsock function.
- It loads and prepares the Winsock library for use by the application.

------

##### 🧩 `MAKEWORD(2, 2)`

- Specifies the **version** of Winsock the application requests — in this case, **version 2.2** (which is standard and widely supported).
- This version is encoded into a single 16-bit value using the `MAKEWORD` macro.

------

##### 🧩 `&wsaData`

- This is a pointer to a `WSADATA` structure, where Winsock will store version and status info.
- This info can include the actual version loaded and descriptions of the Winsock implementation.
- Even if you don’t use the `wsaData` info, it is **required** by the function call.

------

##### 🧩 `!= 0`

- If `WSAStartup()` returns a non-zero value, it indicates **failure**.
- This can happen due to:
  - Missing DLLs
  - Corrupted Winsock installation
  - Version mismatch
  - Privilege issues
- In such a case, the program logs an error and returns `false`, preventing any further network operations.

------

##### 🔐 Why It’s Important

- Winsock is not available by default in a Windows program — it must be explicitly initialized with `WSAStartup()`.
- Every `WSAStartup()` **must** eventually be paired with a `WSACleanup()` call (usually in the `close()` method).

### Creating the TCP Socket for the Client

```c++
clientSocket = socket(AF_INET, SOCK_STREAM, 0);
if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Socket creation failed.\n";
    return false;
}
```

------

#### ✅ **Purpose**

This code creates a **TCP socket** that the client will use to connect to the server. It's a fundamental step in building the client-server connection.

------

#### 🔍 **Line-by-Line Explanation**

🧩 `clientSocket = socket(AF_INET, SOCK_STREAM, 0);`

- **`socket()`** is a Winsock function that creates a socket.
- Parameters:
  - `AF_INET`: Address family — **IPv4** (Internet Protocol v4).
  - `SOCK_STREAM`: Type — **stream socket**, which uses **TCP** (as opposed to `SOCK_DGRAM` for UDP).
  - `0`: Protocol — zero means use the default protocol for this socket type (which will be TCP for `SOCK_STREAM`).

🔹 The result is a new socket descriptor, stored in `clientSocket`. This socket will later be used in the `connect()` call.

------

🧩 `if (clientSocket == INVALID_SOCKET)`

- If the socket creation failed, `socket()` returns `INVALID_SOCKET`.
- This means the socket cannot be used, and something went wrong (e.g., out of system resources, invalid parameters, or Winsock not initialized properly).

🧩 `std::cerr << "Socket creation failed.\n";`

- Logs a human-readable error message to the console so the user knows what went wrong.

🧩 `return false;`

- Exits the `connectToServer()` function early, indicating failure to set up the client socket.

### Configuring the Server Address and Establishing the TCP Connection

------

### 🔹 **Snippet Title:** “Configuring the Server Address and Establishing the TCP Connection”

```c++
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(port);
serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cerr << "Connection to server failed.\n";
    return false;
}

std::cout << "Connected to server!\n";
return true;
```

------

#### ✅ **Purpose**

This snippet configures the client socket’s destination address (`serverAddr`) and attempts to establish a **TCP connection** to the server using `connect()`.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `serverAddr.sin_family = AF_INET;`

- Specifies the address family — in this case, **IPv4**.
- Must match the family used when the socket was created (`AF_INET`).

------

##### 🧩 `serverAddr.sin_port = htons(port);`

- Sets the destination **port number** for the connection.
- `htons()` converts the port number from host byte order to **network byte order** (big-endian).
- Required by the TCP/IP stack.

------

##### 🧩 `serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());`

- Converts the **IP address string** (like `"192.168.1.10"`) to a 32-bit binary format using `inet_addr()`.
- Assigns this binary IP to `sin_addr.s_addr` so that the client knows where to connect.
- Note: `inet_addr()` is deprecated in favor of `inet_pton()` in modern code, but it still works fine in this context.

------

##### 🧩 `connect(...)`

```c++
if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
```

- Attempts to establish a **TCP connection** to the specified IP and port.
- Parameters:
  - `clientSocket`: the socket previously created with `socket()`
  - `&serverAddr`: pointer to the configured address structure
  - `sizeof(serverAddr)`: size of the address structure
- If the server is running and listening, the TCP handshake will succeed.
- If not:
  - The function will block for several seconds (default timeout), then fail.

##### 🧩 `if (...) == SOCKET_ERROR`

- If the connection attempt fails, the function returns `false` and logs an error.
- Common reasons for failure:
  - Server isn’t running
  - Wrong IP or port
  - Firewall blocking connection
  - Socket was not properly created or initialized

------

##### 🧩 `std::cout << "Connected to server!\n";`

- Provides feedback that the connection to the server succeeded.
- At this point, the client and server can now **exchange messages using `send()` and `recv()`**.

##### 🧩 `return true;`

- Signals to the calling code that connection was successful.

### Sending a Message to the Server over TCP

```c++
bool Client::sendMessage(const std::string& message) {
    int result = send(clientSocket, message.c_str(), message.length(), 0);
    return result != SOCKET_ERROR;
}
```

------

#### ✅ **Purpose**

This method sends a `std::string` message from the client to the server over a TCP socket. It returns `true` if the message is sent successfully, and `false` if the send operation fails.

------

#### 🔍 **Line-by-Line Explanation**

##### 🧩 `int result = send(...)`

```c++
send(clientSocket, message.c_str(), message.length(), 0);
```

- Uses Winsock’s `send()` function to transmit data over a TCP connection.
- Parameters:
  - `clientSocket`: the socket connected to the server
  - `message.c_str()`: a C-style null-terminated string representation of the message
  - `message.length()`: number of bytes to send (excluding the null terminator)
  - `0`: default flags (no special behavior)

##### What it does:

- Sends raw bytes over the TCP connection
- Does **not automatically add a newline (`\n`) or delimiter**
  - If your protocol relies on newline-separated messages, you must manually include `\n` in the `message`

------

##### 🧩 `return result != SOCKET_ERROR;`

- Evaluates whether the send operation was successful.
- If `send()` returns `SOCKET_ERROR` (usually `-1`), it indicates failure:
  - Possible causes:
    - Disconnected socket
    - Server not responding
    - Socket not initialized properly
- Returning `false` allows the calling code to detect that the message wasn’t delivered

### Receiving a Message from the Server over TCP”

```c++
std::string Client::receive() {
    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) return "";
    return std::string(buffer, bytesReceived);
}
```

------

### ✅ **Purpose**

This method waits for and receives a message from the server over the established TCP connection, storing the data in a buffer and converting it to a `std::string`.

------

### 🔍 **Line-by-Line Explanation**

#### 🧩 `char buffer[1024] = {};`

- Declares a fixed-size buffer (1024 bytes) to hold the incoming message.
- The `{}` zero-initializes the buffer, which helps prevent stray characters when converting to a string.

------

#### 🧩 `int bytesReceived = recv(...)`

```c++
recv(clientSocket, buffer, sizeof(buffer), 0);
```

- Uses the Winsock `recv()` function to read data from the socket.
- Parameters:
  - `clientSocket`: the active TCP connection to the server
  - `buffer`: where the received data will be stored
  - `sizeof(buffer)`: how many bytes to read (max 1024 here)
  - `0`: flags (0 = default, blocking mode)
- Return value:
  - **`> 0`**: Number of bytes actually received
  - **`0`**: The connection has been closed by the server (graceful shutdown)
  - **`< 0`**: An error occurred (e.g., broken pipe, network failure)

------

#### 🧩 `if (bytesReceived <= 0) return "";`

- Handles connection errors or shutdown:
  - `0` → server disconnected
  - `< 0` → error (e.g., socket closed unexpectedly)
- Returns an **empty string** to indicate failure or disconnect.
- This allows the game loop to detect that the connection was lost.

------

#### 🧩 `return std::string(buffer, bytesReceived);`

- Constructs a new `std::string` using the exact number of bytes received.
- Prevents inclusion of garbage data (because the buffer might contain trailing nulls beyond `bytesReceived`).
- This converts the raw bytes into a safe, C++-friendly format for further processing.

------

### ⚠️ Notes

- TCP is **stream-based**, not message-based — so:
  - If the server sends multiple messages back-to-back, they may arrive **combined in one `recv()`**
  - If a message is large, it may arrive **split across multiple `recv()` calls**
  - For production-ready protocols, you should add message delimiters (like `\n`) and buffer accumulation logic