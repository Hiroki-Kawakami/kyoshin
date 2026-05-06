#include "kyoshin_settings.hpp"
#include <time.h>
#include <vector>

KyoshinSettings kyoshin_settings;

void KyoshinSettings::restore() {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;

    if (nvs_.get("map_region", &u8) == NVS::Error::OK) map_region_ = MapRegion(u8);
    if (nvs_.get("borehole", &u8) == NVS::Error::OK) borehole_ = u8;
    if (nvs_.get("img_type", &u8) == NVS::Error::OK) realtime_img_type_ = RealtimeImgType(u8);
    if (nvs_.get("screen_layout", &u8) == NVS::Error::OK) screen_layout_ = static_cast<ScreenLayout>(u8);
    if (nvs_.get("mute_training", &u8) == NVS::Error::OK) mute_training_ = u8;
    if (nvs_.get("brightness", &u8) == NVS::Error::OK) brightness_ = u8;
    if (nvs_.get("stby_bright", &u8) == NVS::Error::OK) standby_brightness_ = u8;
    if (nvs_.get("night_bright", &u8) == NVS::Error::OK) night_brightness_ = u8;
    if (nvs_.get("night_enable", &u8) == NVS::Error::OK) night_mode_enable_ = u8;
    if (nvs_.get("night_norm_beh", &u8) == NVS::Error::OK) night_normal_behavior_ = static_cast<NightBehavior>(u8);
    if (nvs_.get("night_alt_beh", &u8) == NVS::Error::OK) night_alert_behavior_ = static_cast<NightBehavior>(u8);
    if (nvs_.get("wifi_setup", &u8) == NVS::Error::OK) enter_wifi_setup_ = u8;

    if (nvs_.get("stby_duration", &u32) == NVS::Error::OK) standby_duration_ = u32;

    if (nvs_.get("night_start", &u16) == NVS::Error::OK) night_mode_start_ = u16;
    if (nvs_.get("night_mode_end", &u16) == NVS::Error::OK) night_mode_end_ = u16;

    size_t len = 0;
    if (nvs_.get("normal_sound", static_cast<char*>(nullptr), &len) == NVS::Error::OK) {
        std::vector<char> buf(len);
        nvs_.get("normal_sound", buf.data(), &len);
        normal_sound_ = buf.data();
    }
    len = 0;
    if (nvs_.get("alert_sound", static_cast<char*>(nullptr), &len) == NVS::Error::OK) {
        std::vector<char> buf(len);
        nvs_.get("alert_sound", buf.data(), &len);
        alert_sound_ = buf.data();
    }
}

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
    nvs_.set("img_type", static_cast<uint8_t>(realtime_img_type.value));
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

void KyoshinSettings::setMuteTraining(bool mute_training) {
    mute_training_ = mute_training;
    nvs_.set("mute_training", static_cast<uint8_t>(mute_training));
}

void KyoshinSettings::setStandbyDuration(uint32_t standby_duration) {
    standby_duration_ = standby_duration;
    nvs_.set("stby_duration", standby_duration);
}

void KyoshinSettings::setBrightness(uint8_t brightness) {
    brightness_ = brightness;
    nvs_.set("brightness", brightness);
}

void KyoshinSettings::setStandbyBrightness(uint8_t standby_brightness) {
    standby_brightness_ = standby_brightness;
    nvs_.set("stby_bright", standby_brightness);
}

void KyoshinSettings::setNightBrightness(uint8_t night_brightness) {
    night_brightness_ = night_brightness;
    nvs_.set("night_bright", night_brightness);
}

void KyoshinSettings::setNightModeEnable(bool night_mode_enable) {
    night_mode_enable_ = night_mode_enable;
    nvs_.set("night_enable", static_cast<uint8_t>(night_mode_enable));
}

void KyoshinSettings::setNightModeStart(uint16_t night_mode_start) {
    night_mode_start_ = night_mode_start;
    nvs_.set("night_start", night_mode_start);
}

void KyoshinSettings::setNightModeEnd(uint16_t night_mode_end) {
    night_mode_end_ = night_mode_end;
    nvs_.set("night_mode_end", night_mode_end);
}

void KyoshinSettings::setNightNormalBehavior(NightBehavior night_normal_behavior) {
    night_normal_behavior_ = night_normal_behavior;
    nvs_.set("night_norm_beh", static_cast<uint8_t>(night_normal_behavior));
}

void KyoshinSettings::setNightAlertBehavior(NightBehavior night_alert_behavior) {
    night_alert_behavior_ = night_alert_behavior;
    nvs_.set("night_alt_beh", static_cast<uint8_t>(night_alert_behavior));
}

void KyoshinSettings::setEnterWiFiSetup(bool enter_wifi_setup) {
    enter_wifi_setup_ = enter_wifi_setup;
    nvs_.set("wifi_setup", static_cast<uint8_t>(enter_wifi_setup));
}

bool KyoshinSettings::inNightMode(time_t time) {
    if (!getNightModeEnable()) return false;
    struct tm *tm = localtime(&time);
    uint16_t now = tm->tm_hour * 60 + tm->tm_min;

    if (night_mode_start_ < night_mode_end_) {
        return now >= night_mode_start_ && now < night_mode_end_;
    } else {
        // wraps midnight (e.g. 23:00 – 07:00)
        return now >= night_mode_start_ || now < night_mode_end_;
    }
}
