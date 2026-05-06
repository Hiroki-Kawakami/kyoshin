#pragma once
#include <string>
#include <functional>
#include <vector>

class NetworkManager {
public:
    enum class Result {
        Ok,
        ApNotFound,   // SSIDが見つからない
        AuthFailed,   // パスワード不正 / 認証失敗
        AssocFailed,  // アソシエーション失敗
        IpFailed,     // AP接続後にIPアドレス取得失敗
        Failed,       // その他の失敗
    };

    struct WiFiAP {
        std::string ssid;
        int8_t rssi;
        bool secured;
    };

    void init();
    bool isConfigured();
    std::string getWiFiSSID();
    void connect(std::function<void(Result)> callback);
    void connect(std::string ssid, std::string passwd, std::function<void(Result)> callback);
    bool isConnected();
    void scanAPs(std::function<void(std::vector<WiFiAP>)> callback);
};

extern NetworkManager network_manager;
