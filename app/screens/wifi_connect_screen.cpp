#include "wifi_connect_screen.hpp"
#include "kyoshin_screen.hpp"
#include "network_manager.hpp"

void WiFiConnectScreen::build() {
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    auto spinner = lv_spinner_create(root_);
    lv_obj_set_size(spinner, 60, 60);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 6, LV_PART_INDICATOR);

    auto ssid_label = lv_label_create(root_);
    lv_obj_set_style_pad_top(ssid_label, 16, 0);
    auto ssid_text = "SSID: " + network_manager.getWiFiSSID();
    lv_label_set_text(ssid_label, ssid_text.c_str());

    status_label_ = lv_label_create(root_);
    lv_obj_set_style_pad_top(status_label_, 4, 0);
    lv_label_set_text(status_label_, "Connecting");
}

static void connect_check_timer_cb(lv_timer_t *timer) {
    auto s = static_cast<WiFiConnectScreen*>(lv_timer_get_user_data(timer));
    s->check();
}

static void dot_timer_cb(lv_timer_t *timer) {
    auto s = static_cast<WiFiConnectScreen*>(lv_timer_get_user_data(timer));
    s->animateDots();
}

void WiFiConnectScreen::onAppear() {
    network_manager.connect(CONFIG_APP_WIFI_SSID, CONFIG_APP_WIFI_PASSWORD, [](NetworkManager::Result){});
    timer_ = lv_timer_create(connect_check_timer_cb, 100, this);
    dot_timer_ = lv_timer_create(dot_timer_cb, 500, this);
}

void WiFiConnectScreen::animateDots() {
    dot_count_ = (dot_count_ + 1) % 4;
    const char *dots[] = {"Connecting", "Connecting.", "Connecting..", "Connecting..."};
    lv_label_set_text(status_label_, dots[dot_count_]);
}

void WiFiConnectScreen::check() {
    if (!network_manager.isConnected()) return;
    lv_timer_delete(timer_);
    lv_timer_delete(dot_timer_);
    screen_manager.load(std::make_unique<KyoshinScreen>());
}
