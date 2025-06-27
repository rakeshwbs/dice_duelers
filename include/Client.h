#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class Client {
private:
    WSADATA wsaData;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

public:
    Client();
    ~Client();

    bool connectToServer(const std::string& ip, int port);
    bool sendMessage(const std::string& message);
    std::string receive();
    void close();
};

#endif // CLIENT_H
