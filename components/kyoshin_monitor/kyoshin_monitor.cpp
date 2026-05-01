#include "kyoshin_monitor.hpp"
#include "cJSON.h"
#include <cstdio>

time_t KyoshinMonitor::getLatestTime() {
    auto response = http_client_.get(KYOSHIN_SERVER_CONFIG.latestUrl);
    if (!response.ok()) {
        printf("Get latest.json failed!\n");
        return -1;
    }

    auto body = response.as_string();
    cJSON *json = cJSON_Parse(body.c_str());
    if (!json) {
        printf("JSON parse failed!\n");
        return -1;
    }

    time_t result = -1;
    cJSON *latest_time = cJSON_GetObjectItem(json, "latest_time");
    if (cJSON_IsString(latest_time) && latest_time->valuestring) {
        struct tm t = {};
        if (sscanf(latest_time->valuestring, "%d/%d/%d %d:%d:%d",
                   &t.tm_year, &t.tm_mon, &t.tm_mday,
                   &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
            t.tm_year -= 1900;
            t.tm_mon -= 1;
            t.tm_isdst = -1;
            result = mktime(&t);
        }
    }

    cJSON_Delete(json);
    return result;
}
