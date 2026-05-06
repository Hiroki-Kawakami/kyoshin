#include "settings_misc_screen.hpp"
#include "flash_image.hpp"

void SettingsMiscScreen::build() {
    createBackButton("その他");

    addListRow("画像キャッシュを削除", std::nullopt, [](){
        flash_image.deleteImage();
        showInformation("画像キャッシュが削除されました");
    });
    addSeparator();
    addListRow("WiFiを再設定", std::nullopt, [](){
        kyoshin_settings.setEnterWiFiSetup(true);
        kyoshin_settings.commit();
        kyoshin_port_restart();
    });
    addSeparator();
}
