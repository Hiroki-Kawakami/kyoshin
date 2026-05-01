#pragma once
#include <string>

class NetworkManager {
public:
    void init() {}
    bool isConnected() {
        if (count_ > 20) return true;
        count_++;
        return false;
    }
    std::string getWiFiSSID() {
        return "AP_SSID";
    }

private:
    int count_{0};
};

extern NetworkManager network_manager;
