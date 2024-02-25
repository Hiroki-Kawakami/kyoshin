/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "config_internal.hpp"
#include "resources/sounds.hpp"

DEF_CONFIG_ENUM(SoundType, Alarm1, Alarm2, Alarm3);
struct SoundTypeConfig {
    const char *displayString;
    const uint8_t *data;
    uint32_t repeat;
    int shortInterval;
    uint32_t shortRepeat;
    int longInterval;
};
inline constexpr SoundTypeConfig SOUND_TYPE_CONFIG[SoundType::count] = {
    { "警報音1", SoundData::alarm1, 3, 1000, 1, 5000 },
    { "警報音2", SoundData::alarm2, 1, 800 , 2, 7000 },
    { "警報音3", SoundData::alarm3, 2, 2000, 1, 6000 },
};
inline const char *displayString(SoundType value) {
    return SOUND_TYPE_CONFIG[value.value].displayString;
}

DEF_CONFIG_ENUM(ForecastType, Normal, Alert);
inline std::string forecastTypeId(ForecastType value) {
    switch (value.value) {
    case ForecastType::Normal: return "normal";
    case ForecastType::Alert : return "alert";
    default                  : return "unknown";
    }
}

DEF_CONFIG_ENUM(SoundRepeat, None, Update, On);
inline const char *displayString(SoundRepeat value) {
    switch (value.value) {
    case SoundRepeat::None  : return "なし";
    case SoundRepeat::Update: return "情報更新時";
    case SoundRepeat::On    : return "有効";
    default                 : return "unknown";
    }
}
