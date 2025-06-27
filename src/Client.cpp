#include "Client.h"
#include <iostream>

Client::Client() {
    clientSocket = INVALID_SOCKET;
}

Client::~Client() {
    close();
}

bool Client::connectToServer(const std::string& ip, int port) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed.\n";
        return false;
    }

    std::cout << "Connected to server!\n";
    return true;
}

bool Client::sendMessage(const std::string& message) {
    int result = send(clientSocket, message.c_str(), message.length(), 0);
    return result != SOCKET_ERROR;
}

std::string Client::receive() {
    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) return "";
    return std::string(buffer, bytesReceived);
}

void Client::close() {
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
    WSACleanup();
}
