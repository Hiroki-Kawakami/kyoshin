#include "kyoshin_monitor.hpp"

void KyoshinMonitor::sync() {
    auto response = http_client_.get(KYOSHIN_SERVER_CONFIG.latestUrl);
    if (response.ok()) {
        printf("Get latest.json success!\n");
        printf("%s\n\n", response.as_string().c_str());
    } else {
        printf("Get latest.json failed!\n");
    }
}
