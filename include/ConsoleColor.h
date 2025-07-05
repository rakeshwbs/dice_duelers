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
