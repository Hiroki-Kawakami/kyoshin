#pragma once
#include "settings_page.hpp"

class SettingsSoundScreen: public SettingsPage {
public:
    virtual void build();

private:
    void addSoundRow(
        const char *title,
        std::string sound_settings,
        std::function<void(std::string)> on_change);
    void soundTest(std::string sound_settings);
};
