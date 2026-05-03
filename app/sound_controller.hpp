#pragma once
#include <string>
#include "kyoshin_port.hpp"
#include "resources/resources.h"

class SoundController {
public:
    bool isPlaying() const { return playing_sound_ != nullptr; }
    void play(std::string sound_settings);
    void play(const Sound *sound, bool auto_repeat);
    void stop();

private:
    const Sound *playing_sound_{nullptr};
    bool auto_repeat_{false};
    kyoshin_port_timer_t *timer_{nullptr};
    int timer_count_{0};

    void loop();
};

extern SoundController sound_controller;
