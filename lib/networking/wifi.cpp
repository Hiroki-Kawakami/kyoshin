/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include "networking.hpp"
#include "rtos.hpp"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif_sntp.h"

NETWORKING_IMPL_BEGIN

static const char *TAG = "wifi";

enum class NetworkEventBits {
    WiFiStart        = 1 << 0,
    WiFiConnecting   = 1 << 1,
    WiFiConnectEnd   = 1 << 2,
    WiFiConnected    = 1 << 3,
    NetworkConnected = 1 << 4,
    SntpSynced       = 1 << 5,
};
static RTOS::EventGroup<NetworkEventBits> event_group;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        event_group.setBits(NetworkEventBits::WiFiStart);
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
        event_group.setBits(NetworkEventBits::WiFiConnected);
        event_group.setBits(NetworkEventBits::WiFiConnectEnd);
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        event_group.clearBits(NetworkEventBits::NetworkConnected);
        event_group.clearBits(NetworkEventBits::WiFiConnected);
        event_group.setBits(NetworkEventBits::WiFiConnectEnd);
        break;
    default:
        break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
    case IP_EVENT_STA_GOT_IP: {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        event_group.setBits(NetworkEventBits::NetworkConnected);
        break;
    }
    default:
        break;
    }
}

void init() {
    ESP_LOGI(TAG, "Networking init...");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
}

bool configured() {
    wifi_config_t config = {};
    esp_err_t ret = esp_wifi_get_config(WIFI_IF_STA, &config);
    return ret == ESP_OK && config.sta.ssid[0] != '\0';
}

string ssid() {
    wifi_config_t config = {};
    esp_wifi_get_config(WIFI_IF_STA, &config);
    return string((char *)config.sta.ssid);
}

void waitStationStart() {
    event_group.waitBits(NetworkEventBits::WiFiStart);
}

void setWiFiConfig(wifi_config_t *config) {
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, config));
}

bool connect() {
    if (!event_group.contains(NetworkEventBits::WiFiStart)) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
        waitStationStart();
    }
    event_group.clearBits(NetworkEventBits::WiFiConnectEnd);
    event_group.setBits(NetworkEventBits::WiFiConnecting);
    ESP_ERROR_CHECK(esp_wifi_connect());
    event_group.waitBits(NetworkEventBits::WiFiConnectEnd);
    event_group.clearBits(NetworkEventBits::WiFiConnecting);
    return event_group.contains(NetworkEventBits::WiFiConnected);
}

bool isWiFiConnecting() {
    return event_group.contains(NetworkEventBits::WiFiConnecting);
}

bool isWiFiConnected() {
    return event_group.contains(NetworkEventBits::WiFiConnected);
}

bool isNetworkConnected() {
    return event_group.contains(NetworkEventBits::NetworkConnected);
}

void waitNetworkConnect() {
    event_group.waitBits(NetworkEventBits::NetworkConnected);
}

bool startSntp() {
    // timeval tv = { 1708118600 , 0 };
    // timezone tz = { 0 , 0 };
    // settimeofday(&tv, &tz);
    // setenv("TZ", "JST-9", 1);
    // tzset();
    // return true;

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("ntp.nict.jp");
    ESP_ERROR_CHECK(esp_netif_sntp_init(&config));

    int retry = 0;
    while (true) {
        esp_err_t err = esp_netif_sntp_sync_wait(5000 / portTICK_PERIOD_MS);
        if (err == ESP_OK) {
            char strftime_buf[64];
            time_t now;
            struct tm timeinfo;
            time(&now);
            setenv("TZ", "JST-9", 1);
            tzset();
            localtime_r(&now, &timeinfo);
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);
            return true;
        }
        if (retry < 5 && err == ESP_ERR_TIMEOUT) {
            ESP_LOGE(TAG, "Sync NTP Server Timeout. Retry.");
            retry++;
            continue;
        }
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return false;
    }
}


NETWORKING_IMPL_END
