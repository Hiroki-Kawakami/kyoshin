#pragma once
#include "settings_page.hpp"
#include "sound_controller.hpp"

class SettingsSoundScreen: public SettingsPage {
public:
    SettingsSoundScreen(bool is_alert);
    virtual void build();

private:
    bool is_alert_;
    SoundType type_;
    SoundRepeat repeat_;
    int volume_;

    std::string convert() const {
        return SoundController::convert(type_, repeat_, volume_);
    }
    void save();
    void soundTest();
};
