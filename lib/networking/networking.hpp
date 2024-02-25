/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#define NETWORKING_IMPL_BEGIN namespace Networking {
#define NETWORKING_IMPL_END }

#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <optional>
#include <string>
#include <memory>
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "types.hpp"

NETWORKING_IMPL_BEGIN

using std::function;
using std::optional;
using std::string;

void init();
bool configured();
string ssid();
void waitStationStart();
void getWiFiConfigWithDPP(function<void(const char *)> uriReady, function<void(wifi_config_t *, const char *)> done);
void setWiFiConfig(wifi_config_t *config);
void connect();
bool isConnected();
void waitNetworkConnect();
bool startSntp();

class HTTPClient : private NoMove {
private:
    esp_http_client_handle_t client = nullptr;
    void init(string url);
public:
    int received = 0;
    uint8_t *buffer;
    int bufferSize = 0;
    bool bufferOverflow = false;
    bool shouldReleaseBuffer = false;
    function<void(uint8_t*, int, int)> onData;
    HTTPClient(function<void(uint8_t*, int, int)> onData) : onData(onData) {}
    HTTPClient(uint8_t *buffer, int size) : buffer(buffer), bufferSize(size) {};
    HTTPClient(int bufferSize) : bufferSize(bufferSize) {};
    ~HTTPClient();
    int statusCode();
    void clearBuffer();
    bool get(string url, int redirect = 0);
};

NETWORKING_IMPL_END
