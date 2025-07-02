// NetworkManager.h
#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>

class NetworkManager {
public:
    bool host(int port);
    bool connectToHost(const std::string& ip, int port);
    bool sendMessage(const std::string& message);
    std::string receiveMessage();
    void closeConnection();
};

#endif
