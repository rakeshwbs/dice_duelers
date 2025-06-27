#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class Server {
private:
    WSADATA wsaData;
    SOCKET listenSocket;
    SOCKET clientSocket;
    sockaddr_in serverAddr;

public:
    Server();
    ~Server();

    bool initialize(int port);
    bool acceptClient();
    std::string receive();
    bool sendMessage(const std::string& message);
    void close();
};

#endif // SERVER_H
