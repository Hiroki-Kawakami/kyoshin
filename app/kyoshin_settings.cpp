#include "kyoshin_settings.hpp"

KyoshinSettings kyoshin_settings;

void KyoshinSettings::setMapRegion(MapRegion map_region) {
    map_region_ = map_region;
    nvs_.set("map_region", (uint8_t)map_region.value);
}

void KyoshinSettings::setScreenLayout(ScreenLayout screen_layout) {
    screen_layout_ = screen_layout;
    nvs_.set("screen_layout", static_cast<uint8_t>(screen_layout));
}
