#pragma once
#include "screen_manager.hpp"
#include "network_manager.hpp"
#include <string>
#include <vector>

class WiFiSetupScreen : public Screen {
public:
    virtual void build();
    virtual void onAppear();

private:
    lv_obj_t *content_;

    void startScan();
    void showScanning();
    void showAPList(const std::vector<NetworkManager::WiFiAP> &aps);
    void showPasswordDialog(std::string ssid);
    void connectToAP(std::string ssid, std::string password);
    void showConnecting(const std::string &ssid);
    void handleConnectResult(NetworkManager::Result result, const std::string &ssid);
    void showError(const std::string &ssid, NetworkManager::Result result);
};
