#include <memory>
#include "settings_sound_screen.hpp"
#include "kyoshin_settings.hpp"
#include "sound_controller.hpp"

SettingsSoundScreen::SettingsSoundScreen(bool is_alert): is_alert_{is_alert} {
    auto ss = is_alert ? kyoshin_settings.getAlertSound() : kyoshin_settings.getNormalSound();
    SoundController::parse(ss, &type_, &repeat_, &volume_);
}

void SettingsSoundScreen::build() {
    createBackButton(is_alert_ ? "警報受信時" : "予報受信時");

    addDropdownRow(
        is_alert_ ? "警報通知音" : "予報通知音",
        "なし\n警報1\n警報2\n警報3",
        static_cast<int>(type_),
        std::nullopt,
        [this](int idx){
            type_ = static_cast<SoundType>(idx);
            save();
        });
    addDropdownRow(
        "繰り返し",
        "なし\n情報更新時\nあり",
        static_cast<int>(repeat_),
        std::nullopt,
        [this](int idx){
            repeat_ = static_cast<SoundRepeat>(idx);
            save();
        });
    addSliderRow(
        "音量",
        [](int value){ return std::to_string((int)roundf((float)value * 100 / 255)) + "%"; },
        0, 255, volume_,
        [this](int value, bool is_last){
            if (is_last) {
                volume_ = value;
                save();
            }
        });
    auto button = lv_button_create(list_);
    lv_obj_set_size(button, LV_PCT(100), 30);
    lv_obj_set_style_margin_hor(button, 16, 0);
    lv_obj_set_style_margin_bottom(button, 8, 0);
    lv_obj_add_event_fn(button, LV_EVENT_CLICKED, [this](lv_event_t*){
        soundTest();
    });
    auto label = lv_label_create(button);
    lv_obj_center(label);
    lv_label_set_text(label, "サウンドテスト");
    lv_obj_set_style_text_font(label, R.font.ipa_16, 0);
}

void SettingsSoundScreen::save() {
    if (is_alert_) {
        kyoshin_settings.setAlertSound(convert());
    } else {
        kyoshin_settings.setNormalSound(convert());
    }
}

void SettingsSoundScreen::soundTest() {
    auto scroll_y = lv_obj_get_scroll_y(list_);
    auto msgbox = lv_msgbox_create(NULL);
    auto label = lv_msgbox_add_text(msgbox, "サウンドテスト");
    lv_obj_set_style_text_font(label, R.font.ipa_16, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_ver(label, 16, 0);
    auto button = lv_msgbox_add_footer_button(msgbox, "完了");
    lv_obj_set_style_text_font(button, R.font.ipa_16, 0);
    lv_obj_add_event_fn(button, LV_EVENT_CLICKED, [=, this](lv_event_t*){
        sound_controller.stop();
        lv_async_call([=, this](){ lv_obj_scroll_to_y(list_, scroll_y, LV_ANIM_OFF); });
        lv_msgbox_close(msgbox);
    });
    sound_controller.play(convert());
}
