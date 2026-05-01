#pragma once
#include <time.h>
#include "kyoshin_monitor_config.hpp"
#include "http_client.hpp"

class KyoshinMonitor {
public:
    time_t getLatestTime();

private:
    HttpClient http_client_{KYOSHIN_SERVER_CONFIG.base_url};
};
