#pragma once
#include <string>
#include <functional>

class NetworkManager {
public:
    enum class Result {
        Ok,
        ApNotFound,
        AuthFailed,
        AssocFailed,
        IpFailed,
        Failed,
    };

    void init() {}
    bool isConfigured() { return true; }
    void connect(std::function<void(Result)> callback) { callback(Result::Ok); }
    void connect(std::string /*ssid*/, std::string /*passwd*/, std::function<void(Result)> callback) { callback(Result::Ok); }
    bool isConnected() { return true; }
    std::string getWiFiSSID() { return "AP_SSID"; }
};

extern NetworkManager network_manager;
