#include "sound_controller.hpp"
#include <sstream>

SoundController sound_controller;

void SoundController::play(std::string sound_settings) {
    SoundType type;
    SoundRepeat repeat;
    int volume;
    parse(sound_settings, &type, &repeat, &volume);

    auto sound = this->sound(type);
    if (!sound || volume <= 0) {
        stop();
        return;
    }

    switch (repeat) {
    case SoundRepeat::Once:
        if (!isPlaying()) play(sound, false);
        break;
    case SoundRepeat::Update:
        play(sound, false);
        break;
    case SoundRepeat::Repeat:
        if (playing_sound_ != sound) play(sound, true);
        break;
    default:
        stop();
        break;
    }
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

std::string SoundController::convert(SoundType type, SoundRepeat repeat, int volume) {
    auto convert_type = [](SoundType type){
        switch (type) {
        case SoundType::Alarm1: return std::string{"alarm1"};
        case SoundType::Alarm2: return std::string{"alarm2"};
        case SoundType::Alarm3: return std::string{"alarm3"};
        default: return std::string{"none"};
        }
    };
    auto convert_repeat = [](SoundRepeat repeat){
        switch (repeat) {
        case SoundRepeat::Update: return std::string{"update"};
        case SoundRepeat::Repeat: return std::string{"repeat"};
        default: return std::string{"once"};
        }
    };
    return convert_type(type) + " " + convert_repeat(repeat) + " " + std::to_string(volume);
}

void SoundController::parse(std::string sound_settings, SoundType *type, SoundRepeat *repeat, int *volume) {
    std::stringstream ss(sound_settings);
    std::string timing, sound_id;
    int n;

    if (!(ss >> timing >> sound_id >> n)) {
        printf("Invalid format: %s\n", sound_settings.c_str());
        *type = SoundType::None;
        *repeat = SoundRepeat::Once;
        *volume = 64;
        return;
    }

    if (sound_id == "alarm1") *type = SoundType::Alarm1;
    else if (sound_id == "alarm2") *type = SoundType::Alarm2;
    else if (sound_id == "alarm3") *type = SoundType::Alarm3;
    else { *type = SoundType::None; }

    if (timing == "update") *repeat = SoundRepeat::Update;
    else if (timing == "repeat") *repeat = SoundRepeat::Repeat;
    else *repeat = SoundRepeat::Once;

    if (n < 0) *volume = 0;
    else if (n > 255) *volume = 255;
    else *volume = n;
}

const Sound *SoundController::sound(SoundType type) {
    switch (type) {
    case SoundType::Alarm1: return R.sound.alarm1;
    case SoundType::Alarm2: return R.sound.alarm2;
    case SoundType::Alarm3: return R.sound.alarm3;
    default: return nullptr;
    }
}
