#include "settings_page.hpp"
#include "resources/resources.h"

void SettingsPage::createBackButton(const char *title) {
    auto button = lv_button_create(root_);
    lv_obj_remove_style_all(button);
    lv_obj_set_height(button, 40);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 4, 4);
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
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, R.font.ipa_24, 0);
}

void SettingsPage::createList() {
    list_ = lv_obj_create(root_);
    lv_obj_align(list_, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(list_, 320, 192);
    lv_obj_set_style_border_side(list_, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_width(list_, 1, 0);
    lv_obj_set_style_radius(list_, 0, 0);
    lv_obj_set_flex_flow(list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(list_, 0, 0);
    lv_obj_set_style_pad_row(list_, 0, 0);
}

void SettingsPage::addListRow(
    const char *title,
    std::function<void(lv_obj_t *row, lv_obj_t *col)> factory,
    std::optional<std::function<void()>> on_click) {

    if (!list_) createList();
    auto col = lv_obj_create(list_);
    lv_obj_remove_style_all(col);
    lv_obj_set_size(col, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    if (on_click.has_value()) {
        lv_obj_add_flag(col, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_color(col, lv_color_black(), LV_STATE_PRESSED);
        lv_obj_set_style_bg_opa(col, LV_OPA_20, LV_STATE_PRESSED);
        lv_obj_add_event_fn(col, LV_EVENT_CLICKED, [=](lv_event_t*){ on_click.value(); });
    } else {
        lv_obj_remove_flag(col, LV_OBJ_FLAG_CLICKABLE);
    }

    auto row = lv_obj_create(col);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), 40);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_hor(row, 16, 0);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_CLICKABLE);

    auto label = lv_label_create(row);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, R.font.ipa_16, 0);
    lv_obj_set_flex_grow(label, 1);

    factory(row, col);

    auto sep = lv_obj_create(list_);
    lv_obj_remove_style_all(sep);
    lv_obj_set_size(sep, LV_PCT(100), 1);
    lv_obj_set_style_margin_hor(sep, 8, 0);
    lv_obj_set_style_bg_color(sep, lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
    lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);
}

lv_obj_t *SettingsPage::createDropdown(
    lv_obj_t *parent,
    const char *options,
    int selected,
    std::function<void(int)> on_change) {

    auto dropdown = lv_dropdown_create(parent);
    lv_obj_set_height(dropdown, 32);
    lv_obj_set_style_text_font(dropdown, R.font.ipa_16, 0);
    auto list = lv_dropdown_get_list(dropdown);
    lv_obj_set_style_text_font(list, R.font.ipa_16, 0);
    lv_dropdown_set_options(dropdown, options);
    lv_dropdown_set_selected(dropdown, selected);
    lv_obj_add_event_fn(dropdown, LV_EVENT_VALUE_CHANGED, [=](lv_event_t*){
        on_change(lv_dropdown_get_selected(dropdown));
    });
    return dropdown;
}
