#include "settings_notify_screen.hpp"
#include "settings_sound_screen.hpp"

void SettingsNotifyScreen::build() {
    createBackButton("通知音");

    addListRow("予報受信時", std::nullopt, [](){
        screen_manager.push(std::make_unique<SettingsSoundScreen>(false));
    });
    addSeparator();
    addListRow("警報受信時", std::nullopt, [](){
        screen_manager.push(std::make_unique<SettingsSoundScreen>(true));
    });
    addSeparator();
    addSwitchRow("訓練報をミュート", kyoshin_settings.getMuteTraining(), [](bool value){
        kyoshin_settings.setMuteTraining(value);
    });
    addSeparator();
}
