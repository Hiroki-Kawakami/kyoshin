#pragma once
#include "settings_page.hpp"

class SettingsNightScreen: public SettingsPage {
public:
    virtual void build();

private:
    void addTimePickerRow(const char *title, uint16_t time, std::function<void(uint16_t)> on_change);
    void addNightBehaviorRow(const char *title, NightBehavior value, std::function<void(NightBehavior)> on_change);
};
