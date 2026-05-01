#pragma once
#include <time.h>
#include "kyoshin_monitor_config.hpp"
#include "kyoshin_port.hpp"
#include "http_client.hpp"

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
};
inline KyoshinMonitorEvent operator|(KyoshinMonitorEvent a, KyoshinMonitorEvent b) {
    return static_cast<KyoshinMonitorEvent>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline KyoshinMonitorEvent operator&(KyoshinMonitorEvent a, KyoshinMonitorEvent b) {
    return static_cast<KyoshinMonitorEvent>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

class KyoshinMonitor {
public:
    KyoshinMonitor();
    ~KyoshinMonitor();
    void startUpdateTimer();
    void stopUpdateTimer();

private:
    std::array<uint16_t*, 2> image_buffers_{};
    HttpClient http_client_{KYOSHIN_SERVER_CONFIG.base_url};
    kyoshin_port_timer_t *timer_{nullptr};
    EventGroup<KyoshinMonitorEvent> event_group_{};
    time_t latest_time_{-1};

    time_t getLatestTime();
    void worker1();
    void worker2();
};
