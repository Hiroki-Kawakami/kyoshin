#include "network_manager.hpp"
#include "nvs.hpp"
#include <cstdio>
#include <thread>
#include <chrono>

static const char* TAG = "NetworkManager";
static const char* NVS_NAMESPACE = "wifi";
static const char* KEY_SSID = "ssid";
static const char* KEY_PASS = "pass";

NetworkManager network_manager;

static bool s_connected = false;
static std::string s_ssid = "";

void NetworkManager::init() {
    printf("[%s] init\n", TAG);
    NVS nvs(NVS_NAMESPACE);
    size_t length = 0;
    if (nvs.get(KEY_SSID, (void*)nullptr, &length) == NVS::Error::OK) {
        char buffer[64] = {0};
        if (nvs.get(KEY_SSID, buffer, &length) == NVS::Error::OK) {
            s_ssid = buffer;
            printf("[%s] loaded ssid from nvs: %s\n", TAG, s_ssid.c_str());
        }
    }
}

bool NetworkManager::isConfigured() {
    return !s_ssid.empty();
}

std::string NetworkManager::getWiFiSSID() {
    return s_ssid;
}

void NetworkManager::connect(std::function<void(Result)> callback) {
    printf("[%s] connect to %s\n", TAG, s_ssid.c_str());

    std::thread([this, callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (s_ssid.empty()) {
            if (callback) callback(Result::Failed);
            return;
        }
        s_connected = true;
        if (callback) callback(Result::Ok);
    }).detach();
}

void NetworkManager::connect(std::string ssid, std::string passwd, std::function<void(Result)> callback) {
    printf("[%s] connect to %s (password: %s)\n", TAG, ssid.c_str(), passwd.c_str());

    std::thread([this, ssid, passwd, callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (ssid == "fail_auth") {
            s_connected = false;
            if (callback) callback(Result::AuthFailed);
        } else if (ssid == "fail_not_found") {
            s_connected = false;
            if (callback) callback(Result::ApNotFound);
        } else {
            s_ssid = ssid;
            s_connected = true;

            // Save to NVS
            NVS nvs(NVS_NAMESPACE);
            nvs.set(KEY_SSID, s_ssid.c_str());
            nvs.set(KEY_PASS, passwd.c_str());
            nvs.commit();

            callback(Result::Ok);
        }
    }).detach();
}

bool NetworkManager::isConnected() {
    return s_connected;
}

void NetworkManager::scanAPs(std::function<void(std::vector<WiFiAP>)> callback) {
    printf("[%s] scanAPs\n", TAG);

    std::thread([callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        std::vector<WiFiAP> aps = {
            {"Mock AP 1", -30, true},
            {"Mock AP 2", -60, true},
            {"Mock AP 3", -80, false},
            {"fail_auth", -40, true},
            {"fail_not_found", -50, true},
        };
        callback(aps);
    }).detach();
}
