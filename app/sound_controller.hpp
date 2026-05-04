#pragma once
#include <string>
#include "kyoshin_port.hpp"
#include "resources/resources.h"

enum class SoundType {
    None,
    Alarm1,
    Alarm2,
    Alarm3,
};

enum class SoundRepeat {
    Once,
    Update,
    Repeat,
};

class SoundController {
public:
    bool isPlaying() const { return playing_sound_ != nullptr; }
    void play(std::string sound_settings);
    void play(const Sound *sound, bool auto_repeat);
    void stop();
    static std::string convert(SoundType type, SoundRepeat repeat, int volume);
    static void parse(std::string sound_settings, SoundType *type, SoundRepeat *repeat, int *volume);
    static const Sound *sound(SoundType type);

private:
    const Sound *playing_sound_{nullptr};
    bool auto_repeat_{false};
    kyoshin_port_timer_t *timer_{nullptr};
    int timer_count_{0};

    void loop();
};

extern SoundController sound_controller;
