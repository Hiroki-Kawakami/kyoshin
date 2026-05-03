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

void KyoshinSettings::setNormalSound(std::string normal_sound) {
    normal_sound_ = normal_sound;
    nvs_.set("normal_sound", normal_sound.c_str());
}

void KyoshinSettings::setAlertSound(std::string alert_sound) {
    alert_sound_ = alert_sound;
    nvs_.set("alert_sound", alert_sound.c_str());
}
