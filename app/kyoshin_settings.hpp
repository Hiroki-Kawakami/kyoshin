#pragma once
#include <cstdint>
#include "kyoshin_monitor.hpp"
#include "nvs.hpp"

enum class ScreenLayout {
    AutoHorizontal,
    ZoomHorizontal,
    ZoomVertical,
    HorizontalInfo,
};

class KyoshinSettings {
public:
    MapRegion getMapRegion() { return map_region_; }
    bool getBorehole() { return borehole_; }
    RealtimeImgType getRealtimeImageType() { return realtime_img_type_; }
    ScreenLayout getScreenLayout() { return screen_layout_; }
    std::string getNormalSound() { return normal_sound_; }
    std::string getAlertSound() { return alert_sound_; }
    uint16_t getStandbyDuration() { return standby_duration_; }
    uint8_t getBrightness() { return brightness_; }
    uint8_t getStandbyBrightness() { return standby_brightness_; }
    uint8_t getNightBrightness() { return night_brightness_; }
    uint16_t getNightModeStart() { return night_mode_start_; }
    uint16_t getNightModeEnd() { return night_mode_end_; }

    void setMapRegion(MapRegion map_region);
    void setBorehole(bool borehole);
    void setRealtimeImageType(RealtimeImgType realtime_img_type);
    void setScreenLayout(ScreenLayout kyoshin_screen_layout);
    void setNormalSound(std::string normal_sound);
    void setAlertSound(std::string alert_sound);
    void setStandbyDuration(uint16_t standby_duration);
    void setBrightness(uint8_t brightness);
    void setStandbyBrightness(uint8_t standby_brightness);
    void setNightBrightness(uint8_t night_brightness);
    void setNightModeStart(uint16_t night_mode_start);
    void setNightModeEnd(uint16_t night_mode_end);

    bool inNightMode(time_t time);

private:
    NVS nvs_{"kyoshin"};

    MapRegion map_region_{MapRegion::Japan};
    bool borehole_{false};
    RealtimeImgType realtime_img_type_{RealtimeImgType::RealtimeShindo};
    ScreenLayout screen_layout_{ScreenLayout::AutoHorizontal};
    std::string normal_sound_{"update alarm1 64"};
    std::string alert_sound_{"repeat alarm2 64"};
    uint16_t standby_duration_{30000};
    uint8_t brightness_{127};
    uint8_t standby_brightness_{10};
    uint8_t night_brightness_{0};
    uint16_t night_mode_start_{23 * 60 + 0};
    uint16_t night_mode_end_{7 * 60 + 0};

};

extern KyoshinSettings kyoshin_settings;
