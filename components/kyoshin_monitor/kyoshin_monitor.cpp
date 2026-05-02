#include <cstdio>
#include "kyoshin_monitor.hpp"
#include "json_parser.hpp"

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
            event_group_.clearBits(KyoshinMonitorEvent::Update | KyoshinMonitorEvent::Error);
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

    auto json = JSONValue(response.as_string());
    if (json.isNull()) {
        printf("JSON parse failed!\n");
        return -1;
    }

    time_t result = -1;
    auto latest_time = json["latest_time"];
    if (latest_time.isString()) {
        struct tm t = {};
        if (sscanf(latest_time.stringValue().c_str(), "%d/%d/%d %d:%d:%d",
                   &t.tm_year, &t.tm_mon, &t.tm_mday,
                   &t.tm_hour, &t.tm_min, &t.tm_sec) == 6) {
            t.tm_year -= 1900;
            t.tm_mon -= 1;
            t.tm_isdst = -1;
            result = mktime(&t);
        }
    }
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

std::string KyoshinMonitor::downloadForecast(time_t time) {
    auto path = strftime(time, KYOSHIN_SERVER_CONFIG.forecastUrlFormat);
    auto response = http_client_.get(path);
    if (!response.ok()) return "";
    return response.as_string();
}

bool KyoshinMonitor::downloadRealtimeImage(time_t time) {
    auto path = strftime(time, realtimeImgUrlFormat());
    auto response = http_client_.get(path);
    if (!response.ok()) return false;
    realtime_img_gif_ = std::move(response.data);
    event_group_.setBits(KyoshinMonitorEvent::RealtimeImageDownloaded);
    return true;
}

bool KyoshinMonitor::downloadPsWaveImage(time_t time) {
    auto path = strftime(time, regionConfig().psWaveUrlFormat);
    auto response = http_client_.get(path);
    if (response.ok()) {
        pswave_img_gif_ = std::move(response.data);
        event_group_.setBits(KyoshinMonitorEvent::PsWaveImageDownloaded);
        return true;
    } else {
        event_group_.setBits(KyoshinMonitorEvent::PsWaveImageSkip);
        return false;
    }
}

void KyoshinMonitor::decodeGifImage(uint8_t *data, size_t size) {
    auto err = gif_decoder_.decode(data, size, imageBuffer());
    if (err == GifDecoder::Error::SizeMismatch) {
        return;
    } else if (err != GifDecoder::Error::None) {
        printf("Gif decode failed: %d\n", static_cast<int>(err));
        event_group_.setBits(KyoshinMonitorEvent::Error);
    }
}

void KyoshinMonitor::worker1() {
    auto event = event_group_.waitBits(KyoshinMonitorEvent::Update | KyoshinMonitorEvent::Worker1Stop);
    if (event & KyoshinMonitorEvent::Worker1Stop) {
        event_group_.setBits(KyoshinMonitorEvent::Worker1End);
        return;
    }

    auto time = updateLatestTime();
    if (time < 0) return;

    auto forecast_json = downloadForecast(time);
    if (forecast_json.empty()) return;
    forecast_.update(forecast_json.c_str());

    for (int i = 0; i < KYOSHIN_SERVER_CONFIG.imgWidth * KYOSHIN_SERVER_CONFIG.imgHeight; i++) {
        imageBuffer()[i] = 0xffff;
    }
    if (downloadRealtimeImage(time)) {
        downloadPsWaveImage(time);
    }

    event = event_group_.waitBits(KyoshinMonitorEvent::ImageRendered | KyoshinMonitorEvent::Error);
    if (event & KyoshinMonitorEvent::Error) return;
    event_group_.clearBits(KyoshinMonitorEvent::ImageRendered);
    if (callback_) callback_->onData(imageBuffer());
    image_buffer_idx_ = (image_buffer_idx_ + 1) % image_buffers_.size();
}
void KyoshinMonitor::worker2() {
    auto event = event_group_.waitBits(
        KyoshinMonitorEvent::RealtimeImageDownloaded |
        KyoshinMonitorEvent::PsWaveImageDownloaded |
        KyoshinMonitorEvent::PsWaveImageSkip |
        KyoshinMonitorEvent::Worker2Stop);
    if (event & KyoshinMonitorEvent::RealtimeImageDownloaded) {
        if (realtime_img_gif_.has_value()) {
            decodeGifImage(realtime_img_gif_->data(), realtime_img_gif_->size());
        } else {
            printf("realtime_img_gif_ is empty!\n");
            event_group_.setBits(KyoshinMonitorEvent::Error);
        }
        event_group_.clearBits(KyoshinMonitorEvent::RealtimeImageDownloaded);
        return;
    }
    if (event & KyoshinMonitorEvent::PsWaveImageDownloaded) {
        if (pswave_img_gif_.has_value()) {
            decodeGifImage(pswave_img_gif_->data(), pswave_img_gif_->size());
        } else {
            printf("pswave_img_gif_ is empty!\n");
            event_group_.setBits(KyoshinMonitorEvent::Error);
        }
        event_group_.clearBits(KyoshinMonitorEvent::PsWaveImageDownloaded);
        event_group_.setBits(KyoshinMonitorEvent::ImageRendered);
        return;
    }
    if (event & KyoshinMonitorEvent::PsWaveImageSkip) {
        event_group_.clearBits(KyoshinMonitorEvent::PsWaveImageSkip);
        event_group_.setBits(KyoshinMonitorEvent::ImageRendered);
        return;
    }
    if (event & KyoshinMonitorEvent::Worker2Stop) {
        event_group_.setBits(KyoshinMonitorEvent::Worker2End);
        return;
    }
}
