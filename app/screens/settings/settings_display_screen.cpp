#include <cmath>
#include "settings_display_screen.hpp"
#include "kyoshin_settings.hpp"

void SettingsDisplayScreen::build() {
    createBackButton("画面表示");

    addDropdownRow(
        "画面レイアウト",
        "自動(横)\nズーム(横)\nズーム(縦)\n情報(横)",
        static_cast<int>(kyoshin_settings.getScreenLayout()),
        [](int idx){ kyoshin_settings.setScreenLayout(static_cast<ScreenLayout>(idx)); });
    addSliderRow(
        "明るさ",
        [](int value){ return std::to_string((int)roundf((float)value * 100 / 255)) + "%"; },
        13, 255, kyoshin_settings.getBrightness(),
        [](int value){
            kyoshin_settings.setBrightness(value);
            kyoshin_port_set_brightness(value);
        });
}
