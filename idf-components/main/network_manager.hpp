#pragma once
#include <string>

class NetworkManager {
public:
    void init();
    bool isConnected();
    std::string getWiFiSSID();
};

extern NetworkManager network_manager;
