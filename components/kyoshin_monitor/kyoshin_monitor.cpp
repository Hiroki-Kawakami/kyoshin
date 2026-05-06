#include <cstdio>
#include <cstring>
#include "kyoshin_monitor.hpp"
#include "json_parser.hpp"
#include "flash_image.hpp"

KyoshinMonitor::KyoshinMonitor() {
    for (int i = 0; i < image_buffers_.size(); i++) {
        image_buffers_[i] = (uint16_t*)malloc(sizeof(uint16_t) * KYOSHIN_SERVER_CONFIG.imgWidth * KYOSHIN_SERVER_CONFIG.imgHeight);
        printf("image_buffers_[%d]: %p\n", i, image_buffers_[i]);
    }
    timer_ = kyoshin_port_timer_create("kyoshin", [this](){
        if (latest_time_ > 0) latest_time_++;
        this->event_group_.setBits(KyoshinMonitorEvent::Update);
    });
    startWorkers();
}

KyoshinMonitor::~KyoshinMonitor() {
    kyoshin_port_timer_delete(timer_);
}

void KyoshinMonitor::startUpdateTimer() {
    kyoshin_port_timer_start_periodic(timer_, 1000);
}
void KyoshinMonitor::stopUpdateTimer() {
    kyoshin_port_timer_stop(timer_);
    latest_time_ = -1;
}

bool KyoshinMonitor::loadBaseMapImage(bool download) {
    flash_image.selectImage(getMapRegion().value);
    bool exists = flash_image.exists();
    if (!exists && download) {
        event_group_.setBits(KyoshinMonitorEvent::DownloadBaseMap);
    }
    return exists;
}

void KyoshinMonitor::setImageSource(MapRegion map_region, bool borehole, RealtimeImgType realtime_img_type, bool reload) {
    if (reload) stopUpdateTimer();
    if (reload) stopWorkers();
    map_region_ = map_region;
    borehole_ = borehole;
    realtime_img_type_ = realtime_img_type;
    if (reload) startWorkers();
}

uint16_t *KyoshinMonitor::copyBaseMapImage() {
    memcpy(imageBuffer(), flash_image.getData(), KYOSHIN_SERVER_CONFIG.imgWidth * KYOSHIN_SERVER_CONFIG.imgHeight * 2);
    return imageBuffer();
}

void KyoshinMonitor::updateImage() {
    event_group_.setBits(KyoshinMonitorEvent::UpdateImage);
}

