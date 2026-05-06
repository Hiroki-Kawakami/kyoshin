#include "wifi_connect_screen.hpp"
#include "kyoshin_screen.hpp"
#include "network_manager.hpp"
#include "resources/resources.h"

void WiFiConnectScreen::build() {
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(root_, 12, 0);

    auto spinner = lv_spinner_create(root_);
    lv_obj_set_size(spinner, 48, 48);
    lv_obj_set_style_arc_width(spinner, 5, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 5, LV_PART_INDICATOR);

    auto ssid_label = lv_label_create(root_);
    auto ssid_text = "SSID: " + network_manager.getWiFiSSID();
    lv_label_set_text(ssid_label, ssid_text.c_str());
    lv_obj_set_style_text_font(ssid_label, R.font.ipa_16, 0);

    auto status = lv_label_create(root_);
    lv_label_set_text(status, "接続中...");
    lv_obj_set_style_text_font(status, R.font.ipa_16, 0);
    lv_obj_set_style_text_color(status, lv_color_hex(0x888888), 0);

    auto button = lv_button_create(root_);
    lv_obj_add_event_fn(button, LV_EVENT_CLICKED, [](lv_event_t*){
        kyoshin_settings.setEnterWiFiSetup(true);
        kyoshin_settings.commit();
        kyoshin_port_restart();
    });
    auto button_label = lv_label_create(button);
    lv_label_set_text(button_label, "WiFiを再設定");
    lv_obj_center(button_label);
    lv_obj_set_style_text_font(button_label, R.font.ipa_16, 0);
}

static void connect_check_timer_cb(lv_timer_t *timer) {
    auto s = static_cast<WiFiConnectScreen*>(lv_timer_get_user_data(timer));
    s->check();
}

void WiFiConnectScreen::onAppear() {
    network_manager.connect([](NetworkManager::Result){});
    timer_ = lv_timer_create(connect_check_timer_cb, 100, this);
}

void WiFiConnectScreen::check() {
    if (!network_manager.isConnected()) return;
    lv_timer_delete(timer_);
    screen_manager.load(std::make_unique<KyoshinScreen>());
}
