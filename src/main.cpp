#include <M5Unified.h>
#include "nvs.hpp"
#include "networking.hpp"
#include "ui.hpp"
#include "kyoshin.hpp"
#include "scenes/wifi_connect_scene.hpp"
#include "scenes/wifi_setup_scene.hpp"

// Shared Instances
ImageBuffer imgBuffer;
Networking::HTTPClient httpClient(20 * 1024); // 18KB Buffer
RTOS::Task<portMAX_DELAY> bgTask1("bgTask1");
Settings settings;
FlashImageController flashImage;
SoundController soundController;

extern "C" void app_main() {
    NVS::init();
    Networking::init();

    bgTask1.createQueue();
    bgTask1.start(RTOS::TaskPriority::Normal, 1024 * 3, 1);
    httpClient.clearBuffer();
    settings.restore();
    flashImage.init();

    M5.begin();
    vTaskDelay(pdMS_TO_TICKS(100));

    M5.Lcd.setBrightness(settings.brightness);
    if (Networking::configured() && !settings.wifiSetup) {
        UI::setRootScene(std::make_shared<WifiConnectScene>());
    } else {
        UI::setRootScene(std::make_shared<WifiSetupScene>());
    }
    UI::run();
}
