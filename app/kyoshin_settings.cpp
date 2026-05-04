#include "kyoshin_settings.hpp"
#include <time.h>

KyoshinSettings kyoshin_settings;

void KyoshinSettings::setMapRegion(MapRegion map_region) {
    map_region_ = map_region;
    nvs_.set("map_region", static_cast<uint8_t>(map_region.value));
}

void KyoshinSettings::setBorehole(bool borehole) {
    borehole_ = borehole;
    nvs_.set("borehole", static_cast<uint8_t>(borehole));
}

void KyoshinSettings::setRealtimeImageType(RealtimeImgType realtime_img_type) {
    realtime_img_type_ = realtime_img_type;
    nvs_.set("realtime_img_type", static_cast<uint8_t>(realtime_img_type.value));
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

void KyoshinSettings::setStandbyDuration(uint16_t standby_duration) {
    standby_duration_ = standby_duration;
    nvs_.set("standby_duration", standby_duration);
}

void KyoshinSettings::setBrightness(uint8_t brightness) {
    brightness_ = brightness;
    nvs_.set("brightness", brightness);
}

void KyoshinSettings::setStandbyBrightness(uint8_t standby_brightness) {
    standby_brightness_ = standby_brightness;
    nvs_.set("standby_brightness", standby_brightness);
}

void KyoshinSettings::setNightBrightness(uint8_t night_brightness) {
    night_brightness_ = night_brightness;
    nvs_.set("night_brightness", night_brightness);
}

void KyoshinSettings::setNightModeStart(uint16_t night_mode_start) {
    night_mode_start_ = night_mode_start;
    nvs_.set("night_mode_start", night_mode_start);
}

void KyoshinSettings::setNightModeEnd(uint16_t night_mode_end) {
    night_mode_end_ = night_mode_end;
    nvs_.set("night_mode_end", night_mode_end);
}

bool KyoshinSettings::inNightMode(time_t time) {
    struct tm *tm = localtime(&time);
    uint16_t now = tm->tm_hour * 60 + tm->tm_min;

    if (night_mode_start_ < night_mode_end_) {
        return now >= night_mode_start_ && now < night_mode_end_;
    } else {
        // wraps midnight (e.g. 23:00 – 07:00)
        return now >= night_mode_start_ || now < night_mode_end_;
    }
}
