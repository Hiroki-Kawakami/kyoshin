#include "network_manager.hpp"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <atomic>

static const char* TAG = "NetworkManager";

NetworkManager network_manager;

static std::function<void(NetworkManager::Result)> s_callback;
static std::atomic<bool> s_connected{false};
static std::atomic<bool> s_ap_connected{false}; // AP接続済みだがIP未取得の状態を追跡

static NetworkManager::Result disconnect_reason_to_result(uint8_t reason, bool ap_was_connected) {
    if (ap_was_connected) {
        return NetworkManager::Result::IpFailed;
    }
    switch (reason) {
    case WIFI_REASON_NO_AP_FOUND:
        return NetworkManager::Result::ApNotFound;
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_AUTH_EXPIRE:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
        return NetworkManager::Result::AuthFailed;
    case WIFI_REASON_ASSOC_FAIL:
    case WIFI_REASON_CONNECTION_FAIL:
        return NetworkManager::Result::AssocFailed;
    default:
        return NetworkManager::Result::Failed;
    }
}

static void wifi_event_handler(void*, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_CONNECTED) {
            s_ap_connected = true;
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            bool was_ap_connected = s_ap_connected.exchange(false);
            s_connected = false;
            if (s_callback) {
                auto* info = static_cast<wifi_event_sta_disconnected_t*>(event_data);
                auto result = disconnect_reason_to_result(info->reason, was_ap_connected);
                ESP_LOGW(TAG, "Disconnected: reason=%d -> %d", info->reason, (int)result);
                auto cb = std::move(s_callback);
                s_callback = nullptr;
                cb(result);
            }
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_ap_connected = false;
        s_connected = true;
        if (s_callback) {
            auto cb = std::move(s_callback);
            s_callback = nullptr;
            cb(NetworkManager::Result::Ok);
        }
    }
}

void NetworkManager::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, nullptr, nullptr));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

bool NetworkManager::isConfigured() {
    wifi_config_t cfg = {};
    if (esp_wifi_get_config(WIFI_IF_STA, &cfg) != ESP_OK) return false;
    return cfg.sta.ssid[0] != '\0';
}

void NetworkManager::connect(std::function<void(Result)> callback) {
    s_ap_connected = false;
    s_callback = std::move(callback);
    if (esp_wifi_connect() != ESP_OK) {
        auto cb = std::move(s_callback);
        s_callback = nullptr;
        cb(Result::Failed);
    }
}

void NetworkManager::connect(std::string ssid, std::string passwd, std::function<void(Result)> callback) {
    wifi_config_t cfg = {};
    auto& sta = cfg.sta;
    ssid.copy(reinterpret_cast<char*>(sta.ssid), sizeof(sta.ssid) - 1);
    passwd.copy(reinterpret_cast<char*>(sta.password), sizeof(sta.password) - 1);

    if (esp_wifi_set_config(WIFI_IF_STA, &cfg) != ESP_OK) {
        callback(Result::Failed);
        return;
    }
    connect(std::move(callback));
}

bool NetworkManager::isConnected() {
    return s_connected;
}

std::string NetworkManager::getWiFiSSID() {
    wifi_config_t cfg = {};
    if (esp_wifi_get_config(WIFI_IF_STA, &cfg) != ESP_OK) return "";
    if (cfg.sta.ssid[0] == '\0') return "";
    return std::string(reinterpret_cast<const char*>(cfg.sta.ssid));
}
