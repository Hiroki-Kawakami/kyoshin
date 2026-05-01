#pragma once
#include "kyoshin_monitor_config.hpp"
#include "http_client.hpp"

class KyoshinMonitor {
public:
    void sync();

private:
    HttpClient http_client_{KYOSHIN_SERVER_CONFIG.base_url};
};
