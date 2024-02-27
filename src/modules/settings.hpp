/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <map>
#include "M5Unified.h"
#include "nvs.hpp"
#include "config/server_config.hpp"
#include "config/layout_config.hpp"
#include "config/sound_config.hpp"
#include "esp_err.h"

class Settings {
public:
    NVS nvs = NVS("settings");

    template<class T, class F>
    void restore(const char *key, T defaultValue, F setter) {
        T value;
        if (nvs.get(key, &value) != ESP_OK) {
            value = defaultValue;
        }
        setter(value);
    }
    void restore() {
        restore<uint8_t>("map_region"    , MapRegion::Japan               , [&](uint8_t x) { mapRegion       = MapRegion(x);                     });
        restore<uint8_t>("rtimg_type"    , RealtimeImgType::RealtimeShindo, [&](uint8_t x) { realtimeImgType = RealtimeImgType(x);               });
        restore<uint8_t>("borehole"      , false                          , [&](uint8_t x) { borehole        = x;                                });
        restore<uint8_t>("layout_mode"   , ViewLayoutMode::AutoHorizontal , [&](uint8_t x) { layoutMode      = ViewLayoutMode(x);                });
        restore<uint8_t>("brightness"    , 20                             , [&](uint8_t x) { brightness      = x;                                });
        restore<int16_t>("dim_brightness", 0                              , [&](int16_t x) { dimBrightness   = x;                                });
        restore<int16_t>("dim_duration"  , 30                             , [&](int16_t x) { dimDuration     = x;                                });
        restore<uint8_t>("normal_vol"    , 64                             , [&](uint8_t x) { soundVolume[ForecastType::Normal] = x;              });
        restore<uint8_t>("normal_sound"  , SoundType::Alarm1              , [&](uint8_t x) { soundType  [ForecastType::Normal] = SoundType(x);   });
        restore<uint8_t>("normal_repeat" , SoundRepeat::Update            , [&](uint8_t x) { soundRepeat[ForecastType::Normal] = SoundRepeat(x); });
        restore<uint8_t>("alert_vol"     , 64                             , [&](uint8_t x) { soundVolume[ForecastType::Alert ] = x;              });
        restore<uint8_t>("alert_sound"   , SoundType::Alarm2              , [&](uint8_t x) { soundType  [ForecastType::Alert ] = SoundType(x);   });
        restore<uint8_t>("alert_repeat"  , SoundRepeat::On                , [&](uint8_t x) { soundRepeat[ForecastType::Alert ] = SoundRepeat(x); });
        restore<uint8_t>("use_night_mode", false                          , [&](uint8_t x) { useNightMode    = x;                                });
        restore<int16_t>("night_start"   , 23 * 60                        , [&](int16_t x) { nightStart      = x;                                });
        restore<int16_t>("night_end"     , 7 * 60                         , [&](int16_t x) { nightEnd        = x;                                });
        restore<uint8_t>("mute_training" , true                           , [&](uint8_t x) { muteTraining    = x;                                });
        restore<uint8_t>("wifi_setup"    , false                          , [&](uint8_t x) { wifiSetup       = x;                                });
    }

    MapRegion mapRegion;
    void setMapRegion(MapRegion region) {
        mapRegion = region;
        nvs.set("map_region", (uint8_t)region.value);
    }

    RealtimeImgType realtimeImgType;
    void setRealtimeImgType(RealtimeImgType type) {
        realtimeImgType = type;
        ESP_ERROR_CHECK_WITHOUT_ABORT(nvs.set("rtimg_type", (uint8_t)type.value));
    }

    bool borehole;
    void setBorehole(bool value) {
        borehole = value;
        nvs.set("borehole", (uint8_t)value);
    }

    ViewLayoutMode layoutMode;
    void setLayoutMode(ViewLayoutMode mode) {
        layoutMode = mode;
        nvs.set("layout_mode", (uint8_t)mode.value);
    }

    uint8_t brightness;
    void setBrightness(uint8_t value) {
        brightness = value;
        M5.Display.setBrightness(value);
        nvs.set("brightness", value);
    }

    int16_t dimBrightness;
    void setDimBrightness(int16_t value) {
        dimBrightness = value;
        nvs.set("dim_brightness", value);
    }

    int16_t dimDuration;
    void setDimDuration(int16_t duration) {
        dimDuration = duration;
        nvs.set("dim_duration", duration);
    }

    uint8_t soundVolume[ForecastType::count];
    void setSoundVolume(ForecastType type, uint8_t value) {
        soundVolume[type.value] = value;
        nvs.set(forecastTypeId(type) + "_vol", value);
    }

    SoundType soundType[ForecastType::count];
    void setSoundType(ForecastType type, SoundType sound) {
        soundType[type.value] = sound;
        nvs.set(forecastTypeId(type) + "_sound", (uint8_t)sound.value);
    }

    SoundRepeat soundRepeat[ForecastType::count];
    void setSoundRepeat(ForecastType type, SoundRepeat repeat) {
        soundRepeat[type.value] = repeat;
        nvs.set(forecastTypeId(type) + "_repeat", (uint8_t)repeat.value);
    }

    bool useNightMode;
    void setUseNightMode(bool use) {
        useNightMode = use;
        nvs.set("use_night_mode", (uint8_t)use);
    }

    int16_t nightStart, nightEnd;
    void setNightStart(int16_t start) {
        nightStart = start;
        nvs.set("night_start", start);
    }
    void setNightEnd(int16_t end) {
        nightEnd = end;
        nvs.set("night_end", end);
    }

    bool muteTraining;
    void setMuteTraining(bool mute) {
        muteTraining = mute;
        nvs.set("mute_training", (uint8_t)mute);
    }

    bool wifiSetup;
    void setWifiSetup(bool setup) {
        wifiSetup = setup;
        nvs.set("wifi_setup", (uint8_t)setup);
    }
};
