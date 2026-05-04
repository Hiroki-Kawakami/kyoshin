#include "settings_screen.hpp"
#include "resources/resources.h"

void SettingsScreen::build() {
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(root_, 4, 0);
    lv_obj_set_style_pad_all(root_, 4, 0);

    { // Back Button
        auto button = lv_button_create(root_);
        lv_obj_remove_style_all(button);
        lv_obj_set_height(button, 40);
        lv_obj_set_style_pad_hor(button, 12, 0);
        lv_obj_set_style_bg_color(button, lv_color_black(), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(button, LV_OPA_20, LV_STATE_PRESSED);
        lv_obj_set_style_radius(button, 8, 0);
        lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(button, 12, 0);
        lv_obj_add_event_cb(button, [](lv_event_t*){ screen_manager.pop(); }, LV_EVENT_CLICKED, nullptr);
        auto icon = lv_label_create(button);
        lv_label_set_text(icon, LV_SYMBOL_LEFT);
        lv_obj_set_style_text_font(icon, R.font.ipa_24, 0);
        auto label = lv_label_create(button);
        lv_label_set_text(label, "設定");
        lv_obj_set_style_text_font(label, R.font.ipa_24, 0);
    }

    auto create_row = [this](const lv_img_dsc_t *icon, const char *title, std::function<void()> on_click){
        auto row = lv_obj_create(root_);
        lv_obj_set_style_bg_color(row, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(row, 8, 0);
        lv_obj_set_flex_grow(row, 1);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_hor(row, 10, 0);
        lv_obj_set_style_pad_column(row, 8, 0);
        lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xeeeeee), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, LV_STATE_PRESSED);
        lv_obj_add_event_fn(row, LV_EVENT_CLICKED, [=](lv_event_t*){ on_click(); });

        auto image = lv_image_create(row);
        lv_image_set_src(image, icon);

        auto label = lv_label_create(row);
        lv_label_set_text(label, title);
        lv_obj_set_style_text_font(label, R.font.ipa_16, 0);

        return row;
    };

    create_row(R.icon.monitor, "画面表示", [](){});
    create_row(R.icon.bell_ring, "通知音", [](){});
    create_row(R.icon.moon, "夜間モード", [](){});
    create_row(R.icon.ellipsis, "その他", [](){});
}
