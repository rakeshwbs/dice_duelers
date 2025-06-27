#include "Server.h"
#include <iostream>

Server::Server() {
    listenSocket = INVALID_SOCKET;
    clientSocket = INVALID_SOCKET;
}

Server::~Server() {
    close();
}

bool Server::initialize(int port) {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return false;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        return false;
    }

    std::cout << "Server initialized. Waiting for client...\n";
    return true;
}

bool Server::acceptClient() {
    clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Client connection failed.\n";
        return false;
    }
    std::cout << "Client connected!\n";
    return true;
}

std::string Server::receive() {
    char buffer[1024] = {};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) return "";
    return std::string(buffer, bytesReceived);
}

bool Server::sendMessage(const std::string& message) {
    int result = send(clientSocket, message.c_str(), message.length(), 0);
    return result != SOCKET_ERROR;
}

void Server::close() {
    if (clientSocket != INVALID_SOCKET) closesocket(clientSocket);
    if (listenSocket != INVALID_SOCKET) closesocket(listenSocket);
    WSACleanup();
}
