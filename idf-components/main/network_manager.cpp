#include "network_manager.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.hpp"
#include "wifi_ap.hpp"
#include "wifi_sta.hpp"

static const char *TAG = "NetworkManager";

namespace {
    bool wifi_connected = false;
    espp::WifiSta *wifi_sta;
    espp::WifiSta::Config config{
        .ssid = CONFIG_APP_WIFI_SSID,
        .password = CONFIG_APP_WIFI_PASSWORD,
        .num_connect_retries = 3,
        .on_connected = [](){
            ESP_LOGI(TAG, "wifi connected");
        },
        .on_disconnected = [](){
            ESP_LOGI(TAG, "wifi disconnected");
            wifi_connected = false;
        },
        .on_got_ip =
            [](ip_event_got_ip_t *eventdata) {
                ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&eventdata->ip_info.ip));
                wifi_connected = true;
            },
        .log_level = espp::Logger::Verbosity::DEBUG
    };
}

void NetworkManager::init() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if ((err = nvs_flash_erase()) == ESP_OK) {
            err = nvs_flash_init();
        }
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS flash");
        return;
    }
    wifi_sta = new espp::WifiSta(config);
    esp_wifi_set_ps(WIFI_PS_NONE);
}

bool NetworkManager::isConnected() {
    return wifi_connected;
}

std::string NetworkManager::getWiFiSSID() {
    return CONFIG_APP_WIFI_SSID;
}

NetworkManager network_manager;
