#include "settings_screen.hpp"
#include "settings_display_screen.hpp"

void SettingsScreen::build() {
    createBackButton("設定");
    list_ = lv_obj_create(root_);
    lv_obj_remove_style_all(list_);
    lv_obj_align(list_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(list_, 320, 192);
    lv_obj_set_style_pad_hor(list_, 4, 0);
    lv_obj_set_style_pad_bottom(list_, 4, 0);
    lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(list_, 4, 0);

    auto create_row = [this](const lv_img_dsc_t *icon, const char *title, std::function<void()> on_click){
        auto row = lv_obj_create(list_);
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

    create_row(R.icon.monitor, "画面表示", [](){
        screen_manager.push(std::make_unique<SettingsDisplayScreen>());
    });
    create_row(R.icon.bell_ring, "通知音", [](){});
    create_row(R.icon.moon, "夜間モード", [](){});
    create_row(R.icon.ellipsis, "その他", [](){});
}
