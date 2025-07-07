## **Custom Application-Level Protocol Overview**

In the project, we use a **lightweight, line-oriented, string-based protocol** built on top of **TCP sockets**. While TCP ensures reliable delivery of bytes, your application defines **how to interpret those bytes** — that’s where your **custom protocol** comes in.

------

## ✅ **Message Format**

Each message follows a simple **prefix + data** format:

```c++
<COMMAND>|<DATA>
```

### ✳️ Examples:

- `"NAME|Rakesh"` – Send player’s name
- `"GUESS|3"` – Send guessed number
- `"STAKE|20"` – Send amount of money wagered
- `"ROLL|5"` – Send result of die roll
- `"BALANCE|85"` – Send current balance
- `"EXIT|Rakesh quit the game"` – Signal game exit with reason

This format is human-readable and easy to parse with `substr()` and `find('|')`.

------

## 🧪 **How Messages Are Sent**

### Server/Client send:

```c++
net.sendMessage("GUESS|" + std::to_string(guess));
```

### Server/Client receive:

```
std::string msg = net.receiveMessage();
std::string data = msg.substr(msg.find('|') + 1);
```

You always:

- Use the part **before `|`** to identify the **type of message**
- Use the part **after `|`** as the **payload**

------

## ⚠️ Why We Need This Protocol

TCP is a **byte stream**, not message-based:

- If we send multiple messages quickly, they can arrive **combined**.
- Or a long message may arrive **in chunks**.

That’s why your protocol ensures:

- Each message is **self-contained** (with `|` as a separator)
- We can detect **what type of data** is coming (e.g., `NAME`, `ROLL`, `BALANCE`)
- We can split and route messages accordingly

------

## Optional: Adding Message Delimiters

If needed, we can expand the protocol to include a **line break** (`\n`) or special terminator:

Example:

```
"ROLL|4\n"
"EXIT|Bye!\n"
```

Then in `receiveMessage()`, we could split on `\n` to isolate complete messages.

------

##  Summary

> Our custom protocol is a **string-based message format** using `<COMMAND>|<DATA>`. It's simple, readable, and effective for turn-based gameplay over TCP. It enables the server and client to clearly distinguish between message types and payloads using a predictable format.