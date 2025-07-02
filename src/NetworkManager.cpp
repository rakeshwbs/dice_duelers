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
