#include <memory>
#include "settings_sound_screen.hpp"
#include "kyoshin_settings.hpp"
#include "sound_controller.hpp"

void SettingsSoundScreen::build() {
    createBackButton("通知音");

    addSoundRow(
        "予報通知音",
        kyoshin_settings.getNormalSound(),
        [](std::string sound_settings){ kyoshin_settings.setNormalSound(sound_settings); });
    addSoundRow(
        "警報通知音",
        kyoshin_settings.getAlertSound(),
        [](std::string sound_settings){ kyoshin_settings.setAlertSound(sound_settings); });
}

void SettingsSoundScreen::addSoundRow(
    const char *title,
    std::string sound_settings,
    std::function<void(std::string)> on_change) {

    SoundType type;
    SoundRepeat repeat;
    int volume;
    SoundController::parse(sound_settings, &type, &repeat, &volume);
    auto update = std::make_shared<std::function<std::string(SoundType*, SoundRepeat*, int*)>>(
        [=](SoundType *pType, SoundRepeat *pRepeat, int *pVolume) mutable {
            if (pType) type = *pType;
            if (pRepeat) repeat = *pRepeat;
            if (pVolume) volume = *pVolume;
            return SoundController::convert(type, repeat, volume);
        }
    );

    addDropdownRow(
        title,
        "なし\n警報1\n警報2\n警報3",
        static_cast<int>(type),
        std::nullopt,
        [=](int idx){
            auto type = static_cast<SoundType>(idx);
            on_change((*update)(&type, nullptr, nullptr));
        });
    addDropdownRow(
        "繰り返し",
        "なし\n情報更新時\nあり",
        static_cast<int>(repeat),
        std::nullopt,
        [=](int idx){
            auto repeat = static_cast<SoundRepeat>(idx);
            on_change((*update)(nullptr, &repeat, nullptr));
        });
    addSliderRow(
        "音量",
        [](int value){ return std::to_string((int)roundf((float)value * 100 / 255)) + "%"; },
        0, 255, volume,
        [=](int value, bool is_last){
            if (is_last) {
                on_change((*update)(nullptr, nullptr, &value));
            }
        });
    addSeparator();
}
