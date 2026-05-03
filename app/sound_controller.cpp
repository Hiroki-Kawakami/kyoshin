#include "sound_controller.hpp"
#include <sstream>

SoundController sound_controller;

void SoundController::play(std::string sound_settings) {
    std::stringstream ss(sound_settings);
    std::string timing, sound_id;
    int n;

    if (!(ss >> timing >> sound_id >> n)) {
        printf("Invalid format: %s\n", sound_settings.c_str());
        return;
    }

    const Sound *sound;
    bool auto_repeat;

    if (sound_id == "alarm1") sound = R.sound.alarm1;
    else if (sound_id == "alarm2") sound = R.sound.alarm2;
    else if (sound_id == "alarm3") sound = R.sound.alarm3;
    else { stop(); return; }

    if (timing == "update") {
        auto_repeat = false;
    } else if (timing == "once") {
        if (isPlaying()) return;
        auto_repeat = false;
    } else if (timing == "repeat") {
        if (playing_sound_ == sound) return;
        auto_repeat = true;
    } else {
        stop();
        return;
    }

    play(sound, auto_repeat);
}

void SoundController::play(const Sound *sound, bool auto_repeat) {
    stop();
    auto_repeat_ = auto_repeat;
    timer_count_ = 0;
    playing_sound_ = sound;

    if (!timer_) {
        timer_ = kyoshin_port_timer_create("sound", [this](){ loop(); });
    }
    loop();
    kyoshin_port_timer_start_periodic(timer_, 100);
}

void SoundController::stop() {
    if (!isPlaying()) return;
    kyoshin_port_timer_stop(timer_);
    playing_sound_ = nullptr;
    kyoshin_port_wav_stop();
}

void SoundController::loop() {
    if (timer_count_ >= playing_sound_->long_interval) {
        if (auto_repeat_) timer_count_ = 0;
        else return;
    }
    if ((timer_count_ % playing_sound_->short_interval) == 0 &&
        timer_count_ < playing_sound_->short_interval * playing_sound_->short_repeat) {
        kyoshin_port_wav_play(playing_sound_->data, 100, playing_sound_->repeat);
    }
    timer_count_++;
}
