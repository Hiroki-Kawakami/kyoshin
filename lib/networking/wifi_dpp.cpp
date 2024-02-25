/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include <cstring>
#include "esp_log.h"
#include "esp_dpp.h"
#include "networking.hpp"

NETWORKING_IMPL_BEGIN

static const char *TAG = "wifi_dpp";

static struct {
    optional<function<void(const char *)>> uriReady;
    optional<function<void(wifi_config_t *, const char *)>> done;
} callbacks;

static void clearCallbacks() {
    callbacks.uriReady.reset();
    callbacks.done.reset();
}

static void dpp_enrollee_event_cb(esp_supp_dpp_event_t event, void *data) {
    switch (event) {
    case ESP_SUPP_DPP_URI_READY:
        if (data && callbacks.uriReady.has_value()) {
            callbacks.uriReady.value()((const char *)data);
        }
        break;
    case ESP_SUPP_DPP_CFG_RECVD: {
        wifi_config_t config;
        memcpy(&config, data, sizeof(config));
        ESP_LOGI(TAG, "DPP Authentication successful : %s", config.sta.ssid);
        if (callbacks.done.has_value()) {
            callbacks.done.value()(&config, nullptr);
        }
        clearCallbacks();
        break;
    }
    case ESP_SUPP_DPP_FAIL: {
        const char *err = esp_err_to_name((int)data);
        ESP_LOGI(TAG, "DPP Auth failed (Reason: %s)", err);
        if (callbacks.done.has_value()) {
            callbacks.done.value()(nullptr, err);
        }
        clearCallbacks();
        break;
    }
    default:
        break;
    }
}

void getWiFiConfigWithDPP(function<void(const char *)> uriReady, function<void(wifi_config_t *, const char *)> done) {
    callbacks.uriReady = uriReady;
    callbacks.done = done;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_supp_dpp_init(dpp_enrollee_event_cb));
    ESP_ERROR_CHECK(esp_supp_dpp_bootstrap_gen("6", DPP_BOOTSTRAP_QR_CODE, NULL, NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    waitStationStart();

    ESP_ERROR_CHECK(esp_supp_dpp_start_listen());
    ESP_LOGI(TAG, "Started listening for DPP Authentication");
}

NETWORKING_IMPL_END
