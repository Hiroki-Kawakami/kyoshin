#include "kyoshin_monitor.hpp"
#include <cstdio>

KyoshinMonitor::KyoshinMonitor() {
    timer_ = kyoshin_port_timer_create("kyoshin", [this](){
        this->event_group_.setBits(KyoshinMonitorEvent::Update);
    });
    kyoshin_port_task_create("kyoshin1", [this](){
        while (true) {
            if (event_group_.contains(KyoshinMonitorEvent::Worker1End)) return;
            this->worker1();
        }
    }, 2, 16 * 1024, 0);
    kyoshin_port_task_create("kyoshin2", [this](){
        while (true) {
            if (event_group_.contains(KyoshinMonitorEvent::Worker2End)) return;
            this->worker2();
        }
    }, 2, 16 * 1024, 1);
}

KyoshinMonitor::~KyoshinMonitor() {
    kyoshin_port_timer_delete(timer_);
}

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

void KyoshinMonitor::startUpdateTimer() {
    kyoshin_port_timer_start_periodic(timer_, 1000);
}
void KyoshinMonitor::stopUpdateTimer() {
    kyoshin_port_timer_stop(timer_);
}

void KyoshinMonitor::worker1() {
    auto event = event_group_.waitBits(KyoshinMonitorEvent::Update | KyoshinMonitorEvent::Worker1Stop);
    if (event == KyoshinMonitorEvent::Worker1Stop) {
        event_group_.setBits(KyoshinMonitorEvent::Worker1End);
        return;
    }

    if (latest_time_ < 0) {
        latest_time_ = getLatestTime();
    } else {
        latest_time_++;
    }
    printf("Update: %s", ctime(&latest_time_));
    event_group_.clearBits(KyoshinMonitorEvent::Update);
}
void KyoshinMonitor::worker2() {
    auto event = event_group_.waitBits(KyoshinMonitorEvent::Worker2Stop);
    if (event == KyoshinMonitorEvent::Worker2Stop) {
        event_group_.setBits(KyoshinMonitorEvent::Worker2End);
        return;
    }
}
