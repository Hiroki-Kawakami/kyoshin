/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"

class WifiSetupScene : public UI::Scene {

    void didAppear() override {
        if (M5.Lcd.getBrightness() > 50) M5.Lcd.setBrightness(50);
        drawHeader();

        Networking::getWiFiConfigWithDPP(
            [this](const char *uri) {
                int padding = 24;
                int width = M5.Display.width(), height = M5.Display.height();
                int size = (width < height ? width : height) - padding;
                M5.Display.qrcode(uri, (width - size) / 2, padding, size);
            },
            [this](wifi_config_t *config, const char *err) {
                if (config) {
                    settings.setWifiSetup(false);
                    Networking::setWiFiConfig(config);
                    succeeded((char*)config->sta.ssid);
                    RTOS::timeout(2000, []() {
                        esp_restart();
                    });
                } else {
                    failed(err);
                }
                setNeedsDisplay();
            }
        );
    }

    void drawHeader() {
        M5.Display.clear(TFT_WHITE);
        setTextStyle();
        M5.Display.setCursor(0, 0);
        M5.Display.println("WiFi Setup (DPP)");
    }

    void setTextStyle(int color = TFT_BLACK) {
        M5.Display.setTextColor(color, TFT_WHITE);
        M5.Display.setFont(&defaultFount);
    }

    void succeeded(const char *msg) {
        drawHeader();
        setTextStyle(TFT_GREEN);
        M5.Display.println("Succeeded.");
        setTextStyle();
        M5.Display.println(msg);
    }

    void failed(const char *msg) {
        drawHeader();
        setTextStyle(TFT_RED);
        M5.Display.println("Failed.");
        setTextStyle();
        M5.Display.println(msg);
        M5.Display.println("Please reset and try again.");
    }
};
