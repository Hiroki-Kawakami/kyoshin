#include "settings_display_screen.hpp"

void SettingsDisplayScreen::build() {
    createBackButton("画面表示");

    addDropdownRow(
        "画面レイアウト",
        "自動(横)\nズーム(横)\nズーム(縦)\n情報(横)",
        static_cast<int>(kyoshin_settings.getScreenLayout()),
        std::nullopt,
        [](int idx){ kyoshin_settings.setScreenLayout(static_cast<ScreenLayout>(idx)); });
    addSeparator();
    addSliderRow(
        "明るさ",
        [](int value){ return std::to_string((int)roundf((float)value * 100 / 255)) + "%"; },
        13, 255, kyoshin_settings.getBrightness(),
        [](int value, bool is_last){
            kyoshin_settings.setBrightness(value);
            if (is_last) kyoshin_port_set_brightness(value);
        });
    addSeparator();
    static const uint32_t kStandbyDurations[] = {
        10000, 15000, 20000, 30000, 45000,
        60000, 120000, 300000, 600000, 900000, 1800000,
        UINT32_MAX,
    };
    auto standbyIndex = []() -> int {
        uint32_t current = kyoshin_settings.getStandbyDuration();
        for (int i = 0; i < 12; i++) {
            if (kStandbyDurations[i] == current) return i;
        }
        return 0;
    };
    addDropdownRow(
        "スタンバイモードに移行",
        "10秒後\n15秒後\n20秒後\n30秒後\n45秒後\n1分後\n2分後\n5分後\n10分後\n15分後\n30分後\nしない",
        standbyIndex(),
        100,
        [](int idx){ kyoshin_settings.setStandbyDuration(kStandbyDurations[idx]); });
    addSeparator();
    addSliderRow(
        "スタンバイモードの明るさ",
        [](int value){ return std::to_string((int)roundf((float)value * 100 / 255)) + "%"; },
        0, 255, kyoshin_settings.getStandbyBrightness(),
        [](int value, bool is_last){
            if (is_last) {
                kyoshin_settings.setStandbyBrightness(value);
                kyoshin_port_set_brightness(kyoshin_settings.getBrightness());
            } else {
                kyoshin_port_set_brightness(value);
            }
        });
    addSeparator();
}
