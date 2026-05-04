#include "settings_night_screen.hpp"
#include <cstdio>

void SettingsNightScreen::build() {
    createBackButton("夜間モード");

    addSwitchRow("夜間モードを使用", kyoshin_settings.getNightModeEnable(), [](bool value){
        kyoshin_settings.setNightModeEnable(value);
    });
    addSeparator();
    addTimePickerRow("開始時刻", kyoshin_settings.getNightModeStart(), [](uint16_t value){
        kyoshin_settings.setNightModeStart(value);
    });
    addSeparator();
    addTimePickerRow("終了時刻", kyoshin_settings.getNightModeEnd(), [](uint16_t value){
        kyoshin_settings.setNightModeEnd(value);
    });
    addSeparator();
    addNightBehaviorRow("予報", kyoshin_settings.getNightNormalBehavior(), [](NightBehavior value){
        kyoshin_settings.setNightNormalBehavior(value);
    });
    addSeparator();
    addNightBehaviorRow("警報", kyoshin_settings.getNightAlertBehavior(), [](NightBehavior value){
        kyoshin_settings.setNightAlertBehavior(value);
    });
    addSeparator();
    addSliderRow(
        "夜間モードの明るさ",
        [](int value){ return std::to_string((int)roundf((float)value * 100 / 255)) + "%"; },
        0, 255, kyoshin_settings.getNightBrightness(),
        [](int value, bool is_last){
            if (is_last) {
                kyoshin_settings.setNightBrightness(value);
                kyoshin_port_set_brightness(kyoshin_settings.getBrightness());
            } else {
                kyoshin_port_set_brightness(value);
            }
        });
    addSeparator();
}

void SettingsNightScreen::addTimePickerRow(const char *title, uint16_t time, std::function<void(uint16_t)> on_change) {
    addListRow(title, [=](lv_obj_t *row, lv_obj_t*){
        std::string hour_opts;
        for (int i = 0; i < 24; i++) {
            if (i > 0) hour_opts += '\n';
            char buf[3];
            snprintf(buf, sizeof(buf), "%02d", i);
            hour_opts += buf;
        }
        std::string min_opts;
        for (int i = 0; i < 60; i += 5) {
            if (i > 0) min_opts += '\n';
            char buf[3];
            snprintf(buf, sizeof(buf), "%02d", i);
            min_opts += buf;
        }

        auto make_dd = [](lv_obj_t *parent, const char *opts, int selected) -> lv_obj_t* {
            auto dd = lv_dropdown_create(parent);
            lv_obj_set_size(dd, 56, 32);
            lv_obj_set_style_text_font(dd, R.font.ipa_16, 0);
            lv_obj_set_style_text_font(lv_dropdown_get_list(dd), R.font.ipa_16, 0);
            lv_dropdown_set_options(dd, opts);
            lv_dropdown_set_selected(dd, selected);
            return dd;
        };

        auto hour_dd = make_dd(row, hour_opts.c_str(), time / 60);

        auto sep = lv_label_create(row);
        lv_label_set_text(sep, ":");
        lv_obj_set_style_text_font(sep, R.font.ipa_16, 0);

        auto min_dd = make_dd(row, min_opts.c_str(), (time % 60) / 5);

        lv_obj_add_event_fn(hour_dd, LV_EVENT_VALUE_CHANGED, [=](lv_event_t*){
            on_change(lv_dropdown_get_selected(hour_dd) * 60 + lv_dropdown_get_selected(min_dd) * 5);
        });
        lv_obj_add_event_fn(min_dd, LV_EVENT_VALUE_CHANGED, [=](lv_event_t*){
            on_change(lv_dropdown_get_selected(hour_dd) * 60 + lv_dropdown_get_selected(min_dd) * 5);
        });
    });
}

void SettingsNightScreen::addNightBehaviorRow(const char *title, NightBehavior value, std::function<void(NightBehavior)> on_change) {
    addDropdownRow(
        title,
        "通知しない\nミュート\n通知する",
        static_cast<int>(value),
        std::nullopt,
        [=](int value){ on_change(static_cast<NightBehavior>(value)); });
}