bool KyoshinMonitor::downloadBaseMapImage() {
    auto response = http_client_.get(regionConfig().baseMapUrl);
    if (!response.ok()) return false;
    auto err = gif_decoder_.decode(response.data.data(), response.data.size(), imageBuffer());
    if (err != GifDecoder::Error::None) {
        printf("BaseMap decode failed: %d\n", static_cast<int>(err));
        return false;
    }
    return flash_image.writeImage(imageBuffer());
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
        kyoshin_port_timer_restart(timer_, 1000);
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

void KyoshinMonitor::startWorkers() {
    if (!event_group_.contains(KyoshinMonitorEvent::Worker1Active)) {
        event_group_.setBits(KyoshinMonitorEvent::Worker1Active);
        event_group_.clearBits(
            KyoshinMonitorEvent::Update |
            KyoshinMonitorEvent::UpdateImage |
            KyoshinMonitorEvent::Worker1Stop);
        kyoshin_port_task_create("kyoshin1", [this](){
            while (!event_group_.contains(KyoshinMonitorEvent::Worker1Stop)) {
                this->worker1();
            }
            event_group_.clearBits(KyoshinMonitorEvent::Worker1Active);
        }, 3, 24 * 1024, 0);
    }
    if (!event_group_.contains(KyoshinMonitorEvent::Worker2Active)) {
        event_group_.setBits(KyoshinMonitorEvent::Worker2Active);
        event_group_.clearBits(
            KyoshinMonitorEvent::RealtimeImageDownloaded |
            KyoshinMonitorEvent::PsWaveImageDownloaded |
            KyoshinMonitorEvent::PsWaveImageSkip |
            KyoshinMonitorEvent::DownloadBaseMap |
            KyoshinMonitorEvent::Worker2Stop);
        kyoshin_port_task_create("kyoshin2", [this](){
            while (!event_group_.contains(KyoshinMonitorEvent::Worker2Stop)) {
                this->worker2();
            }
            event_group_.clearBits(KyoshinMonitorEvent::Worker2Active);
        }, 3, 24 * 1024, 1);
    }
}
void KyoshinMonitor::stopWorkers() {
    event_group_.setBits(KyoshinMonitorEvent::Worker1Stop | KyoshinMonitorEvent::Worker2Stop);
    while (event_group_.contains(KyoshinMonitorEvent::Worker1Active)) usleep(20000);
    while (event_group_.contains(KyoshinMonitorEvent::Worker2Active)) usleep(20000);
}

void KyoshinMonitor::worker1() {
    auto event = event_group_.waitBits(
        KyoshinMonitorEvent::Update |
        KyoshinMonitorEvent::UpdateImage |
        KyoshinMonitorEvent::Worker1Stop);
    if (event & KyoshinMonitorEvent::Worker1Stop) return;

    auto time = updateLatestTime();
    if (time < 0) {
        http_client_.close();
        event_group_.clearBits(KyoshinMonitorEvent::Update | KyoshinMonitorEvent::UpdateImage);
        return;
    }

    auto prev_power_mode = kyoshin_port_get_power_mode();
    if (event & KyoshinMonitorEvent::Update) {
        auto forecast_json = downloadForecast(time);
        if (forecast_json.empty()) {
            http_client_.close();
            event_group_.clearBits(KyoshinMonitorEvent::Update | KyoshinMonitorEvent::UpdateImage);
            return;
        }
        forecast_.update(forecast_json.c_str());
        kyoshin_port_update_power_mode(time, forecast_);
        printf("forecast(json): %s\n", forecast_json.c_str());
        printf("forecast: %s\n", forecast_.toString().c_str());
        printf("powermode: %d\n", static_cast<int>(kyoshin_port_get_power_mode()));
    }

    if (kyoshin_port_get_power_mode() == PowerMode::Normal || prev_power_mode == PowerMode::Normal || time % 10 == 0) {
        event_group_.clearBits(KyoshinMonitorEvent::ImageRendered | KyoshinMonitorEvent::Error);
        copyBaseMapImage();
        if (downloadRealtimeImage(time) && !forecast_.empty()) {
            downloadPsWaveImage(time);
        } else {
            event_group_.setBits(KyoshinMonitorEvent::PsWaveImageSkip);
        }
        http_client_.close();

        event = event_group_.waitBits(
            KyoshinMonitorEvent::ImageRendered |
            KyoshinMonitorEvent::Error |
            KyoshinMonitorEvent::Worker1Stop);
        if (event & KyoshinMonitorEvent::Worker1Stop) return;
        if (event & KyoshinMonitorEvent::Error) {
            if (callback_) callback_->onData(time, nullptr);
        } else {
            if (callback_) callback_->onData(time, imageBuffer());
            image_buffer_idx_ = (image_buffer_idx_ + 1) % image_buffers_.size();
        }
    } else {
        http_client_.close();
        if (callback_) callback_->onData(time, nullptr);
    }
    forecast_.updateReportTime();
    event_group_.clearBits(KyoshinMonitorEvent::Update | KyoshinMonitorEvent::UpdateImage);
}
void KyoshinMonitor::worker2() {
    auto event = event_group_.waitBits(
        KyoshinMonitorEvent::RealtimeImageDownloaded |
        KyoshinMonitorEvent::PsWaveImageDownloaded |
        KyoshinMonitorEvent::PsWaveImageSkip |
        KyoshinMonitorEvent::DownloadBaseMap |
        KyoshinMonitorEvent::Worker2Stop);
    if (event & KyoshinMonitorEvent::Worker2Stop) return;
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
    if (event & KyoshinMonitorEvent::DownloadBaseMap) {
        auto result = downloadBaseMapImage();
        if (callback_) callback_->onBaseMapReady(result);
        event_group_.clearBits(KyoshinMonitorEvent::DownloadBaseMap);
        return;
    }
}
