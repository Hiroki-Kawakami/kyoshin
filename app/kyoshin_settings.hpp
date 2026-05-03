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
    ScreenLayout getScreenLayout() { return screen_layout_; }
    std::string getNormalSound() { return normal_sound_; }
    std::string getAlertSound() { return alert_sound_; }

    void setMapRegion(MapRegion map_region);
    void setScreenLayout(ScreenLayout kyoshin_screen_layout);
    void setNormalSound(std::string normal_sound);
    void setAlertSound(std::string alert_sound);

private:
    NVS nvs_{"kyoshin"};

    MapRegion map_region_{MapRegion::Japan};
    ScreenLayout screen_layout_{ScreenLayout::AutoHorizontal};
    std::string normal_sound_{"update alarm1 64"};
    std::string alert_sound_{"repeat alarm2 64"};

};

extern KyoshinSettings kyoshin_settings;
