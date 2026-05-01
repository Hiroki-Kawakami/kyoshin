#include "kyoshin_monitor.hpp"
#include <cstdio>

KyoshinMonitor::KyoshinMonitor() {
    for (int i = 0; i < image_buffers_.size(); i++) {
        image_buffers_[i] = (uint16_t*)malloc(sizeof(uint16_t) * KYOSHIN_SERVER_CONFIG.imgWidth * KYOSHIN_SERVER_CONFIG.imgHeight);
        printf("image_buffers_[%d]: %p\n", i, image_buffers_[i]);
    }
    timer_ = kyoshin_port_timer_create("kyoshin", [this](){
        this->event_group_.setBits(KyoshinMonitorEvent::Update);
    });
    kyoshin_port_task_create("kyoshin1", [this](){
        while (!event_group_.contains(KyoshinMonitorEvent::Worker1End)) {
            this->worker1();
        }
    }, 2, 16 * 1024, 0);
    kyoshin_port_task_create("kyoshin2", [this](){
        while (!event_group_.contains(KyoshinMonitorEvent::Worker2End)) {
            this->worker2();
        }
    }, 2, 16 * 1024, 1);
}

KyoshinMonitor::~KyoshinMonitor() {
    kyoshin_port_timer_delete(timer_);
}

void KyoshinMonitor::startUpdateTimer() {
    kyoshin_port_timer_start_periodic(timer_, 1000);
}
void KyoshinMonitor::stopUpdateTimer() {
    kyoshin_port_timer_stop(timer_);
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

time_t KyoshinMonitor::updateLatestTime() {
    if (latest_time_ < 0) {
        latest_time_ = getLatestTime();
    } else {
        latest_time_++;
    }
    return latest_time_;
}

static std::string strftime(time_t time, const char *format) {
    char buf[128];
    struct tm timeinfo;
    localtime_r(&time, &timeinfo);
    strftime(buf, sizeof(buf), format, &timeinfo);
    return std::string(buf);
}

bool KyoshinMonitor::downloadRealtimeImage(time_t time) {
    auto path = strftime(time, realtimeImgUrlFormat());
    auto response = http_client_.get(path);
    if (!response.ok()) return false;
    realtime_img_gif_ = std::move(response.data);
    event_group_.setBits(KyoshinMonitorEvent::RealtimeImageDownloaded);
    return true;
}

void KyoshinMonitor::decodeRealtimeImage() {
    if (!realtime_img_gif_.has_value()) {
        printf("KyoshinMonitor::decodeRealtimeImage: !realtime_img_gif_.has_value()\n");
        event_group_.setBits(KyoshinMonitorEvent::Error);
        return;
    }
    auto err = gif_decoder_.decode(realtime_img_gif_->data(), realtime_img_gif_->size(), imageBuffer());
    if (err == GifDecoder::Error::None) {
        event_group_.setBits(KyoshinMonitorEvent::ImageRendered);
    } else {
        printf("Gif decode failed: %d\n", static_cast<int>(err));
        event_group_.setBits(KyoshinMonitorEvent::Error);
    }
}

void KyoshinMonitor::worker1() {
    const auto mask1 = KyoshinMonitorEvent::Update | KyoshinMonitorEvent::Worker1Stop;
    auto event = event_group_.waitBits(mask1) & mask1;
    if (event == KyoshinMonitorEvent::Worker1Stop) {
        event_group_.setBits(KyoshinMonitorEvent::Worker1End);
        return;
    }

    auto time = updateLatestTime();
    if (time < 0) goto end;

    for (int i = 0; i < KYOSHIN_SERVER_CONFIG.imgWidth * KYOSHIN_SERVER_CONFIG.imgHeight; i++) {
        imageBuffer()[i] = 0xffff;
    }
    if (!downloadRealtimeImage(time)) goto end;

    event_group_.waitBitsAndClear(KyoshinMonitorEvent::ImageRendered);
    if (callback_) callback_->onData(imageBuffer());
    image_buffer_idx_ = (image_buffer_idx_ + 1) % image_buffers_.size();
end:
    event_group_.clearBits(KyoshinMonitorEvent::Update);
}
void KyoshinMonitor::worker2() {
    const auto mask2 = KyoshinMonitorEvent::RealtimeImageDownloaded | KyoshinMonitorEvent::Worker2Stop;
    auto event = event_group_.waitBitsAndClear(mask2) & mask2;
    switch (event) {
    case KyoshinMonitorEvent::RealtimeImageDownloaded:
        decodeRealtimeImage();
        break;
    case KyoshinMonitorEvent::Worker2Stop:
        event_group_.setBits(KyoshinMonitorEvent::Worker2End);
        break;
    default:
        printf("FATAL ERROR: Invalid event in worker2\n");
        event_group_.setBits(KyoshinMonitorEvent::Worker2End);
        break;
    }
}
