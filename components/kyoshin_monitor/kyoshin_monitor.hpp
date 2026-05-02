#pragma once
#include <optional>
#include <vector>
#include <time.h>
#include "kyoshin_monitor_config.hpp"
#include "kyoshin_forecast.hpp"
#include "kyoshin_port.hpp"
#include "http_client.hpp"
#include "gif_decoder.hpp"

enum class KyoshinMonitorEvent : uint32_t {
    Update                  = 1 << 0,
    StartImageRenderer      = 1 << 1,
    RealtimeImageDownloaded = 1 << 2,
    PsWaveImageDownloaded   = 1 << 3,
    PsWaveImageSkip         = 1 << 4,
    ImageRendered           = 1 << 5,
    Worker1Stop             = 1 << 6,
    Worker2Stop             = 1 << 7,
    Worker1End              = 1 << 8,
    Worker2End              = 1 << 9,
    Error                   = 1 << 10,
};
struct KyoshinMonitorEventBits {
    uint32_t value;
    operator bool() const { return value != 0; }
    operator KyoshinMonitorEvent() const { return static_cast<KyoshinMonitorEvent>(value); }
};
inline KyoshinMonitorEventBits operator|(KyoshinMonitorEvent a, KyoshinMonitorEvent b) {
    return {static_cast<uint32_t>(a) | static_cast<uint32_t>(b)};
}
inline KyoshinMonitorEventBits operator&(KyoshinMonitorEvent a, KyoshinMonitorEvent b) {
    return {static_cast<uint32_t>(a) & static_cast<uint32_t>(b)};
}

class KyoshinMonitorCallback {
public:
    virtual void onData(uint16_t *img) = 0;
};

class KyoshinMonitor {
public:
    KyoshinMonitor();
    ~KyoshinMonitor();
    void startUpdateTimer();
    void stopUpdateTimer();
    void setCallback(KyoshinMonitorCallback *callback) { callback_ = callback; }
    const KyoshinForecast &getForecast() const { return forecast_; }

private:
    MapRegion map_region_{MapRegion::Japan};
    bool borehole_{false};
    RealtimeImgType realtime_img_type_{RealtimeImgType::RealtimeShindo};
    std::array<uint16_t*, 2> image_buffers_{};
    uint8_t image_buffer_idx_{0};
    HttpClient http_client_{KYOSHIN_SERVER_CONFIG.base_url};
    GifDecoder gif_decoder_{KYOSHIN_SERVER_CONFIG.imgWidth, KYOSHIN_SERVER_CONFIG.imgHeight};
    kyoshin_port_timer_t *timer_{nullptr};
    EventGroup<KyoshinMonitorEvent> event_group_{};
    time_t latest_time_{-1};
    KyoshinForecast forecast_;
    std::optional<std::vector<uint8_t>> realtime_img_gif_{std::nullopt};
    std::optional<std::vector<uint8_t>> pswave_img_gif_{std::nullopt};
    KyoshinMonitorCallback *callback_;

    const MapRegionConfig &regionConfig() const {
        return KYOSHIN_SERVER_CONFIG.regions[map_region_.value];
    }
    const char *realtimeImgUrlFormat() const {
        if (borehole_) return regionConfig().boreholeUrlFormats[realtime_img_type_.value];
        return regionConfig().surfaceUrlFormats[realtime_img_type_.value];
    }
    uint16_t *imageBuffer() const {
        return image_buffers_[image_buffer_idx_];
    }

    time_t getLatestTime();
    time_t updateLatestTime();
    std::string downloadForecast(time_t time);
    bool downloadRealtimeImage(time_t time);
    bool downloadPsWaveImage(time_t time);
    void decodeGifImage(uint8_t *data, size_t size);

    void worker1();
    void worker2();
};
