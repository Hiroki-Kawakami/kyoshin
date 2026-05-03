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
    Update                  = 1u << 0,
    UpdateImage             = 1u << 1,
    StartImageRenderer      = 1u << 2,
    RealtimeImageDownloaded = 1u << 3,
    PsWaveImageDownloaded   = 1u << 4,
    PsWaveImageSkip         = 1u << 5,
    ImageRendered           = 1u << 6,
    DownloadBaseMap         = 1u << 7,
    Worker1Active           = 1u << 8,
    Worker2Active           = 1u << 9,
    Worker1Stop             = 1u << 10,
    Worker2Stop             = 1u << 11,
    Error                   = 1u << 31,
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
    virtual void onData(uint16_t *img) {};
    virtual void onBaseMapReady(bool result) {};
};

class KyoshinMonitor {
public:
    KyoshinMonitor();
    ~KyoshinMonitor();
    void startUpdateTimer();
    void stopUpdateTimer();
    void setCallback(KyoshinMonitorCallback *callback) { callback_ = callback; }
    bool loadBaseMapImage(bool download = false);
    MapRegion getMapRegion() const { return map_region_; }
    const KyoshinForecast &getForecast() const { return forecast_; }
    uint16_t *copyBaseMapImage();
    void setMapRegion(MapRegion region);
    void updateImage();

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
    bool downloadBaseMapImage();
    bool downloadRealtimeImage(time_t time);
    bool downloadPsWaveImage(time_t time);
    void decodeGifImage(uint8_t *data, size_t size);

    void startWorkers();
    void stopWorkers();
    void worker1();
    void worker2();
};
