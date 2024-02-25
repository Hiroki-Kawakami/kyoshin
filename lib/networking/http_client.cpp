/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#include "networking.hpp"
#include "esp_log.h"
#include "esp_err.h"

NETWORKING_IMPL_BEGIN

static const char *TAG = "HTTPC";

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // printf("%s\n", (char*)evt->data);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                auto client = static_cast<HTTPClient*>(evt->user_data);
                client->onData((uint8_t*)evt->data, client->received, evt->data_len);
                client->received += evt->data_len;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}

HTTPClient::~HTTPClient() {
    if (client) {
        esp_http_client_cleanup(client);
        client = nullptr;
    }
    if (buffer && shouldReleaseBuffer) {
        delete[] buffer;
    }
}

void HTTPClient::init(string url) {
    if (client) {
        esp_http_client_set_url(client, url.c_str());
        return;
    }
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.event_handler = http_event_handler;
    config.user_data = this;
    config.timeout_ms = 1000;
    config.disable_auto_redirect = true;
    client = esp_http_client_init(&config);
    if (client == nullptr) {
        ESP_LOGE(TAG, "Failed to create esp_http_client");
        assert(0);
    }
}

int HTTPClient::statusCode() {
    if (!client) return -1;
    return esp_http_client_get_status_code(client);
}

void HTTPClient::clearBuffer() {
    if (!buffer && bufferSize) {
        buffer = new uint8_t[bufferSize];
        shouldReleaseBuffer = true;
    }

    if (buffer) {
        if (!onData) onData = [this](uint8_t* data, int offset, int length){
            if (offset + length > bufferSize - 1) {
                bufferOverflow = true;
                length = bufferSize - offset - 1;
            }
            memcpy(buffer + offset, data, length);
        };
        memset(buffer, 0, bufferSize);
    }
    received = 0;
    bufferOverflow = false;
}

bool HTTPClient::get(string url, int redirect) {
    init(url);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    while (true) {
        clearBuffer();
        esp_err_t err = esp_http_client_perform(client);
        if (err != ESP_OK) return false;
        int code = statusCode();
        if (!(redirect--) || code / 10 != 30) return true; // not redirect
        esp_http_client_set_redirection(client);
    };
    return true;
}

NETWORKING_IMPL_END
