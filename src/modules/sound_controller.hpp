/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include <cstdint>
#include "M5Unified.h"
#include "date.hpp"
#include "config/sound_config.hpp"

class SoundController {
private:
    uint32_t repeat = 0, shortRepeat = 0, shortRepeatCount = 0;
    Date shortIntervalTime = 0, longIntervalTime = 0;
    int shortInterval = 0, longInterval = 0;
    const uint8_t *playData = nullptr;
public:
    bool isPlaying() const {
        return playData != nullptr;
    }
    void playSound(SoundType soundType, bool longRepeat, uint8_t volume) {
        auto &config = SOUND_TYPE_CONFIG[soundType.value];
        playWavInterval(config.data, config.repeat, config.shortInterval, config.shortRepeat, longRepeat ? config.longInterval : 0, volume);
    }
    void playWavInterval(const uint8_t *data, uint32_t repeat, int shortInterval, uint32_t shortRepeat, int longInterval, uint8_t volume) {
        M5.Speaker.stop();
        M5.Speaker.setVolume(volume);
        if (volume == 0) return;
        M5.Speaker.setChannelVolume(0, 0);
        M5.Speaker.tone(440, UINT32_MAX, 0, false);
        M5.Speaker.playWav(data, ~0, repeat, 1, true);
        this->repeat = repeat;
        this->shortRepeat = shortRepeat;
        this->shortRepeatCount = 0;
        this->shortIntervalTime = this->longIntervalTime = Date();
        this->shortInterval = shortInterval;
        this->longInterval = longInterval;
        this->playData = data;
    }
    void stop() {
        if (isPlaying()) {
            repeat = shortRepeat = shortRepeatCount = 0;
            shortIntervalTime = longIntervalTime = 0;
            shortInterval = longInterval = 0;
            playData = nullptr;
            M5.Speaker.setVolume(0);
            M5.Speaker.stop();
        }
    }
    void playRepeat() {
        M5.Speaker.stop();
        M5.Speaker.tone(440, UINT32_MAX, 0, false);
        M5.Speaker.playWav(playData, ~0, repeat, 1, true);
    }
    void eventLoop() {
        auto now = Date();
        if (!playData) return;
        if (longInterval > 0 && now - longIntervalTime >= longInterval) {
            playRepeat();
            shortRepeatCount = 0;
            shortIntervalTime = longIntervalTime = now;
            return;
        }
        if (now - shortIntervalTime >= shortInterval) {
            if (++shortRepeatCount < shortRepeat) {
                playRepeat();
                shortIntervalTime = now;
            } else {
                if (longInterval <= 0) {
                    stop();
                }
            }
        }
    }
};
