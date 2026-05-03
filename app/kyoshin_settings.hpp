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

    void setMapRegion(MapRegion map_region);
    void setScreenLayout(ScreenLayout kyoshin_screen_layout);

private:
    NVS nvs_{"kyoshin"};

    MapRegion map_region_{MapRegion::Japan};
    ScreenLayout screen_layout_{ScreenLayout::AutoHorizontal};
};

extern KyoshinSettings kyoshin_settings;
