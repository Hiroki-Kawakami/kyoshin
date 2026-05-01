#include "kyoshin_app.hpp"
#include "screens/wifi_connect_screen.hpp"
#include "network_manager.hpp"

KyoshinMonitor *kyoshin_monitor;

void show_wifi_connect() {
    auto wifi_connect_screen = std::make_unique<WiFiConnectScreen>();
    screen_manager.push(std::move(wifi_connect_screen));
}

extern "C" void kyoshin_app() {
    network_manager.init();
    lv_async_call([](){
        show_wifi_connect();
    });
}
