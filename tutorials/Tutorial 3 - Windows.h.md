## Standard Console Colors in Windows (`<windows.h>`)

`<windows.h>` is a comprehensive Windows-specific header file in C and C++ programming that provides direct access to the **Windows API (Application Programming Interface)**. It contains declarations for **functions, macros, data types, structures, and constants** used to interact with the Windows operating system at a low level.

This header is essential for writing **native Windows applications**, **console utilities**, and performing **system-level tasks** such as window creation, process control, memory management, file I/O, and console manipulation.

#### Common Functional Areas Provided by `<windows.h>`

| Area                   | Example Function(s)             | Description                                        |
| ---------------------- | ------------------------------- | -------------------------------------------------- |
| Console I/O            | `SetConsoleTextAttribute`       | Modify text and background color in the console    |
| Window Management      | `CreateWindow`, `ShowWindow`    | Create and manage GUI windows                      |
| File System Access     | `CreateFile`, `ReadFile`        | Handle file and device I/O                         |
| Memory Management      | `VirtualAlloc`, `GlobalAlloc`   | Allocate and manage memory manually                |
| Process/Thread Control | `CreateProcess`, `Sleep`        | Launch new processes or manage threads             |
| Registry Access        | `RegOpenKeyEx`, `RegSetValue`   | Read from or write to the Windows Registry         |
| Message Loop           | `GetMessage`, `DispatchMessage` | Handle event-driven Windows GUI message processing |
| System Info            | `GetSystemTime`, `GetVersionEx` | Retrieve system-level information                  |

#### Typical Usage Example

```c++
#include <windows.h>
#include <iostream>

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 10); // Light Green text
    std::cout << "Hello from Windows Console!" << std::endl;
    SetConsoleTextAttribute(hConsole, 7);  // Reset to default
    return 0;
}
```

#### **Important Notes**

- `<windows.h>` is **Windows-only** and not portable to Linux or macOS.
- Including it may **increase compilation time**, as it pulls in many sub-headers.
- Use `#define WIN32_LEAN_AND_MEAN` before including it to reduce unnecessary components:

```c++
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
```

#### Sub-Headers Included (Examples)

| Header                | Purpose                           |
| --------------------- | --------------------------------- |
| `winuser.h`           | GUI components (buttons, windows) |
| `winbase.h`           | Basic system services             |
| `wingdi.h`            | Graphics Device Interface         |
| `wincon.h`            | Console input/output              |
| `processthreadsapi.h` | Process/thread management         |

#### When to Use `<windows.h>`

Use this header when you are:

- Developing **Windows desktop applications**
- Accessing **native OS-level features**
- Customizing **console appearance and behavior**
- Working with the **Windows registry, threads, or events**



### ConsoleColor.h used in the Game

```c++
#ifndef CONSOLECOLOR_H
#define CONSOLECOLOR_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>

enum class ConsoleColor {
    Black = 0,
    Blue = 1,
    Green = 2,
    Aqua = 3,
    Red = 4,
    Purple = 5,
    Yellow = 6,
    White = 7,
    Gray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightAqua = 11,
    LightRed = 12,
    LightPurple = 13,
    LightYellow = 14,
    BrightWhite = 15
};

inline void setColor(ConsoleColor color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<int>(color));
}

inline void printColor(const std::string& text, ConsoleColor color) {
    setColor(color);
    std::cout << text;
    setColor(ConsoleColor::White);  // Reset to default
}

inline void printColorLine(const std::string& text, ConsoleColor color) {
    setColor(color);
    std::cout << text << std::endl;
    setColor(ConsoleColor::White);
}

#endif // CONSOLECOLOR_H

```

This file isn’t a typical class — it’s a **header-only utility** that provides:

- An `enum class` to represent color codes
- Inline functions to:
  - Set console text color
  - Print colored strings

1. **Enum Class – `ConsoleColor`**

```c++
enum class ConsoleColor {
    Black = 0,
    Blue = 1,
    Green = 2,
    Aqua = 3,
    Red = 4,
    Purple = 5,
    Yellow = 6,
    White = 7,
    Gray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightAqua = 11,
    LightRed = 12,
    LightPurple = 13,
    LightYellow = 14,
    BrightWhite = 15
};
```

#### Purpose:

- Associates readable names (`LightRed`, `Aqua`, etc.) with numeric color codes used by **Windows console**.
- Avoids magic numbers in the code like `SetConsoleTextAttribute(..., 12)`

2. ##### Setting Console Text Color

```c++
inline void setColor(ConsoleColor color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<int>(color));
}
```

- Uses the Windows API function `SetConsoleTextAttribute`.
- Changes the **foreground color** of text output.
- `STD_OUTPUT_HANDLE` is the standard output stream (your console window)

3. ##### Printing Colored Text

```c++
inline void printColor(const std::string& text, ConsoleColor color) {
    setColor(color);
    std::cout << text;
    setColor(ConsoleColor::White);  // Reset to default
}
```

```c++
inline void printColorLine(const std::string& text, ConsoleColor color) {
    setColor(color);
    std::cout << text << std::endl;
    setColor(ConsoleColor::White);
}
```

####  Explanation:

- These functions **temporarily set a color**, print the string, then **reset to white**.
- `printColor()` → inline output (no newline)
- `printColorLine()` → appends `endl` for newline

#### Usage in Game Code

In `GameManager.cpp`, you’ll see:

```c++
printColorLine("[INFO] Welcome to Dice Duelers over LAN!", ConsoleColor::Aqua);
printColor("[You] Enter your stake: ", ConsoleColor::Yellow);
printColorLine("Invalid stake. Try again.", ConsoleColor::LightRed);
```

This makes it immediately obvious whether something is:

- A **prompt**
- A **warning**
- A **game result**
- A **debug log**

####  Why It’s Smart

- Improves UX in a console-only game
- Keeps message formatting **separate from logic**
- Allows **consistent styling** (e.g., always print errors in `LightRed`)

#### Summary

| Component          | Role                                 |
| ------------------ | ------------------------------------ |
| `ConsoleColor`     | Enum for all console text colors     |
| `setColor()`       | Changes text color using Windows API |
| `printColor()`     | Prints colored text without newline  |
| `printColorLine()` | Prints colored text with newline     |