#pragma once
#include "settings_page.hpp"

class SettingsScreen: public SettingsPage {
public:
    virtual void build();
    virtual void onDisappear();
};
