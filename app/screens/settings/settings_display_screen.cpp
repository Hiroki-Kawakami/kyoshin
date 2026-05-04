#include "settings_display_screen.hpp"
#include "kyoshin_settings.hpp"

void SettingsDisplayScreen::build() {
    createBackButton("画面表示");

    addListRow("画面レイアウト", [this](lv_obj_t *row, lv_obj_t*){
        createDropdown(
            row,
            "自動(横)\nズーム(横)\nズーム(縦)\n情報(横)",
            static_cast<int>(kyoshin_settings.getScreenLayout()),
            [](int idx){ kyoshin_settings.setScreenLayout(static_cast<ScreenLayout>(idx)); });
    });
}
