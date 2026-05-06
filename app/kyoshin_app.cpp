#include "kyoshin_app.hpp"
#include "screens/wifi_connect_screen.hpp"
#include "screens/wifi_setup_screen.hpp"
#include "nvs.hpp"
#include "network_manager.hpp"

KyoshinMonitor *kyoshin_monitor;

void show_wifi_connect() {
    auto wifi_connect_screen = std::make_unique<WiFiConnectScreen>();
    screen_manager.push(std::move(wifi_connect_screen));
}

void kyoshin_app() {
    NVS::init();
    network_manager.init();
    if (network_manager.isConfigured()) {
        lv_lock();
        lv_async_call([](){
            screen_manager.push(std::make_unique<WiFiConnectScreen>());
        });
        lv_unlock();
    } else {
        lv_lock();
        lv_async_call([](){
            screen_manager.push(std::make_unique<WiFiSetupScreen>());
        });
        lv_unlock();
    }
}
