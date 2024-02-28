/*
 * Copyright (c) 2024 Hiroki Kawakami
 */

#pragma once
#include "kyoshin.hpp"
#include "map_view_scene.hpp"

class WifiConnectScene : public UI::Scene {
    string ssid = "";

    void didAppear() override {
        M5.Display.clear(TFT_WHITE);
        M5.Display.setCursor(0, 0);
        connect();
    }

    void connect() {
        ssid = Networking::ssid();
        bgTask1.send([&]() {
            this->connectNetwork();
            this->syncRealtimeClock();
        });
    }

    void connectNetwork() {
        this->setTextStyle();
        M5.Display.print(("Connecting to " + ssid + "...").c_str());
        for (int i = 0; i < 3; i++) {
            if (Networking::connect()) break;
            M5.Display.print("\nFailed. Retrying...");
        }
        if (!Networking::isWiFiConnected()) {
            error("\nFailed to connect to " + ssid + ".");
        }
        Networking::waitNetworkConnect();
        M5.Display.print(" : ");
        this->setTextStyle(TFT_GREEN);
        M5.Display.println("done.");
    }

    void syncRealtimeClock() {
        this->setTextStyle();
        M5.Display.print("Synchronizing realtime clock...");
        if (Networking::startSntp()) {
            M5.Display.print(" : ");
            this->setTextStyle(TFT_GREEN);
            M5.Display.println("done.");
            this->done();
        } else {
            this->setTextStyle(TFT_RED);
            M5.Display.println("failed.");
            this->error();
        }
    }

    void setTextStyle(int color = TFT_BLACK) {
        M5.Display.setTextColor(color, TFT_WHITE);
        M5.Display.setFont(&defaultFount);
    }

    void error(std::string msg = "") {
        setTextStyle(TFT_RED);
        if (!msg.empty()) M5.Display.println(msg.c_str());
        M5.Display.println("Please reset and try again.");
        RTOS::timeout(5000, []() {
            esp_restart();
        });
    }

    void done() {
        setTextStyle();
        presentScene(std::make_shared<MapViewScene>());
    }

    void eventLoop() override {
        if (M5.BtnA.wasPressed()) {
            settings.setWifiSetup(true);
            esp_restart();
        }
    }

};
